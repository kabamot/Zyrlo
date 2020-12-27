#if !defined URLVSTREAM_H_INCLUDED
#define URLVSTREAM_H_INCLUDED

#include "stdlib.h"
#include "vplatform.h"

#define URLVSTREAM_TRUE NUAN_TRUE
#define URLVSTREAM_FALSE NUAN_FALSE
#define URLVSTREAM_OK NUAN_OK
#define URLVSTREAM_NOTINIT NUAN_E_NOTINITIALIZED
#define URLVSTREAM_OUTOFMEMORY NUAN_E_OUTOFMEMORY
#define URLVSTREAM_INTERNAL NUAN_E_SYSTEM_ERROR
#define URLVSTREAM_INVALIDARGUMENT NUAN_E_INVALIDARG
#define URLVSTREAM_READERROR NUAN_E_FILEREADERROR
#define URLVSTREAM_SEEKERROR NUAN_E_FILESEEK
#define URLVSTREAM_SEEK_SET VE_STREAM_SEEK_SET
#define URLVSTREAM_SEEK_CUR VE_STREAM_SEEK_CUR
#define URLVSTREAM_SEEK_END VE_STREAM_SEEK_END
#define URLVSTREAM_BACKWARD VE_STREAM_BACKWARD    
#define URLVSTREAM_FORWARD VE_STREAM_FORWARD    

typedef NUAN_ERROR URLVSTREAM_ERROR;
typedef NUAN_BOOL URLVSTREAM_BOOL;
typedef void * URLVSTREAM_HURL;
typedef void * URLVSTREAM_HCLASS;
typedef void * URLVSTREAM_CRITSEC_T;
typedef VE_STREAM_ORIGIN URLVSTREAM_ORIGIN;
typedef VE_STREAM_DIRECTION     URLVSTREAM__DIRECTION;

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

URLVSTREAM_ERROR vplatform_urlstream_Initialize(void *hHeap, URLVSTREAM_HCLASS *hClass);
void vplatform_urlstream_Uninitialize(URLVSTREAM_HCLASS hClass);

URLVSTREAM_ERROR vplatform_urlstream_Open(URLVSTREAM_HCLASS hClass, const char *pszUri, URLVSTREAM_HURL *hUri);
void vplatform_urlstream_Close(URLVSTREAM_HURL hUri);
size_t vplatform_urlstream_Read(void *pBuffer, size_t  cElementBytes, size_t cElements, URLVSTREAM_HURL hUri);
URLVSTREAM_ERROR vplatform_urlstream_Seek(URLVSTREAM_HURL hUri, size_t cOffset, URLVSTREAM_ORIGIN eOrigin, URLVSTREAM__DIRECTION eDirection);
size_t vplatform_urlstream_GetSize(URLVSTREAM_HURL hUri);
URLVSTREAM_ERROR vplatform_urlstream_Error(URLVSTREAM_HURL hUri);
URLVSTREAM_BOOL  UrlIsUrl(const char *AbsoluteUrlName, URLVSTREAM_BOOL *bRemote);
const char *UrlVStreamGetName(URLVSTREAM_HURL hUri);
#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* URLVSTREAM_H_INCLUDED*/

