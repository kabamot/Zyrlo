#if defined(__VTHREAD_H__)
  #error "__VTHREAD_H__ has been already included"
#else
#define __VTHREAD_H__

/*===================================================================
 **  Copyright (c) 2018 Nuance Communications, Inc.
 **  All rights reserved. Company confidential.
 **
 **  Workfile:    vthread.h
 **  Purpose:     Multithreading primitive interfaces.
 **==================================================================*/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/

#include "vplatform.h"

#if defined( __cplusplus )
extern "C"
{
#endif

/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

#define VPLATFORM_THREAD_ARG void*

#define VPLATFORM_THREAD_DEFINE_THREAD_FUNC(name, x) \
void* name(void* x)


typedef VE_HSAFE NUAN_THREAD_T;
typedef NUAN_THREAD_T* NUAN_THREAD_H;
typedef VE_HSAFE NUAN_SEMAPHORE_T;
typedef NUAN_SEMAPHORE_T* NUAN_SEMAPHORE_H;

/*------------------------------------------------------------------*/
/**
 * @brief Allocate memory for a thread handle in a safe mode.
 * @param phClass     [unused here]
          pHeap       [in] handle of the heap to be passed to the platform
                           heap primitives
          pThread     [out] a pointer to the thread handle
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_thread_ObjOpen(void* phClass, void* pHeap, void ** pThreadH);

/*------------------------------------------------------------------*/
/**
 * @brief Destroy a thread handle.
 * @param pThread [in] a pointer to the thread handle
 * @return NUAN_ERROR | Success or failure
 * @note There isn't any control on the thread status. If the thread referenced by
 *       the pThreadH is still running the handle to it will be lost.
 */
NUAN_ERROR vplatform_thread_ObjClose(void * pThreadH);

/*------------------------------------------------------------------*/
/**
 * @brief Runs the pFuncThread in a separate thread with ID pThread
 * @param pThread      [in] a pointer to the thread handle
          pThreadFunc  [in] thread main loop function
          pUserData    [in] pointer to user specific data
          pStackSize   [in] (Windows only) The initial size of the stack, in bytes 
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_thread_Start(void * pThreadH, 
                                  VPLATFORM_THREAD_STARTFUNC pStartFunction,
                                  void* pArgs,
                                  size_t pStackSize);

/*------------------------------------------------------------------*/
/**
 * @brief The vplatform_thread_join() function suspends execution of 
 *        the calling thread until the target pThreadH terminates, 
 *        unless the target thread has already terminated. On return 
 *        from a successful join call with a non-NULL pStatus argument, 
 *        the value passed to pthread_exit() by the terminating thread
 *        is made available in the location referenced by pStatus.
 * @param pThread [in] a pointer to the thread handle
 *        pStatus [out] 
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_thread_Join(void * pThreadH,void* pStatus);

/*------------------------------------------------------------------*/
/**
 * @brief Puts the pThreadH in a sleep state for pMS milliseconds
 * @param pThread [in] a pointer to the thread handle
 *        pMS     [in] amount of milliseconds to sleep 
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_thread_SleepMs(void *pThreadH ,unsigned short pMS);

/*------------------------------------------------------------------*/
/**
 * @brief Returns the ID of the calling thread.
 * @param pThreadH   [in] handle to the threads
          pThreadId  [out] the threadID 
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_thread_GetCallingThreadId(void *pThreadH, unsigned int *pThreadId);

/*------------------------------------------------------------------*/
/**
 * @brief Allocate memory for a Semaphore handle in a safe mode.
 * @param phClass     [unused here]
          pHeap       [in] handle of the heap to be passed to the platform
                           heap primitives
          pSem        [out] a pointer to the semaphore handle
          pInit       [in] The initial count for the semaphore object. 
                           This value must be greater than or equal to zero 
                           and less than or equal to pMax
          pInit       [in] The maximum count for the semaphore object. 
                           This value must be greater than zero.
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_Semaphore_ObjOpen(void* pHclass,
                                       void* pHeap,
                                       signed int pInit,
                                       signed int pMax,
                                       void ** pSem);

/*------------------------------------------------------------------*/
/**
 * @brief Destroy a semaphore.
 * @param pSem [in] a pointer to the semaphore handle
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_Semaphore_ObjClose(void * pSem);

/*------------------------------------------------------------------*/
/**
 * @brief Acquire a semaphore.
 * @param pSem [in] a pointer to the semaphore handle
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_Semaphore_Acquire(void * pSem);

/*------------------------------------------------------------------*/
/**
 * @brief Release a semaphore.
 * @param pSem [in] a pointer to the semaphore handle
 * @return NUAN_ERROR | Success or failure
 */
NUAN_ERROR vplatform_Semaphore_Release(void * pSem); 


/*-------------------------------------------------------------------
**  @func   Get the interface and an instance handle of the Heap service.
**  @rdesc  NUAN_ERROR | Success or failure
**------------------------------------------------------------------*/
NUAN_ERROR vplatform_thread_GetInterface(
    VE_INSTALL       * pInstall,   /* @parm [out] <nl>
                                      ** Vocalizer install info */
    VPLATFORM_RESOURCES * pResources  /* @parm [in] <nl>
                                      ** Platform resources*/
);
    
/*-------------------------------------------------------------------
**  @func   Release the handle of the Thread service.
**  @rdesc  NUAN_ERROR | Success or failure
**------------------------------------------------------------------*/    
NUAN_ERROR vplatform_thread_ReleaseInterface(
    void * hThread                  /* @parm [in] <nl>
                                    ** Handle of concern */
);

#if defined( __cplusplus )
}
#endif

#endif
