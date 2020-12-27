/* ******************************************************************
**  Cerence, Inc.
** ******************************************************************/

/* ******************************************************************
**
**  COPYRIGHT INFORMATION
**
**  This program contains proprietary information that is a trade secret
**  of Cerence, Inc. and also is protected as an unpublished
**  work under applicable Copyright laws. Recipient is to retain this
**  program in confidence and is not permitted to use or make copies
**  thereof other than as permitted in a prior written agreement with
**  Cerence, Inc. or its affiliates.
**
**  (c) Copyright 2019 Cerence, Inc.
**  All rights reserved. Company confidential.
**
** ******************************************************************/

/* ******************************************************************
**  HEADER (INCLUDE) SECTION
** ******************************************************************/

#include "vplatform.h"
#include "vplatform_tchar.h"
#include "vheap.h"
#include "vdata.h"
#include "vfile.h"
#include "urlvstream.h"
#include "localvstream.h"
#include "audiovstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ******************************************************************
**  COMPILER DIRECTIVES
** ******************************************************************/


/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

/*lint -esym(715, hHeap, hDataClass)
**           not used in this implementation */
typedef enum { VSTREAM_NONE, VSTREAM_LOCAL_FILE, VSTREAM_AUDIO_FILE, VSTREAM_URL} eStreamType;

/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/

/* ------------------------------------------------------------------
** Data access services
**
** Implementation : stream handle is a struct holding either a
**  - file handle for local file access
**  - file handle for (encoded) audio file access
**  - handle to do URL remote access
** -----------------------------------------------------------------*/

typedef struct VSTREAM_HANDLE_S {
  void *hvStreamHandle;
  eStreamType eType;
  void *hHeap;
} VSTREAM_HANDLE_T;

NUAN_ERROR vplatform_datastream_Close(void *   hStream);

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_datastream_Open(
  void        * hDataClass,
  void        * hHeap,
  const char  * szName,
  const char  * szMode,
  void *      * phStream)
{
    VSTREAM_HANDLE_T *hStream;
    NUAN_ERROR err = NUAN_OK;
    NUAN_BOOL bRemote = NUAN_FALSE;

    if (hDataClass == NULL || szName == NULL || szMode == NULL || phStream == NULL)
    {
        return NUAN_E_INVALIDARG;
    }

    *phStream = NULL;
    hStream = (VSTREAM_HANDLE_T *)vplatform_heap_Malloc(hHeap, sizeof(VSTREAM_HANDLE_T));
    if (hStream == NULL)
    {
        return NUAN_E_OUTOFMEMORY;
    }
    hStream->hvStreamHandle = NULL;
    hStream->eType = VSTREAM_NONE;
    hStream->hHeap = hHeap;

    if ((szMode[0] == 'r') && UrlIsUrl(szName, &bRemote))
    {
        URLVSTREAM_HCLASS hClass = NULL;
        err  = vplatform_data_GetStreamClassData(hDataClass,&hClass);
        if (err == NUAN_OK)
        {
            err = vplatform_urlstream_Open(hClass,szName,&hStream->hvStreamHandle);
            hStream->eType = VSTREAM_URL;
        }
    }
    else if ((szMode[0] == 'r') && IsAudioFile(szName))
    {
        err = vplatform_audiostream_Open(hDataClass, hHeap, szName, &hStream->hvStreamHandle);
        hStream->eType = VSTREAM_AUDIO_FILE;
    }
    else
    {
        err = vplatform_localstream_Open(hDataClass,hHeap,szName,szMode,&hStream->hvStreamHandle);
        hStream->eType = VSTREAM_LOCAL_FILE;
    }
    if (err != NUAN_OK)
    {
        vplatform_datastream_Close(hStream);
    }
    else
    {
        *phStream = (void *)hStream;
    }
    return err;
} /* vplatform_datastream_Open */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_datastream_Close(
  void *   hStream)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;

    if (pStream)
    {
        switch(pStream->eType)
        {
            case VSTREAM_URL:
                vplatform_urlstream_Close(pStream->hvStreamHandle);
                break;
            case VSTREAM_AUDIO_FILE:
                /* fall through */
            case VSTREAM_LOCAL_FILE:
                if (pStream->hvStreamHandle)
                {
                    (void)vplatform_file_Close((VPLATFORM_FILE_H)pStream->hvStreamHandle);
                }
                break;
            default:
                break;
        }
        vplatform_heap_Free(pStream->hHeap,pStream);
    }
    return NUAN_OK;
} /* vplatform_datastream_Close */

