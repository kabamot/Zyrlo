#ifndef __LOCALSTREAM_H__
#define __LOCALSTREAM_H__

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

#if defined( __cplusplus )
extern "C"
{
#endif

/* ******************************************************************
**  GLOBAL FUNCTION PROTOTYPES
** ******************************************************************/

/* ------------------------------------------------------------------
**  Data access services (local only) - only Open function
** -----------------------------------------------------------------*/


/* ------------------------------------------------------------------
** interface functions                                                 
** -----------------------------------------------------------------*/

/*------------------------------------------------------------------*/
NUAN_ERROR vplatform_localstream_Open(
    void       * hDataClass,
    void       * hHeap,
    const char * szName,
    const char * szMode,
    void *     * phStream
);

#if defined( __cplusplus )
}
#endif

/* ******************************************************************
**  END
** ******************************************************************/

#endif /* #ifndef __LOCALSTREAM_H__ */
