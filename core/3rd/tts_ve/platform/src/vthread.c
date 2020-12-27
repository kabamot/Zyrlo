/*===================================================================
 **  Copyright (c) 2018 Nuance Communications, Inc.
 **  All rights reserved. Company confidential.
 **
 **  Project:     BET5e
 **  Workfile:    vthread.c
 **  Purpose:     Multithreading primitive interfaces.
 **==================================================================*/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/

#include "vplatform.h"
#include "vthread.h"
#include "vheap.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>  /* Needed for select() in Linux */
#include <sys/types.h>



/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

#define NUAN_THREAD_HCHECK     135972
#define NUAN_SEMAPHORE_HCHECK  135973

#define CHECK_POINTER(__PTR) \
{\
  if(NULL==__PTR){\
  return NUAN_E_NULLPOINTER;\
  }\
}

#define RETURN_ON_ERROR(__RES) \
{\
  if(NUAN_OK!=__RES){\
  return __RES;\
  }\
}

typedef struct NUAN_THREAD_S {
  pthread_t*   threadHandle;
  void*    vHeap;
} NUAN_VTHREAD;

typedef struct NUAN_SEMAPHORE_S {
  sem_t*   semaphoreHandle;
  void*     vHeap;
} NUAN_VSEMAPHORE;


/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/

/*------------------------------------------------------------------*/
static NUAN_ERROR vplatform_thread_ValidateHandle(VE_HSAFE * pSafeHandle, NUAN_U32 pCheckVal)
{
  NUAN_ERROR lRet = NUAN_OK;
  
  CHECK_POINTER(pSafeHandle);

  if (pSafeHandle->u32Check != pCheckVal)
  {
    return NUAN_E_INVALIDHANDLE;
  }

  if (NULL == pSafeHandle->pHandleData) 
  {
    return NUAN_E_NULL_HANDLE;
  }
  
  return lRet;
}


/* ------------------------------------------------------------------
**  THREADS
** -----------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*lint -esym(715, phClass) */
NUAN_ERROR vplatform_thread_ObjOpen(void* phClass, void* pHeap, void ** pThreadH)
{
  NUAN_ERROR      fRet = NUAN_OK;
  NUAN_THREAD_H   lExtThread; 
  NUAN_VTHREAD*   lThread = NULL;
  *pThreadH = (NUAN_THREAD_H)vplatform_heap_Malloc(pHeap,sizeof(NUAN_THREAD_T));
  CHECK_POINTER(*pThreadH);

  lExtThread = ((NUAN_THREAD_H)(*pThreadH));

  lExtThread->u32Check = NUAN_THREAD_HCHECK;

  lThread = (NUAN_VTHREAD*)vplatform_heap_Malloc(pHeap,sizeof(NUAN_VTHREAD));
  if (NULL != lThread) {
    lThread->threadHandle = (pthread_t*)vplatform_heap_Malloc(pHeap,sizeof(pthread_t));
    if(NULL != lThread->threadHandle) {
      lThread->vHeap = pHeap; 
      lExtThread->pHandleData = (void*)lThread;
    }else {
      /*rollback*/
      vplatform_heap_Free(pHeap,lThread);
      vplatform_heap_Free(pHeap,pThreadH);
      fRet = NUAN_E_MALLOC;
    }
  } else {
    /*rollback*/
    vplatform_heap_Free(pHeap,pThreadH);
    fRet = NUAN_E_MALLOC;
  }
  
  return fRet;
}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_thread_ObjClose(void * pThreadH)
{
  NUAN_ERROR     fRet = NUAN_OK;
  NUAN_VTHREAD* lThread = NULL;
  NUAN_THREAD_H  lExtThread = (NUAN_THREAD_H)pThreadH;
  CHECK_POINTER(pThreadH);
  fRet = vplatform_thread_ValidateHandle(lExtThread,NUAN_THREAD_HCHECK);
  RETURN_ON_ERROR(fRet);

  lThread = (NUAN_VTHREAD*)lExtThread->pHandleData;
  vplatform_heap_Free(lThread->vHeap, lThread->threadHandle);
  vplatform_heap_Free(lThread->vHeap,pThreadH);
  vplatform_heap_Free(lThread->vHeap, lThread);

  return fRet;
}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_thread_Start(void * pThreadH, 
                                  VPLATFORM_THREAD_STARTFUNC pStartFunction,
                                  void* pArgs,
                                  size_t pStackSize)
{
  NUAN_ERROR              fRet = NUAN_OK;
  NUAN_VTHREAD             *lThread = NULL;
  int                     lRes = 0;
  NUAN_THREAD_H  lExtThread = ((NUAN_THREAD_H)(pThreadH));
  pthread_attr_t          lAttr;
  
  CHECK_POINTER(pThreadH);

  fRet = vplatform_thread_ValidateHandle(lExtThread,NUAN_THREAD_HCHECK);
  RETURN_ON_ERROR(fRet);

  lThread = (NUAN_VTHREAD*)(lExtThread->pHandleData);

  /* the usual pthread default stack size is 2MB which is much more than we
    really need. At higher thread counts, thread creation can fail because
    of the lack of virtual memory. For now, 100K seems safe. */
  pthread_attr_init(&lAttr);
  pthread_attr_setstacksize(&lAttr, pStackSize);
  pthread_attr_setdetachstate(&lAttr, PTHREAD_CREATE_JOINABLE);

  lRes = pthread_create((pthread_t *)(lThread->threadHandle), 
                                     &lAttr, pStartFunction, pArgs);
  if (lRes != 0) {
    fRet = NUAN_E_COULDNOTOPENFILE;
  }

  return fRet;
}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_thread_Join(void * pThreadH,void* pStatus)
{
  NUAN_ERROR      fRet = NUAN_OK;
  NUAN_VTHREAD   *lThread = NULL;
  int             lRes = 0; 
  NUAN_THREAD_H   lExtThread = ((NUAN_THREAD_H)(pThreadH));
  
  CHECK_POINTER(pThreadH);

  fRet = vplatform_thread_ValidateHandle(lExtThread,NUAN_THREAD_HCHECK);
  RETURN_ON_ERROR(fRet);

  lThread = (NUAN_VTHREAD*)(lExtThread->pHandleData);

  lRes = pthread_join(*(lThread->threadHandle), pStatus);
  
  if (lRes != 0) {
    fRet = NUAN_E_NOK;
  }

  return fRet;
}