/*------------------------------------------------------------------*/
size_t vplatform_datastream_Read(
  void                    * pBuffer,
  size_t                    cElementBytes,
  size_t                    cElements,
  void *     hStream)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;

    if (pStream)
    {
        switch(pStream->eType)
        {
            case VSTREAM_URL:
                return vplatform_urlstream_Read(pBuffer,cElementBytes,cElements,pStream->hvStreamHandle);
                break;
            case VSTREAM_AUDIO_FILE:
                /* fall through */
            case VSTREAM_LOCAL_FILE:
                return vplatform_file_Read(pBuffer, cElementBytes, cElements, (VPLATFORM_FILE_H)pStream->hvStreamHandle);
            default:
                break;
       }
    }
    return 0;
} /* vplatform_datastream_Read */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_datastream_Seek(
  void *           hStream,
  size_t                          cOffset,
  VE_STREAM_ORIGIN         eOrigin,
  VE_STREAM_DIRECTION          eDirection)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;
    if (pStream)
    {
        switch(pStream->eType)
        {
            case VSTREAM_URL:
                return vplatform_urlstream_Seek(pStream->hvStreamHandle,cOffset, eOrigin, eDirection);
                break;
            case VSTREAM_AUDIO_FILE:
                /* fall through */
            case VSTREAM_LOCAL_FILE:
                return vplatform_file_Seek((VPLATFORM_FILE_H)pStream->hvStreamHandle, cOffset, eOrigin, eDirection);
                break;
            default:
                break;
        }
    }
    return NUAN_E_INVALIDARG;
} /* vplatform_datastream_Seek */

/*------------------------------------------------------------------*/
size_t vplatform_datastream_GetSize(
  void *   hStream)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;

    if (pStream)
    {
        switch(pStream->eType)
        {
            case VSTREAM_URL:
                return vplatform_urlstream_GetSize(pStream->hvStreamHandle);
                break;
            case VSTREAM_AUDIO_FILE:
                /* fall through */
            case VSTREAM_LOCAL_FILE:
                return vplatform_file_GetSize((VPLATFORM_FILE_H)pStream->hvStreamHandle);
                break;
            default:
                break;
        }
    }
    return 0;
} /* vplatform_datastream_GetSize */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_datastream_Error(
  void *   hStream)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;

    if (pStream)
    {
        switch(pStream->eType)
        {
            case VSTREAM_URL:
                return vplatform_urlstream_Error(pStream->hvStreamHandle);
                break;
            case VSTREAM_AUDIO_FILE:
                /* fall through */
            case VSTREAM_LOCAL_FILE:
                return  vplatform_file_Error((VPLATFORM_FILE_H)pStream->hvStreamHandle);
                break;
            default:
                break;
        }
    }
    return NUAN_E_INVALIDARG;
} /* vplatform_datastream_Error */

/*------------------------------------------------------------------*/
size_t vplatform_datastream_Write(
  const void * pBuffer,
  size_t       cElementBytes,
  size_t       cElements,
  void       * hStream)
{
    VSTREAM_HANDLE_T *pStream = (VSTREAM_HANDLE_T *)hStream;

    if (pStream != NULL)
    {
        if (pStream->eType == VSTREAM_LOCAL_FILE)
        {
            return vplatform_file_Write(pBuffer, cElementBytes, cElements, (VPLATFORM_FILE_H)pStream->hvStreamHandle);
        }
        else
        {
            return NUAN_E_NOTIMPLEMENTED;
        }
    }
    return NUAN_E_INVALIDARG;
} /* vplatform_datastream_Write */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_datastream_Init(
  void * hDataClass,
  void *hHeap
)
{
    URLVSTREAM_HCLASS hClass = NULL;
    URLVSTREAM_ERROR urlRet = vplatform_urlstream_Initialize(hHeap,&hClass);

    if (urlRet != NUAN_OK)
    {
        return NUAN_E_INTFNOTFOUND;
    }
    (void)vplatform_data_SetStreamClassData(hDataClass,hClass);
    return NUAN_OK;
}

/*------------------------------------------------------------------*/
void vplatform_datastream_Uninit(
  void *  hDataClass
)
{
    URLVSTREAM_HCLASS hClass = NULL;

    if (NUAN_OK == vplatform_data_GetStreamClassData(hDataClass,&hClass))
    {
        vplatform_urlstream_Uninitialize(hClass);
    }
    (void)vplatform_data_SetStreamClassData(hDataClass,NULL);
}



/* ******************************************************************
**  END
** ******************************************************************/
