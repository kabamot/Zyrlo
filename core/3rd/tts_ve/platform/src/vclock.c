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
**  (c) Copyright 2017 Nuance Communications, Inc.
**  All rights reserved. Company confidential.
**
** ******************************************************************/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/

#include "vplatform.h"
#include "vheap.h"
#include "vclock.h"

/* Fairly high resolution UNIX version, including Linux and Solaris */
#include <sys/types.h>
#include <time.h>               /* For CLK_TCK */
#include <sys/times.h>          /* For times( ) */

/* ftime() is obsolete, use gettimeofday() instead */
#ifdef _USE_FTIME_
#include <sys/timeb.h>          /* for ftime( )/_ftime( ) */
#else
#include <sys/time.h>           /* for gettimeofday( ) */
#endif

/* The time.h CLK_TCK macro should NOT be used anymore,
   it is confusing and has also been removed from more
   recent standards. Instead, it is recommended to use
   the value returned by sysconf(). This is what recent
   versions of bits/time.h also do when defining CLK_TCK */
#ifndef CLK_TCK
#include <unistd.h>
#define CLK_TCK ((clock_t) sysconf (_SC_CLK_TCK))
#endif

/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

/* ******************************************************************
**  FUNCTIONS
** ******************************************************************/

/* ------------------------------------------------------------------
**  Clock service
** -----------------------------------------------------------------*/

/*-------------------------------------------------------------------
** Time structure for the clock interface
--------------------------------------------------------------------*/

#ifdef _USE_FTIME_

/* FTIME */

typedef struct VPLATFORM_CLOCK_DATA_PRIV_S
{
  struct timeb rtime;
  clock_t      utime;
  clock_t      stime;
} VPLATFORM_CLOCK_DATA_PRIV_T;

#elif defined _USE_CLOCK_GETTIME_

/* CLOCK_GETTIME */

typedef struct VPLATFORM_CLOCK_DATA_PRIV_S
{
  struct timespec rtime;
  struct timespec utime;
  struct timespec stime;
} VPLATFORM_CLOCK_DATA_PRIV_T;

#else

/* all other cases */

typedef struct VPLATFORM_CLOCK_DATA_PRIV_S
{
  struct timeval rtime;
  clock_t        utime;
  clock_t        stime;
} VPLATFORM_CLOCK_DATA_PRIV_T;

#endif


/*-------------------------------------------------------------------
** Structure for the clock instance
**------------------------------------------------------------------*/

typedef struct VPLATFORM_CLOCK_S {
  void                        * hHeap;
  VPLATFORM_CLOCK_DATA_PRIV_T * pOrigin;
} VPLATFORM_CLOCK_T;


/*-------------------------------------------------------------------
** set_clock_data (defined according to the compiler flag set)
**------------------------------------------------------------------*/

#ifdef _USE_FTIME_

/* FTIME */

static NUAN_ERROR set_clock_data(
  VPLATFORM_CLOCK_DATA_PRIV_T * pClockData
)
{
  NUAN_ERROR fRet = NUAN_OK;
  struct tms cpuTimeBuf;

  ftime(&(pClockData->real);
  times(&cpuTimeBuf);
  pClockData->utime = cpuTimeBuf.tms_utime;
  pClockData->stime = cpuTimeBuf.tms_stime;

  return fRet;
}

#elif defined _USE_CLOCK_GETTIME_

/* CLOCK_GETTIME */

static NUAN_ERROR set_clock_data(
  VPLATFORM_CLOCK_DATA_PRIV_T * pClockData
)
{
  NUAN_ERROR fRet = NUAN_OK;

  fRet = clock_gettime(CLOCK_MONOTONIC, &(pClockData->rtime));
  if (fRet) {
    pClockData->rtime.tv_sec = 0;
    pClockData->rtime.tv_nsec = 0;
  }
  /* NOTE CLOCK_PROCESS_CPUTIME_ID does not distinguish between utime and stime
  **      so we simply store its returned time in utime, and set stime to zero
  **      so that their sum roughly indicates the amount of time
  **      consumed by the process */
  fRet = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &(pClockData->utime));
  if (fRet) {
    pClockData->utime.tv_sec = 0;
    pClockData->utime.tv_nsec = 0;
  }
  pClockData->stime.tv_sec = 0;
  pClockData->stime.tv_nsec = 0;

  return fRet;
}

