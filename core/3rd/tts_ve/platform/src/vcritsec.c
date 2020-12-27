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
#include "vcritsec.h"

#include <stdlib.h>

/* ******************************************************************
**  COMPILER DIRECTIVES
** ******************************************************************/


/* ******************************************************************
**  DEFINITIONS
** ******************************************************************/

/*lint -esym(715, hHeap, hCritSecClass, hCritSec,hCSClass) 
**           not used in this implementation */


/* ******************************************************************
**  FUNCTIONS
** ******************************************************************/

/* ------------------------------------------------------------------
**  Critical Sections service
** -----------------------------------------------------------------*/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_ObjOpen(
  void *      hCCritSec,
  void *      hHeap,
  void **     phCritSec)
{
  /* Stub implementation */
  (void) hCCritSec;

  /* Validate args */
  if (! phCritSec) {
    return NUAN_E_INVALIDARG;
  }

  /* Set output arg */
  *phCritSec = (void * ) 0x1234abcd;

  return NUAN_OK;
} /* vplatform_critsec_ObjOpen */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_ObjClose(
  void *  hCritSec)
{
  (void) hCritSec;
  return NUAN_OK;
} /* vplatform_critsec_ObjClose */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_Enter(
  void *  hCritSec)
{
  (void) hCritSec;
  return NUAN_OK;
} /* vplatform_critsec_Enter */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_Leave(
  void *  hCritSec)
{
  (void) hCritSec;
  return NUAN_OK;
} /* vplatform_critsec_Leave */


/*===================================================================
**  Definition of static interfaces
**==================================================================*/
/*
static const VE_CRITSEC_INTERFACE     ICritSec = {
  vplatform_critsec_ObjOpen,
  vplatform_critsec_ObjClose,
  vplatform_critsec_Enter,
  vplatform_critsec_Leave
};
*/

/* ******************************************************************
**  PUBLIC INTERFACE RETRIEVAL FUNCTIONS
** ******************************************************************/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_GetInterface(
  VE_INSTALL       * pInstall,
  VPLATFORM_RESOURCES * pResources)
{
  (void) pResources;
  pInstall->pICritSec = NULL; /*&ICritSec;*/
  pInstall->hCSClass = NULL;
  return NUAN_OK;
} /* vplatform_critsec_GetInterface */

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_critsec_ReleaseInterface(
  void  * hCSClass)
{
  (void) hCSClass;
  return NUAN_OK;
} /* vplatform_critsec_ReleaseInterface */


/* ******************************************************************
**  END
** ******************************************************************/
