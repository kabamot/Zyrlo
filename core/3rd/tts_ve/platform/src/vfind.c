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

#include "vfind.h"

/* ******************************************************************
**  COMPILER DIRECTIVES
** ******************************************************************/


/* POSIX implementation for Linux, Solaris, and other Unix variants */
#include <dlfcn.h>
#include <sys/types.h> /* For "open()". */
#include <sys/stat.h>  /* For "stat()". */
#include <dirent.h>    /* For "readdir()" and "opendir()". */
#include <string.h>
#include <strings.h>

#define VPLATFORM_FIND_SZ_DIR_SEP "/"

/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

/* Structure to keep track of a directory search */
struct VPLATFORM_FIND_S {
  /***/ void *    hHeap;
  /***/ const char                * szRoot;
  /***/ DIR                       * dir;
};

/* ******************************************************************
**  LOCAL FUNCTIONS
** ******************************************************************/


/* ******************************************************************
**  GLOBAL FUNCTIONS (prototypes in header file)
** ******************************************************************/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_find_Open(
        void *        hDataClass,
        void *    hHeap,
  const char                      * szDir,
        char                     ** pszElem,
        VPLATFORM_FIND_TYPE       * peType,
        VPLATFORM_FIND_H          * phFind)
{
  NUAN_ERROR Err = NUAN_OK;
  DIR * dir = NULL;
  VPLATFORM_FIND_T * pFind = NULL;

  /* Open a directory and start retrieving its contents */
  if ((szDir == NULL) || (szDir[0] == 0) || (phFind == NULL))
  {
    return NUAN_E_NULL_POINTER;
  }

  *pszElem = NULL;
  *peType = VPLATFORM_FIND_OTHER;  
  *phFind = NULL;

  /* POSIX implementation */
  dir = opendir(szDir);
  if (dir == NULL)
  {
    Err = NUAN_E_NOTFOUND;
  }
  else
  {
    pFind = (VPLATFORM_FIND_T *)vplatform_heap_Malloc(hHeap, sizeof(VPLATFORM_FIND_T));
    if (pFind == NULL)
    {
      closedir(dir);
      return NUAN_E_OUTOFMEMORY;
    }
    
    pFind->hHeap = hHeap;
    pFind->dir = dir;
    pFind->szRoot = szDir;

    Err = vplatform_find_Next(pFind, pszElem, peType);
    if (Err != NUAN_OK)
    {
      vplatform_find_Close(pFind);
    }
    else
    {
      *phFind = pFind;
    }
  }

  return Err;
} /* vplatform_find_Open */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_find_Next(
  const VPLATFORM_FIND_H      hFind,
        char               ** pszElem,
        VPLATFORM_FIND_TYPE * peType)
{
  NUAN_ERROR Err = NUAN_OK;
  VPLATFORM_FIND_T * pFind = NULL;
  struct dirent * dirinfo = NULL;
  struct stat statbuf;
  size_t ElemSize;
  char * szConcat = NULL;

  /* Find the next file for a directory search */
  if (hFind == NULL)
  {
    return NUAN_E_NULL_POINTER;
  }

  pFind = (VPLATFORM_FIND_T *)hFind;
  *pszElem = NULL;
  *peType = VPLATFORM_FIND_OTHER;  

  /* Skip the "." and ".." directories */
  do
  {
    if ((dirinfo = readdir(pFind->dir)) == NULL)
    {
      Err = NUAN_E_NOTFOUND;
    }
  }
  while ((Err == NUAN_OK) && ((strcmp(dirinfo->d_name, ".") == 0) ||
                              (strcmp(dirinfo->d_name, "..") == 0)));
  
  if (Err == NUAN_OK)
  {
    szConcat = vplatform_CombinePath(pFind->hHeap, pFind->szRoot, dirinfo->d_name);  
    if (szConcat == NULL) return NUAN_E_OUTOFMEMORY;
    
    if ((stat(szConcat, &statbuf) == 0) && (S_ISDIR(statbuf.st_mode)))
    {
      *peType = VPLATFORM_FIND_DIR;
    }
    else if (strlen(dirinfo->d_name) > 4)
    {
      char *pExt = &(dirinfo->d_name)[strlen(dirinfo->d_name) - 4];
      if (strcasecmp(pExt, ".hdr") == 0)
      {
        *peType = VPLATFORM_FIND_HDR;
      }
      else if (strcasecmp(pExt, ".dat") == 0)
      {
        *peType = VPLATFORM_FIND_DATA;
      }
    }

    vplatform_heap_Free(pFind->hHeap, szConcat);
    
    ElemSize = strlen(dirinfo->d_name) + 1;
    *pszElem = (char *)vplatform_heap_Malloc(pFind->hHeap, ElemSize);
    if (*pszElem == NULL)
    {
      return NUAN_E_OUTOFMEMORY;
    }

    strcpy(*pszElem, dirinfo->d_name);
  }

  return Err;
} /* vplatform_find_Next */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_find_Close(
  VPLATFORM_FIND_H hFind)
{
  NUAN_ERROR Err = NUAN_OK;
  VPLATFORM_FIND_T * pFind = NULL;

  /* Close a directory search */
  if (hFind == NULL)
  {
    return NUAN_E_NULL_POINTER;
  }

  pFind = (VPLATFORM_FIND_T *)hFind;

  /* POSIX implementation */
  if (0 != closedir(pFind->dir))
  {
    return NUAN_E_SYSTEM_ERROR;
  }

  vplatform_heap_Free(pFind->hHeap, pFind);

  return Err;
} /* vplatform_find_Close */

/*------------------------------------------------------------------*/
char * vplatform_CombinePath(
  void *          hHeap,
  const char                      * szDir,
  const char                      * szFile)
{
  NUAN_U32 len = (NUAN_U32)strlen(szDir);
  char *szPath = (char *)
    vplatform_heap_Malloc(hHeap, len + strlen(szFile) + 2);
  if (szPath)
  {
    strcpy(szPath, szDir);
    if ((szPath[len-1] != '\\') && (szPath[len-1] != '/')) 
    {
        strcat(szPath, VPLATFORM_FIND_SZ_DIR_SEP);
    }
    strcat(szPath, szFile);
  }

  return szPath;
} /* vplatform_CombinePath */

#define VTOLOWER(x) ( (((x) >= 'A') && ((x) <= 'Z')) ? (x)+'a'-'A' : (x))

/*------------------------------------------------------------------*/
char * vplatform_CopyToChar(
  void *          hHeap,
  const char                      * szFile)
{
  char *szFileName = (char *)vplatform_heap_Malloc(hHeap, strlen(szFile) + 1);
  if (szFileName)
  {
    size_t i;
    for (i = 0; i <= strlen(szFile); i++)
    {
      szFileName[i] = VTOLOWER(szFile[i]);
    }
  }
  return szFileName;
} /* vplatform_CopyToChar */

/* ******************************************************************
**  END
** ******************************************************************/