#else

/* all other cases */

static NUAN_ERROR set_clock_data(
  VPLATFORM_CLOCK_DATA_PRIV_T * pClockData
)
{
  NUAN_ERROR fRet = NUAN_OK;
  struct tms cpuTimeBuf;

  gettimeofday(&(pClockData->rtime), NULL);
  times(&cpuTimeBuf);
  pClockData->utime = cpuTimeBuf.tms_utime;
  pClockData->stime = cpuTimeBuf.tms_stime;

  return fRet;
}

#endif


/*-------------------------------------------------------------------
** vplatform_clock_ComputeClockTimeDifference overloaded
** for the different types defined by the compiler flags set
**------------------------------------------------------------------*/

#ifdef _USE_CLOCK_GETTIME_

/* CLOCK_GETTIME */
static NUAN_ERROR vplatform_clock_ComputeClockTimeDifference_timespec(
  struct timespec vBegin,
  struct timespec vEnd,
  NUAN_TIME * pResult
)
{
  if (vEnd.tv_sec < vBegin.tv_sec)
  {
    *pResult = 0;
  }
  else if ((vEnd.tv_sec == vBegin.tv_sec) && (vEnd.tv_nsec < vBegin.tv_nsec))
  {
    *pResult = 0;
  }
  else
  {
    *pResult = (NUAN_TIME)(((vEnd.tv_sec - vBegin.tv_sec) * 1000) + ((vEnd.tv_nsec - vBegin.tv_nsec) / 1000000));
  }
  return NUAN_OK;
}

#else

/* all cases other than CLOCK_GETTIME */

static NUAN_ERROR vplatform_clock_ComputeClockTimeDifference_clock_t(
  clock_t vBegin,
  clock_t vEnd,
  NUAN_TIME * pResult
)
{
  if (vEnd < vBegin)
  {
    *pResult = 0;
  }
  else
  {
    *pResult = (NUAN_TIME)((1000 * (vEnd - vBegin)) / CLK_TCK);
  }
  return NUAN_OK;
}

#ifdef _USE_FTIME_

/* FTIME */

static NUAN_ERROR vplatform_clock_ComputeClockTimeDifference_timeb(
  struct timeb vBegin,
  struct timeb vEnd,
  NUAN_TIME * pResult
)
{
  if (vEnd.time < vBegin.time)
  {
    *pResult = 0;
  }
  else if ((vEnd.time == vBegin.time) && (vEnd.millitm < vBegin.millitm))
  {
    *pResult = 0;
  }
  else
  {
    *pResult = (NUAN_TIME)(((vEnd.time - vBegin.time) * 1000) + (vEnd.millitm - vBegin.millitm));
  }
  return NUAN_OK;
}
#else

/* all other cases */

static NUAN_ERROR vplatform_clock_ComputeClockTimeDifference_timeval(
  struct timeval vBegin,
  struct timeval vEnd,
  NUAN_TIME * pResult
)
{
  if (vEnd.tv_sec < vBegin.tv_sec)
  {
    *pResult = 0;
  }
  else if ((vEnd.tv_sec == vBegin.tv_sec) && (vEnd.tv_usec < vBegin.tv_usec))
  {
    *pResult = 0;
  }
  else
  {
    *pResult = (NUAN_TIME)(((vEnd.tv_sec - vBegin.tv_sec) * 1000) + ((vEnd.tv_usec - vBegin.tv_usec) / 1000));
  }
  return NUAN_OK;
}
#endif

#endif


