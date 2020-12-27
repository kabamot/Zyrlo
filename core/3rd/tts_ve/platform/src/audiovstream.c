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

/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/

/* ------------------------------------------------------------------
**  Data access services
**
**  Simple implementation : stream handle == file handle
**  If the audio file is encoded, it is decoded and the decoded
**  audio is written to a temporary file, so that the handling 
**  is the same as for a local file.
**  Only the vplatform_audiostream_Open is needed. The remaining are
**  simply equivalent to vplatform_file_* functions
** -----------------------------------------------------------------*/

#define MIME_MAX_LEN    32

/*------------------------------------------------------------------*/
/* check if the identifier contains a predefined mime type          */
/* e.g. audio/l16: for 16-bit LE PCM file without WAV header        */
NUAN_BOOL IsAudioFile(const char *szName)
{
    if ((strchr(szName, '.') != NULL) && (strchr(szName, ':') != NULL) && (strstr(szName, "audio/") != NULL))
    {
        return NUAN_TRUE;
    }
    return NUAN_FALSE;
}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_audiostream_Open(
    void       * hDataClass,
    void       * hHeap,
    const char * szName,
    void *     * phStream)
{
    char szMime[MIME_MAX_LEN]= { 0 };
    char szTempFile[MIME_MAX_LEN]= ".daf"; /* decompressed audio tempfile*/
    VPLATFORM_FILE_H hAudioFile;
    void *pOriginalBuffer    = NULL;
    void *pDecodedBuffer     = NULL;
    size_t cOriginalElements = 0;
    size_t cDecodedElements  = 0;
    size_t mimelen = 0;
    NUAN_ERROR fRet = NUAN_OK;

    *phStream = NULL;

    if ((! szName) || (! hDataClass))
    {
        fRet = NUAN_E_INVALIDARG;
    }
    
    /* copy mime type */
    memset(szMime, 0, MIME_MAX_LEN);
    mimelen = strcspn(szName, ":");
    if (mimelen > MIME_MAX_LEN-1 )
    {
        return (NUAN_E_INVALIDARG);
    }
    strncpy(szMime, &szName[0], mimelen);
    szName = strchr(szName, ':');
    szName++;

    /* Open compressed audio file */
    fRet = vplatform_file_OpenUTF8(hDataClass, hHeap, szName, "rb", &hAudioFile);
    if (fRet != NUAN_OK) goto endOpen;

    cOriginalElements = vplatform_file_GetSize(hAudioFile);
    pOriginalBuffer = (void *)vplatform_heap_Calloc(hHeap, cOriginalElements + 1, sizeof(char));
    if (!pOriginalBuffer)
    {
        fRet = NUAN_E_OUTOFMEMORY;
        vplatform_file_Close(hAudioFile);
        goto endOpen;
    }

    if (vplatform_file_Read(pOriginalBuffer, sizeof(char), cOriginalElements, hAudioFile) == cOriginalElements)
    {
        fRet = vplatform_file_Close(hAudioFile);
        if (fRet != NUAN_OK) goto endOpen;
        
        /* start decoding (based on szMime) pOriginalBuffer to pDecodedBuffer */
        /* implement or call decoder here */
        pDecodedBuffer = pOriginalBuffer;
        cDecodedElements = cOriginalElements;
        /* write decoded audio to temp file */
        fRet = vplatform_file_OpenUTF8(hDataClass, hHeap, szTempFile, "wb", hAudioFile);
        if (fRet != NUAN_OK) goto endOpen;

        if (vplatform_file_Write((void *)pDecodedBuffer, 1, cDecodedElements, hAudioFile) == cDecodedElements)
        {
            fRet = vplatform_file_Close(hAudioFile);
            if (fRet == NUAN_OK)
            {
                fRet = vplatform_file_OpenUTF8(hDataClass, hHeap, szTempFile, "rb", &hAudioFile);
                if (fRet == NUAN_OK)
                {
                    /* stream handle = handle to decoded audio file */
                    *phStream = hAudioFile;
                }
            }
        }
        else
        {
            vplatform_file_Close(hAudioFile);
            fRet = NUAN_E_FILEREADERROR;
        }
    }
    else
    {
        fRet = NUAN_E_FILEREADERROR;
    }
    
endOpen:
    if (pOriginalBuffer != NULL) vplatform_heap_Free(hHeap, pOriginalBuffer);
    return fRet;
} /* vplatform_audiostream_Open */


/* ******************************************************************
**  END
** ******************************************************************/