/*------------------------------------------------------------------*/
/*lint -esym(715, pThreadH) */
NUAN_ERROR vplatform_thread_SleepMs(void *pThreadH ,unsigned short pMS)
{
  NUAN_ERROR     fRet = NUAN_OK;
  useconds_t luseconds;
  
  luseconds = pMS * 1000;
  
  usleep(luseconds);

  return fRet;
}

/*------------------------------------------------------------------*/
/*lint -esym(715, pThreadH) */
NUAN_ERROR vplatform_thread_GetCallingThreadId(void *pThreadH, unsigned int *pThreadId)
{
  NUAN_ERROR     fRet = NUAN_OK;

if (NULL != pThreadId) {
  *pThreadId = 0;
  *pThreadId += pthread_self();
}

  return fRet;
}


/* -------------------------------------------------------------------------+
|   SEMAPHORES SECTIONS BEGIN                                              |
+ -------------------------------------------------------------------------*/

/*lint -esym(715, pHclass) */
NUAN_ERROR vplatform_Semaphore_ObjOpen(void* pHclass,
                                       void* pHeap,
                                       signed int pInit,
                                       signed int pMax,
                                       void ** pSem)
{
  NUAN_ERROR fRet = NUAN_OK;
  NUAN_VSEMAPHORE* lSemaphore;
  NUAN_SEMAPHORE_H lExtSemaphore;

  (*pSem) = (NUAN_SEMAPHORE_H)vplatform_heap_Malloc(pHeap,sizeof(NUAN_SEMAPHORE_T));
  CHECK_POINTER(*pSem);
  
  lExtSemaphore = ((NUAN_SEMAPHORE_H)(*pSem));

  lExtSemaphore->u32Check = NUAN_SEMAPHORE_HCHECK;
  lSemaphore = (NUAN_VSEMAPHORE*)vplatform_heap_Malloc(pHeap,sizeof(NUAN_VSEMAPHORE));

  if (NULL != lSemaphore) {
    lSemaphore->semaphoreHandle = (sem_t*)vplatform_heap_Malloc(pHeap,sizeof(sem_t));
    if (NULL != lSemaphore->semaphoreHandle) {
      lSemaphore->vHeap = pHeap;
      sem_init(lSemaphore->semaphoreHandle,0,pInit);
      lExtSemaphore->pHandleData = (void*)lSemaphore;
    } else {
      /*rollback*/
      vplatform_heap_Free(pHeap,lSemaphore);
      vplatform_heap_Free(pHeap,pSem);
      fRet = NUAN_E_MALLOC;
    }
  } else {
    /*rollback*/
    vplatform_heap_Free(pHeap,pSem);
    fRet =NUAN_E_MALLOC;
  }

  return fRet;
}

