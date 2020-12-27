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

#include "vplatform.h"
#include "vplatform_tchar.h"
#include "vheap.h"
#include "vdata.h"
#include "vfile.h"
#include "vcharconv.h"

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

/*lint -esym(715, hHeap, hDataClass, pbIsDir, pszPath, pszDir) 
**           not used in this implementation */

/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/

/* ------------------------------------------------------------------
**  FILE I/O
** -----------------------------------------------------------------*/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_Open(
  void *         hDataClass,
  void *     hHeap,
  const char                 * szName,
  const char                 * szMode,
  VPLATFORM_FILE_H           * phFile)
{
#ifdef PLATFORM_UNICODE
  return NUAN_E_NOTIMPLEMENTED;
#else
  *phFile = (VPLATFORM_FILE_H *)fopen(szName, szMode);
  return (*phFile == NULL) ? NUAN_E_COULDNOTOPENFILE : NUAN_OK;
#endif
}

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_OpenUTF8(
  void *         hDataClass,
  void *     hHeap,
  const char                 * szName,
  const char                 * szMode,
  VPLATFORM_FILE_H           * phFile)
{
    NUAN_ERROR fRet = NUAN_OK;
    PLATFORM_TCHAR *tszName = NULL;
    PLATFORM_TCHAR *tszMode = NULL;
  
    /* Convert to PLATFORM_TCHAR */
    fRet = vplatform_UTF8_to_PLATFORM_TCHAR(&tszName, szName, hHeap);
    if (fRet != NUAN_OK) return fRet;
  
    fRet = vplatform_UTF8_to_PLATFORM_TCHAR(&tszMode, szMode, hHeap);
    if (fRet != NUAN_OK) goto endOpenUTF8;
  
    /* Open file */
    fRet = vplatform_file_Open(hDataClass, hHeap, tszName, tszMode, phFile);

endOpenUTF8:
  if (tszName != NULL) vplatform_heap_Free(hHeap, tszName);
  if (tszMode != NULL) vplatform_heap_Free(hHeap, tszMode);
  return fRet;
} /* vplatform_file_OpenUTF8 */



/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_Close(
  VPLATFORM_FILE_H  hFile)
{
  NUAN_ERROR     fRet = NUAN_OK;
  FILE          *fp = (FILE *) hFile;

  if (0 != fclose(fp))
  {
    fRet = NUAN_E_FILECLOSE;
  }

  return fRet;
} /* vplatform_file_Close */

/*------------------------------------------------------------------*/
size_t vplatform_file_Read(
  void                * pBuffer,
  size_t              cElementBytes,
  size_t              cElements,
  VPLATFORM_FILE_H      hFile)
{
  FILE *fp = (FILE *) hFile;
  size_t u32Read = fread(pBuffer, cElementBytes, cElements, fp);
  return ((u32Read == 0) && (ferror(fp)) ? (size_t)0xffffffff : u32Read);
} /* vplatform_file_Read */
/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_Seek(
  VPLATFORM_FILE_H            hFile,
  size_t                      cOffset,
  VE_STREAM_ORIGIN     eOrigin,
  VE_STREAM_DIRECTION      eDirection)
{
  NUAN_ERROR fRet = NUAN_OK;
  FILE *fp = (FILE *) hFile;
  int iRet;
  NUAN_S32 s32Origin;
  
  switch (eOrigin)
  {
  case VE_STREAM_SEEK_SET: s32Origin = SEEK_SET; break;
  case VE_STREAM_SEEK_CUR: s32Origin = SEEK_CUR; break;
  case VE_STREAM_SEEK_END: s32Origin = SEEK_END; break;
  default:
    return NUAN_E_INVALIDARG;
    break;
  }

  /* Not safe for files larger then 2 GB, but portable for
  ** demonstration purposes. For production use we recommend a seek
  ** method that supports files up to UINT_MAX, such as _fseeki64( )
  ** for Windows OSes and fseeko( ) for many Unix variants. */
  if (cOffset > INT_MAX)
  {
    fRet = NUAN_E_FILESEEK;
  }
  else
  {
    iRet = fseek(fp, eDirection * (long int) cOffset, (int) s32Origin);
    if (iRet != 0)
    {
      fRet = NUAN_E_FILESEEK;
    }
  }
  
  return fRet;
} /* vplatform_file_Seek */

/*------------------------------------------------------------------*/
size_t vplatform_file_GetSize(
  VPLATFORM_FILE_H  hFile)
{
  NUAN_U32        u32FileSize = 0;
  FILE           *fp = (FILE *) hFile;
  NUAN_S32        s32Cur, s32Size;

  /* Not safe for files larger then 2 GB, but portable for
  ** demonstration purposes. For production use we recommend a file
  ** size method that supports files up to UINT_MAX, such as _filelengthi64( )
  ** for Windows OSes and fstat(fileno(fp), [...]) for many Unix variants. */
  s32Cur = (NUAN_S32)ftell(fp);
  if (s32Cur < 0) goto errorGetSize;

  if (0 != fseek(fp, 0, SEEK_END)) goto errorGetSize;
  s32Size = (NUAN_S32)ftell(fp);
  if (s32Size < 0) goto errorGetSize;
  u32FileSize = (NUAN_U32)s32Size;

  if (0 != fseek(fp, s32Cur, SEEK_SET)) goto errorGetSize;

  return u32FileSize;
  
errorGetSize:
  /* return 0 if something is wrong */
  return 0;
} /* vplatform_file_GetSize */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_Error(
  VPLATFORM_FILE_H  hFile)
{
  FILE *fp = (FILE *) hFile;
  return (ferror(fp) ? NUAN_E_FILEREADERROR : NUAN_OK);
} /* vplatform_file_Error */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_Flush(
  VPLATFORM_FILE_H  hFile)
{
  NUAN_ERROR     fRet = NUAN_OK;
  FILE *fp = (FILE *) hFile;
  
  if (0 != fflush(fp))
  {
    fRet = NUAN_E_FILEWRITEERROR;
  }
    
  return (fRet);
} /* vplatform_file_Flush */

/*------------------------------------------------------------------*/
size_t vplatform_file_Write(
  const void          * pBuffer,
  size_t                cElementBytes,
  size_t                cElements,
  VPLATFORM_FILE_H      hFile)
{
  FILE *fp = (FILE *) hFile;
  return fwrite(pBuffer, cElementBytes, cElements, fp);
} /* vplatform_file_Write */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_DeleteFile(
  void *         hDataClass,
  const PLATFORM_TCHAR       * pszFile)
{
  NUAN_ERROR nRc = NUAN_OK;

#ifdef PLATFORM_UNICODE
  return NUAN_E_NOTIMPLEMENTED;
#else
  /* ignore the error, return value is implementation defined */
  remove(pszFile);

  return nRc;
#endif    
} /* vplatform_file_DeleteFile */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_CreateDirectory(
  void *         hDataClass,
  const PLATFORM_TCHAR       * pszDir)
{
    return NUAN_E_NOTIMPLEMENTED;
} /* vplatform_file_CreateDirectory */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_IsDirectory(
  void *      hDataClass,
  const PLATFORM_TCHAR    * pszPath,
  NUAN_BOOL               * pbIsDir)
{
  return NUAN_E_NOTIMPLEMENTED;
} /* vplatform_file_IsDirectory */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_file_SetUnbufferedOutput(void)
{
    return NUAN_E_NOTIMPLEMENTED;
} /* vplatform_file_SetUnbufferedOutput */


/* ******************************************************************
**  END
** ******************************************************************/
