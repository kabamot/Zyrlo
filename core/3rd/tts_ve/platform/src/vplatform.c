/* ******************************************************************
**  Nuance Communications, Inc.
** ******************************************************************/

/* ******************************************************************
**
**  COPYRIGHT INFORMATION
**
**  This program contains proprietary information that is a trade secret
**  of Nuance Communications, Inc. and also is protected as an unpublished
**  work under applicable Copyright laws. Recipient is to retain this
**  program in confidence and is not permitted to use or make copies
**  thereof other than as permitted in a prior written agreement with
**  Nuance Communications, Inc. or its affiliates.
**
**  (c) Copyright 2008 Nuance Communications, Inc.
**  All rights reserved. Company confidential.
**  
** ******************************************************************/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/

#include <stdlib.h>
#include "vplatform_tchar.h"
#include "vplatform.h"
#include "vheap.h"
#include "vdata.h"
#include "vcritsec.h"
#include "vlog.h"

#ifdef _TTS_EXT_CLOCK_
#include "vclock.h"
#endif

#include "vthread.h"



/* ******************************************************************
**  COMPILER DIRECTIVES
** ******************************************************************/

/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/


/* ******************************************************************
**  GLOBAL FUNCTIONS (prototypes in header file)
** ******************************************************************/

NUAN_ERROR vplatform_GetInterfaces(
  VE_INSTALL            * pInstall,
  VPLATFORM_RESOURCES      * pResources
)
{
  NUAN_ERROR fRet = NUAN_OK;

  if ( (pResources->fmtVersion > VPLATFORM_CURRENT_VERSION) ||
       (pResources->fmtVersion < VPLATFORM_CURRENT_VERSION))
    return (NUAN_E_VERSION);
    
  fRet = vplatform_heap_GetInterface(pInstall, pResources);

  if (fRet == NUAN_OK)
  {
    fRet = vplatform_critsec_GetInterface(pInstall, pResources);
    
    if (fRet == NUAN_OK)
    {
      fRet = vplatform_data_GetInterface(pInstall, pResources);

      if (fRet == NUAN_OK)
      {
        fRet = vplatform_log_GetInterface(pInstall, pResources);

#if defined(EXT_MULTITHREAD)
        if (fRet == NUAN_OK) {

          fRet = vplatform_thread_GetInterface(pInstall, pResources);   
        }
#endif
#ifdef _TTS_EXT_CLOCK_
        if (fRet == NUAN_OK)
        {
          fRet = vplatform_clock_GetInterface(pInstall, pResources);
        }
#endif

      }
    }
  }
  if (fRet != NUAN_OK)
  {
    vplatform_ReleaseInterfaces(pInstall);
  }
  
  return fRet;
}

/*-------------------------------------------------------------------
**  @func   Release all interfaces and handles.
**  @rdesc  NUAN_ERROR | Success or failure
**------------------------------------------------------------------*/
NUAN_ERROR vplatform_ReleaseInterfaces(
  VE_INSTALL     * pInstall)
{
#ifdef _TTS_EXT_CLOCK_
  if (pInstall->hClock != NULL)
  {
    vplatform_clock_ReleaseInterface(pInstall->hClock);
    pInstall->hLog = NULL;
  }
#endif
  if (pInstall->hLog != NULL)
  {
    vplatform_log_ReleaseInterface(pInstall->hLog);
    pInstall->hLog = NULL;
  }
  if (pInstall->hDataClass != NULL)
  {
    vplatform_data_ReleaseInterface(pInstall->hDataClass);
    pInstall->hDataClass = NULL;
  }
  if (pInstall->hCSClass != NULL)
  {
    vplatform_critsec_ReleaseInterface(pInstall->hCSClass);
    pInstall->hCSClass = NULL;
  }

#if defined(EXT_MULTITHREAD)
  if(NULL != pInstall->pIThread)
  {
    vplatform_thread_ReleaseInterface((void*)pInstall->pIThread);
    pInstall->pIThread = NULL;
  }
  vplatform_heap_ReleaseInterface(pInstall->hHeap);
#endif

  return NUAN_OK;
}


/* ******************************************************************
**  END
** ******************************************************************/