NUAN_ERROR vplatform_Semaphore_ObjClose(void * pSem) 
{
  NUAN_ERROR     fRet = NUAN_OK;

  NUAN_VSEMAPHORE* lSemaphore;
  NUAN_SEMAPHORE_H lExtSemaphore = (NUAN_SEMAPHORE_H)pSem;

  CHECK_POINTER(pSem);

  fRet = vplatform_thread_ValidateHandle(lExtSemaphore,NUAN_SEMAPHORE_HCHECK);
  RETURN_ON_ERROR(fRet);

  lSemaphore = (NUAN_VSEMAPHORE*)(lExtSemaphore->pHandleData);
  sem_destroy(lSemaphore->semaphoreHandle);

  vplatform_heap_Free(lSemaphore->vHeap, lSemaphore->semaphoreHandle);
  vplatform_heap_Free(lSemaphore->vHeap,pSem);
  vplatform_heap_Free(lSemaphore->vHeap,lSemaphore);

  return fRet;
}


NUAN_ERROR vplatform_Semaphore_Acquire(void * pSem)
{
  NUAN_ERROR fRet = NUAN_OK;
  NUAN_VSEMAPHORE* lSemaphore;
  NUAN_SEMAPHORE_H lExtSemaphore = ((NUAN_SEMAPHORE_H)(pSem));

  CHECK_POINTER(pSem);

  fRet = vplatform_thread_ValidateHandle(lExtSemaphore,NUAN_SEMAPHORE_HCHECK);
  RETURN_ON_ERROR(fRet);

  lSemaphore=(NUAN_VSEMAPHORE*)(lExtSemaphore->pHandleData);

  if(0 != sem_wait(lSemaphore->semaphoreHandle))
  {
    fRet = NUAN_E_SYSTEM_ERROR;
  }
  
  return fRet;
}

NUAN_ERROR vplatform_Semaphore_Release(void * pSem)
{
  NUAN_ERROR fRet = NUAN_OK;
  NUAN_VSEMAPHORE* lSemaphore;
  NUAN_SEMAPHORE_H lExtSemaphore = ((NUAN_SEMAPHORE_H)(pSem));

  CHECK_POINTER(pSem);

  fRet = vplatform_thread_ValidateHandle(lExtSemaphore,NUAN_SEMAPHORE_HCHECK);
  RETURN_ON_ERROR(fRet);

  lSemaphore=(NUAN_VSEMAPHORE*)(lExtSemaphore->pHandleData);
  
  if(0 != sem_post(lSemaphore->semaphoreHandle))
  {
    fRet = NUAN_E_SYSTEM_ERROR;
  }

  return fRet;
}


/*===================================================================
**  Definition of static interfaces
**==================================================================*/

static const VE_MTHREAD_INTERFACE IVthreadInterface = {
  vplatform_thread_ObjOpen,
  vplatform_thread_ObjClose,
  vplatform_thread_Start,
  vplatform_thread_Join,
  vplatform_thread_SleepMs,
  vplatform_thread_GetCallingThreadId
};

static const VE_SEMAPHORE_INTERFACE IVSemaphoreInterface = {
  vplatform_Semaphore_ObjOpen,
  vplatform_Semaphore_ObjClose,
  vplatform_Semaphore_Acquire,
  vplatform_Semaphore_Release
};


/*===================================================================
**  PUBLIC INTERFACE RETRIEVAL FUNCTIONS
**==================================================================*/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_thread_GetInterface( VE_INSTALL     * pInstall, 
                                          VPLATFORM_RESOURCES * pResources) {
  NUAN_ERROR l_result = NUAN_OK;
  void * l_heap;
  pInstall->pIThread = &IVthreadInterface;
  pInstall->pISemaphore = &IVSemaphoreInterface;
  return l_result;
}
    
 /*------------------------------------------------------------------*/  
NUAN_ERROR vplatform_thread_ReleaseInterface(void * hThread) {
  (void) hThread;
  return NUAN_OK;
}

/* ******************************************************************
**  END
** ******************************************************************/