NUAN_ERROR vplatform_clock_GetRelativeTime(
  void            * hClock,
  NUAN_CLOCK_DATA * pClockData
)
{
  NUAN_ERROR                    fRet   = NUAN_OK;
  VPLATFORM_CLOCK_T           * pClock = NULL;
  VPLATFORM_CLOCK_DATA_PRIV_T   now;

  if (hClock == NULL)
  {
    fRet = NUAN_E_NULLPOINTER;
    goto end;
  }

  pClock = (VPLATFORM_CLOCK_T *)hClock;
  fRet = set_clock_data(&now);
  if (fRet != NUAN_OK)
  {
    goto end;
  }

#ifdef _USE_FTIME_
  /* FTIME */
  fRet = vplatform_clock_ComputeClockTimeDifference_timeb(pClock->pOrigin->rtime, now.rtime, &(pClockData->rtime));
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_clock_t(pClock->pOrigin->utime, now.utime, &(pClockData->utime));
  }
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_clock_t(pClock->pOrigin->stime, now.stime, &(pClockData->stime));
  }
#elif defined _USE_CLOCK_GETTIME_
  /* CLOCK_GETTIME */
  fRet = vplatform_clock_ComputeClockTimeDifference_timespec(pClock->pOrigin->rtime, now.rtime, &(pClockData->rtime));
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_timespec(pClock->pOrigin->utime, now.utime, &(pClockData->utime));
  }
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_timespec(pClock->pOrigin->stime, now.stime, &(pClockData->stime));
  }
#else
/* all other cases */
  fRet = vplatform_clock_ComputeClockTimeDifference_timeval(pClock->pOrigin->rtime, now.rtime, &(pClockData->rtime));
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_clock_t(pClock->pOrigin->utime, now.utime, &(pClockData->utime));
  }
  if (fRet == NUAN_OK)
  {
    fRet = vplatform_clock_ComputeClockTimeDifference_clock_t(pClock->pOrigin->stime, now.stime, &(pClockData->stime));
  }
#endif

end:
  return fRet;
}

/*===================================================================
**  Definition of static interfaces
**==================================================================*/

static const VE_CLOCK_INTERFACE IClock = {
  vplatform_clock_GetRelativeTime
};


/* ******************************************************************
**  PUBLIC INTERFACE RETRIEVAL FUNCTIONS
** ******************************************************************/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_clock_GetInterface(
  VE_INSTALL       * pInstall,
  VPLATFORM_RESOURCES * pResources
)
{
  NUAN_ERROR fRet = NUAN_OK;
  void              * hHeap  = NULL;
  VPLATFORM_CLOCK_T * pClock = NULL;

  /* silence lint for unused arg */
  ((void)pResources);

  hHeap = pInstall->hHeap;

  /* allocate space for the clock structure */
  pClock = (VPLATFORM_CLOCK_T *)vplatform_heap_Calloc(hHeap, 1, sizeof(VPLATFORM_CLOCK_T));
  if (pClock == NULL)
  {
    fRet = NUAN_E_OUTOFMEMORY;
    goto end;
  }
  pClock->pOrigin = (VPLATFORM_CLOCK_DATA_PRIV_T *)vplatform_heap_Calloc(hHeap, 1, sizeof(VPLATFORM_CLOCK_DATA_PRIV_T));
  if (pClock->pOrigin == NULL)
  {
    vplatform_heap_Free(hHeap, pClock);
    fRet = NUAN_E_OUTOFMEMORY;
    goto end;
  }

  /* set the clock origin */
  fRet = set_clock_data(pClock->pOrigin);
  if (fRet != NUAN_OK)
  {
    vplatform_heap_Free(hHeap, pClock->pOrigin);
    vplatform_heap_Free(hHeap, pClock);
    goto end;
  }

  pInstall->pIClock = &IClock;
  pClock->hHeap = hHeap;
  pInstall->hClock = (void *)pClock;

end:
  return fRet;
} /* vplatform_clock_GetInterface */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_clock_ReleaseInterface(
  void * hClock
)
{
  VPLATFORM_CLOCK_T * pClock = (VPLATFORM_CLOCK_T *)hClock;

  vplatform_heap_Free(pClock->hHeap, pClock->pOrigin);
  vplatform_heap_Free(pClock->hHeap, pClock);

  return NUAN_OK;
} /* vplatform_clock_ReleaseInterface */


/* ******************************************************************
**  END
** ******************************************************************/

