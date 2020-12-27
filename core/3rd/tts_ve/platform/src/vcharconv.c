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
**  (c) Copyright 2014 Nuance Communications, Inc.
**  All rights reserved. Company confidential.
**  
** ******************************************************************/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/
#include <string.h>
#include "vplatform.h"
#include "vheap.h"
#include "vcharconv.h"


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
**  GLOBAL FUNCTIONS
**
**  Conversion from char to PLATFORM_TCHAR using simple casts
** ******************************************************************/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_UTF8_to_PLATFORM_TCHAR(
    PLATFORM_TCHAR             **pDst,
    const char                  *pSrc,
    void *     hHeap)
{
    NUAN_U32 len = 0;
    const char * p = pSrc;
    PLATFORM_TCHAR * cp;
  
    if (pSrc == NULL) return NUAN_E_INVALIDPOINTER;
    while (*p != 0) { p++; len++;}
    *pDst = vplatform_heap_Calloc(hHeap, len + 1, sizeof(PLATFORM_TCHAR));
    if (*pDst == NULL) return NUAN_E_OUTOFMEMORY;

    cp = *pDst;
    /*lint --e(571) */
    while( (*cp = (PLATFORM_TCHAR) *pSrc) != 0 ) { cp++; pSrc++; }/* Copy src to end of dst */
    
    return NUAN_OK;

}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_PLATFORM_TCHAR_to_UTF8(
    char                       **pDst,
    const PLATFORM_TCHAR        *pSrc,
    void *     hHeap)
{
    NUAN_U32 len = 0;
    const PLATFORM_TCHAR * p = pSrc;
    char * cp;
   
    while (*p != 0) { p++; len++;}
    *pDst = vplatform_heap_Calloc(hHeap, len + 1, sizeof(char));
    if (*pDst == NULL) return NUAN_E_OUTOFMEMORY;

    cp = *pDst;

    /*lint --e(571) */
    while( (*cp = (char) *pSrc) != 0 ) { cp++; pSrc++; }/* Copy src to end of dst */
    
    return NUAN_OK;
}
