#include <string.h>
#include "curl/curl.h"
#include "urlvstream.h"
#include "vheap.h"
#include "vcritsec.h"

/* uncomment the next line to trace reference counting of URL resources */
/* #define ELQ_TEST_CHAIN */

/* implement access to URI remote accesses using CURL library and offering 
   a 'file system like' set of primitives (fread, fopen, fclose, fseek)
   performing on the URL data as if it were local */

/* the URL content is fully downloaded when calling fopen */
/* in order to avoid duplication, a reference counting mechanism is provided */

#if defined ELQ_TEST_CHAIN
#define DebugDevice stdout
#endif

/* data is the binary content of the URL, dataLen is the number of bytes, 
   pszUrl is the remote address string of the remote URL */
typedef struct URLCONTENT_DATABUFFER_S {
  void *data;
  size_t dataLen;
  char *pszUrl;
  void *hHeap;
} URLCONTENT_DATABUFFER_T;

/* internally used by the reference counting mechanism, do not bother */
typedef struct URLCONTENT_CHAINLINK_S {
  char *pszUrl;
  URLCONTENT_DATABUFFER_T *databuffer;
  size_t refCount;
  struct URLCONTENT_CHAINLINK_S *next;
} URLCONTENT_CHAINLINK_T;

/* internally used by the reference counting mechanism, do not bother */
typedef struct URLCONTENT_COLLECTION_S {
  URLVSTREAM_HCLASS hClass;
  URLCONTENT_CHAINLINK_T *urlCChain;
  size_t cChainLen;
  URLVSTREAM_CRITSEC_T pCritSec;
} URLCONTENT_COLLECTION_T;

/* global class for memory management, CURL management */
typedef struct URLVSTREAM_CLASS_S {
  void *hHeap;
  void *hCurlHandle;
  URLCONTENT_COLLECTION_T *urlCCollection;
} URLVSTREAM_CLASS_T;

/* just like a file pointer, this is used to keep track of current read position */
typedef struct URLVSTREAM_URL_S {
  URLVSTREAM_HCLASS hClass;
  URLCONTENT_DATABUFFER_T *databuffer;
  size_t cOffset;
  URLVSTREAM_ERROR LastError;
} URLVSTREAM_URL_T;

/* local functions */

/* this function frees local memory from the URL content */
static void DataBufferFree(URLVSTREAM_CLASS_T *pClass,URLCONTENT_DATABUFFER_T *databuffer)
{
  if(databuffer)
  {
    if(databuffer->data)
      vplatform_heap_Free(pClass->hHeap,databuffer->data);
    databuffer->data = NULL;
    if(databuffer->pszUrl)
      vplatform_heap_Free(pClass->hHeap,databuffer->pszUrl);
    databuffer->pszUrl = NULL;
    databuffer->hHeap = NULL;
    vplatform_heap_Free(pClass->hHeap,databuffer);
  }
}

/* CURL callback for writing URL Content */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t nBytesToWrite = size * nmemb;
  size_t newLen = 0;
  char *p = NULL;
  URLCONTENT_DATABUFFER_T *pDatabuffer = (URLCONTENT_DATABUFFER_T *)userdata;
  newLen = pDatabuffer->dataLen + nBytesToWrite;
  p = (char *)vplatform_heap_Realloc(pDatabuffer->hHeap,pDatabuffer->data,newLen);
  if(p != NULL)
  {
    memcpy(p + pDatabuffer->dataLen,ptr,nBytesToWrite);
    pDatabuffer->dataLen = newLen;
    pDatabuffer->data = p;
#if defined ELQ_TEST_CHAIN
    fprintf(DebugDevice,"Writing %lu bytes\n",nBytesToWrite);
#endif
    return nBytesToWrite;
  }
  else return 0;
}

/* this function downloads the URL content and store it into databuffer */
static URLVSTREAM_ERROR DataBufferLoadFromUrl(URLVSTREAM_CLASS_T *pClass, const char *pszUrl, URLCONTENT_DATABUFFER_T **databuffer)
{
  URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
  long http_code = 0;

  URLCONTENT_DATABUFFER_T *pDatabuffer = NULL;
  if(pszUrl == NULL || databuffer == NULL)
  {
    return URLVSTREAM_INVALIDARGUMENT;
  }
  pDatabuffer = (URLCONTENT_DATABUFFER_T *)vplatform_heap_Malloc(pClass->hHeap,sizeof(URLCONTENT_DATABUFFER_T));
  if(pDatabuffer == NULL)
  {
    fRet = URLVSTREAM_OUTOFMEMORY;
    goto endof_DataBufferLoadFromUrl;
  }
  pDatabuffer->dataLen = 0;
  pDatabuffer->data = NULL;
  pDatabuffer->pszUrl = NULL;
  pDatabuffer->hHeap = pClass->hHeap;
  if (curl_easy_setopt(pClass->hCurlHandle, CURLOPT_URL, pszUrl) != CURLE_OK)
  {
    fRet =  URLVSTREAM_INTERNAL;
    goto endof_DataBufferLoadFromUrl;
  }
  if (curl_easy_setopt(pClass->hCurlHandle, CURLOPT_WRITEFUNCTION, write_callback) != CURLE_OK)
  {
    fRet =  URLVSTREAM_INTERNAL;
    goto endof_DataBufferLoadFromUrl;
  }
  if (curl_easy_setopt(pClass->hCurlHandle, CURLOPT_WRITEDATA, pDatabuffer) != CURLE_OK)
  {
    fRet =  URLVSTREAM_INTERNAL;
    goto endof_DataBufferLoadFromUrl;
  }
  if (curl_easy_perform(pClass->hCurlHandle) != CURLE_OK)
  {
    fRet =  URLVSTREAM_INTERNAL;
    goto endof_DataBufferLoadFromUrl;
  }
  if (curl_easy_getinfo(pClass->hCurlHandle, CURLINFO_RESPONSE_CODE , &http_code) != CURLE_OK) 
  {
    fRet =  URLVSTREAM_INTERNAL;
    goto endof_DataBufferLoadFromUrl;
  }
  if(http_code>=400)
  {
    fRet =  URLVSTREAM_READERROR;
    goto endof_DataBufferLoadFromUrl;
  }
  pDatabuffer->pszUrl = (char *)vplatform_heap_Malloc(pClass->hHeap,strlen(pszUrl)+1);
  if(pDatabuffer->data == NULL || pDatabuffer->pszUrl == NULL)
  {
    fRet =  URLVSTREAM_OUTOFMEMORY;
    goto endof_DataBufferLoadFromUrl;
  }
  strcpy(pDatabuffer->pszUrl,pszUrl);
endof_DataBufferLoadFromUrl:
  if(fRet != URLVSTREAM_OK)
    DataBufferFree(pClass,pDatabuffer);
  else
    *databuffer = pDatabuffer;
  return fRet;
}

/* find URLCONTENT_CHAINLINK_T given pszUrl name, if any. This way we check whether the URL content is already stored in memory */ 
static URLCONTENT_CHAINLINK_T * LocateChainLink( URLCONTENT_COLLECTION_T *urlCCollection, const char *pszUrl) 
{
  URLCONTENT_CHAINLINK_T *urlCChainLink = NULL;
  if(urlCCollection == NULL)
    return NULL;
  urlCChainLink = urlCCollection -> urlCChain;
  while(urlCChainLink)
  {
    if(strcmp(urlCChainLink->pszUrl,pszUrl)==0)
      return urlCChainLink;
    urlCChainLink=urlCChainLink->next;
  }
  return NULL;
}

/* add content of pszUrl */
static URLVSTREAM_ERROR UrlContentAdd
  (
  URLCONTENT_COLLECTION_T *urlCCollection,
  URLCONTENT_DATABUFFER_T **pdatabuffer, /* data buffer found or created */
  const char *pszUrl
  ) {
    URLCONTENT_CHAINLINK_T *urlCChainLink = NULL;
    URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
    URLVSTREAM_CLASS_T *pClass = (URLVSTREAM_CLASS_T *) urlCCollection->hClass;
#if defined ELQ_TEST_CHAIN
    fprintf(DebugDevice,"in UrlContentAdd: pszUrl = %s\n",pszUrl);
#endif
    if(urlCCollection == NULL)
      return URLVSTREAM_NOTINIT;
    (void)vplatform_critsec_Enter(urlCCollection->pCritSec);
    if ((urlCChainLink = LocateChainLink(urlCCollection,pszUrl)) == NULL)
    {
      fRet = DataBufferLoadFromUrl(pClass,pszUrl,pdatabuffer);
      if(fRet == URLVSTREAM_OK)
      {
        urlCCollection->cChainLen++;
        if((urlCChainLink = (URLCONTENT_CHAINLINK_T *) vplatform_heap_Malloc(pClass->hHeap,sizeof(URLCONTENT_CHAINLINK_T)))!=NULL)
        {
          urlCChainLink->refCount = 0;
          urlCChainLink->pszUrl = NULL;
          urlCChainLink->databuffer = *pdatabuffer;

          urlCChainLink->pszUrl = (char *)vplatform_heap_Malloc(pClass->hHeap, sizeof(char) * (strlen(pszUrl)+1));
          if(urlCChainLink->pszUrl == NULL)
          {
            fRet = URLVSTREAM_OUTOFMEMORY;
            vplatform_heap_Free(pClass->hHeap,urlCChainLink);
            DataBufferFree(pClass,*pdatabuffer);
          }
          else
          {
            (void)strcpy(urlCChainLink->pszUrl,pszUrl);
            urlCChainLink->next = urlCCollection -> urlCChain;
            urlCChainLink->refCount = 1;
#if defined ELQ_TEST_CHAIN
            fprintf(DebugDevice,"Reference counting after creation: %d\n",urlCChainLink -> refCount);
#endif
            urlCCollection->urlCChain = urlCChainLink;
          }
        }
        else
        {
          fRet = URLVSTREAM_OUTOFMEMORY;
          DataBufferFree(pClass,*pdatabuffer);
        }
      }
    }
    else 
    {
      *pdatabuffer = urlCChainLink -> databuffer;
      urlCChainLink -> refCount++;
#if defined ELQ_TEST_CHAIN
      fprintf(DebugDevice,"Reference counting after insertion %d\n",urlCChainLink -> refCount);
#endif
    }
    (void)vplatform_critsec_Leave(urlCCollection->pCritSec);
    return fRet;
}

/* remove content of pszUrl */
static void UrlContentRemove( 
  URLCONTENT_COLLECTION_T *urlCCollection,
  const char *pszUrl
) {
  void *elem = NULL;
  URLCONTENT_CHAINLINK_T *urlCChainLink = NULL;
  URLVSTREAM_CLASS_T *pClass = (URLVSTREAM_CLASS_T *) urlCCollection->hClass;
#if defined ELQ_TEST_CHAIN
  fprintf(DebugDevice,"in UrlContentRemove: pszUrl = %s\n",pszUrl);
#endif
  if(urlCCollection == NULL)
    return;
  (void)vplatform_critsec_Enter(urlCCollection->pCritSec);
  urlCChainLink = urlCCollection -> urlCChain;
  if(urlCChainLink != NULL)
  {
    elem = urlCChainLink -> databuffer;
    if(strcmp(urlCChainLink->pszUrl,pszUrl)==0)
      /* first element of urlCChainLink */
    {
      urlCChainLink -> refCount--;
#if defined ELQ_TEST_CHAIN
      fprintf(DebugDevice,"reference count after removal: %d\n",urlCChainLink->refCount);
#endif
      if(urlCChainLink->refCount == 0)
      {
        if(elem != NULL)
        {
          DataBufferFree(pClass,(URLCONTENT_DATABUFFER_T *)elem);
        }
        urlCCollection->cChainLen--;
        vplatform_heap_Free(pClass->hHeap,urlCChainLink->pszUrl);
        if(urlCChainLink->next){ 
          URLCONTENT_CHAINLINK_T *c = urlCChainLink;
          urlCCollection->urlCChain = urlCChainLink->next;
          vplatform_heap_Free(pClass->hHeap,c);
        }
        else  {
          vplatform_heap_Free(pClass->hHeap,urlCChainLink);
          urlCCollection->urlCChain = NULL;
        }
      }
    }
    else /* this is not the first element of urlCChainLink */
    {
      while(urlCChainLink -> next != NULL)
      {
        elem = urlCChainLink->next->databuffer;
        if(strcmp(urlCChainLink->next->pszUrl,pszUrl)==0)
        {
          urlCChainLink->next->refCount--;
#if defined ELQ_TEST_CHAIN
          fprintf(DebugDevice,"reference count after removal: %d\n",urlCChainLink->next->refCount);
#endif
          if(urlCChainLink->next->refCount == 0)
          {
            URLCONTENT_CHAINLINK_T *c = urlCChainLink->next->next;
            if(elem != NULL)
            {
              DataBufferFree(pClass,(URLCONTENT_DATABUFFER_T *)elem);
            }
            urlCCollection->cChainLen--;
            vplatform_heap_Free(pClass->hHeap,urlCChainLink->next->pszUrl);
            vplatform_heap_Free(pClass->hHeap,urlCChainLink->next);
            urlCChainLink->next = c;
          }
          break;
        }
        urlCChainLink = urlCChainLink->next;
      }
    }
  }
  (void)vplatform_critsec_Leave(urlCCollection->pCritSec);
}

static URLVSTREAM_ERROR UrlCollectionIni( URLVSTREAM_CLASS_T *pClass, URLCONTENT_COLLECTION_T **puc )
{
    URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
    URLCONTENT_COLLECTION_T *urlCCollection = NULL;
    if (puc!=NULL)
      *puc = NULL; /* return value initialization */

    urlCCollection = (URLCONTENT_COLLECTION_T *)vplatform_heap_Malloc(pClass->hHeap, sizeof(URLCONTENT_COLLECTION_T));
    if(urlCCollection != NULL)
    {
      urlCCollection->urlCChain = NULL;
      urlCCollection->cChainLen=0;
      urlCCollection->pCritSec=NULL;
      urlCCollection->hClass = (URLVSTREAM_HCLASS)pClass;
      vplatform_critsec_ObjOpen(NULL,pClass->hHeap,&urlCCollection->pCritSec);
      if(urlCCollection->pCritSec == NULL)
      {
        return URLVSTREAM_INTERNAL;
      }
      *puc = urlCCollection;
    }
    else fRet = URLVSTREAM_OUTOFMEMORY;
    return fRet;
}

static void UrlCollectionDelete(URLCONTENT_COLLECTION_T *urlCCollection) 
{
  URLVSTREAM_CLASS_T *pClass = (URLVSTREAM_CLASS_T *) urlCCollection->hClass;
  if(urlCCollection!=NULL)
  {
    if(urlCCollection->urlCChain != NULL)
    {
      URLCONTENT_CHAINLINK_T *urlCChainLink = urlCCollection->urlCChain;
      while(NULL != urlCChainLink)
      {
        size_t RefCnt = urlCChainLink->refCount;
        URLCONTENT_CHAINLINK_T *next = urlCChainLink->next;
        if(RefCnt != 0)
        {
          UrlContentRemove(urlCCollection,urlCChainLink->pszUrl);
          if(RefCnt == 1)
            urlCChainLink = next;
        }
      }
    }
    urlCCollection->urlCChain = NULL;
    if(urlCCollection->pCritSec)
      (void)vplatform_critsec_ObjClose(urlCCollection->pCritSec);
    vplatform_heap_Free(pClass->hHeap,urlCCollection);
  }
}

/* exported functions */

URLVSTREAM_ERROR vplatform_urlstream_Initialize(void *hHeap, URLVSTREAM_HCLASS *hClass)
{
  URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
  URLVSTREAM_CLASS_T *pClass = NULL;
  if(hClass == NULL)
    return URLVSTREAM_NOTINIT;
  pClass = (URLVSTREAM_CLASS_T *)vplatform_heap_Calloc(hHeap,1, sizeof(URLVSTREAM_CLASS_T));
  if(pClass == NULL)
    return URLVSTREAM_OUTOFMEMORY;
  pClass->hHeap = hHeap;
  pClass->hCurlHandle = NULL;
  pClass->urlCCollection = NULL;
  fRet = UrlCollectionIni(pClass,&pClass->urlCCollection);
  if(fRet != URLVSTREAM_OK)
    goto end_of_UrlVStreamInitialize;
  pClass->hCurlHandle =  curl_easy_init();
  if(pClass->hCurlHandle == NULL)
  {
    fRet = URLVSTREAM_INTERNAL;
    goto end_of_UrlVStreamInitialize;
  }  
end_of_UrlVStreamInitialize:
  if(fRet == URLVSTREAM_OK)
  {
    *hClass = (URLVSTREAM_HCLASS)pClass;
  }
  else {
    vplatform_urlstream_Uninitialize((URLVSTREAM_HCLASS)pClass);
  }
  return fRet;
}

void vplatform_urlstream_Uninitialize(URLVSTREAM_HCLASS hClass) 
{
  URLVSTREAM_CLASS_T *pClass = (URLVSTREAM_CLASS_T *)hClass;
  if(pClass)
  {
    if(pClass->hCurlHandle)
      curl_easy_cleanup(pClass->hCurlHandle);
    pClass->hCurlHandle = NULL;
    if(pClass->urlCCollection)
      UrlCollectionDelete(pClass->urlCCollection);
    pClass->urlCCollection = NULL;
    vplatform_heap_Free(pClass->hHeap,pClass);
  }
}

URLVSTREAM_ERROR vplatform_urlstream_Open(URLVSTREAM_HCLASS hClass, const char *pszUri, URLVSTREAM_HURL *hUri)
{
  URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
  URLVSTREAM_URL_T *pUrl = NULL;
  URLVSTREAM_CLASS_T *pClass = (URLVSTREAM_CLASS_T *)hClass;
  if(pClass)
  {
    pUrl = (URLVSTREAM_URL_T *)vplatform_heap_Malloc(pClass->hHeap,sizeof(URLVSTREAM_URL_T));
    if(pUrl == NULL)
    {
      return URLVSTREAM_OUTOFMEMORY;
    }
    pUrl->hClass = hClass;
    pUrl->cOffset = 0;
    pUrl->databuffer = NULL;
    pUrl->LastError = URLVSTREAM_OK;
    fRet = UrlContentAdd(pClass->urlCCollection,&pUrl->databuffer,pszUri);
    if(fRet != URLVSTREAM_OK)
      goto endof_UrlVStream_Open;
  }
  else fRet = URLVSTREAM_NOTINIT;
endof_UrlVStream_Open:
  if(fRet != URLVSTREAM_OK)
    vplatform_urlstream_Close(pUrl);
  else
  {
    if(pUrl)
      *hUri = (URLVSTREAM_HURL)pUrl;
  }
  return fRet;
}

void vplatform_urlstream_Close(URLVSTREAM_HURL hUri) {
  URLVSTREAM_URL_T *pUrl = NULL;
  URLVSTREAM_CLASS_T *pClass = NULL;
  pUrl = (URLVSTREAM_URL_T *)hUri;
  if(pUrl)
  {
    pClass= (URLVSTREAM_CLASS_T *)pUrl->hClass;
    if(pClass)
    {
      if(pUrl->databuffer)
        UrlContentRemove(pClass->urlCCollection,pUrl->databuffer->pszUrl);
      vplatform_heap_Free(pClass->hHeap,pUrl);
    }
  }
}

size_t vplatform_urlstream_Read(void *pBuffer, size_t  cElementBytes, size_t cElements, URLVSTREAM_HURL hUri)
{
  size_t nBytesToRead = cElementBytes*cElements;
  URLVSTREAM_URL_T *pUrl = (URLVSTREAM_URL_T *)hUri;
  char *p = NULL;
  if(pBuffer == NULL)
    return 0;
  if(pUrl==NULL || pUrl->databuffer == NULL || pUrl->databuffer->data == NULL)
    return 0;
  if(pUrl->cOffset >=  pUrl->databuffer->dataLen)
    return 0;
  p = (char *)pUrl->databuffer->data + pUrl->cOffset;
  if(pUrl->databuffer->dataLen-pUrl->cOffset < nBytesToRead)
    nBytesToRead = pUrl->databuffer->dataLen-pUrl->cOffset;
  memcpy(pBuffer,p,nBytesToRead);
  pUrl->cOffset+=nBytesToRead;
  return nBytesToRead;
}

URLVSTREAM_ERROR vplatform_urlstream_Seek(URLVSTREAM_HURL hUri, size_t cOffset, URLVSTREAM_ORIGIN eOrigin, URLVSTREAM__DIRECTION eDirection)
{
  URLVSTREAM_ERROR fRet = URLVSTREAM_OK;
  URLVSTREAM_URL_T *pUrl = (URLVSTREAM_URL_T *)hUri;
  if(pUrl==NULL || pUrl->databuffer == NULL || pUrl->databuffer->data == NULL)
    return URLVSTREAM_SEEKERROR;
  switch(eOrigin)
  {
  case URLVSTREAM_SEEK_SET:
    if(eDirection == URLVSTREAM_BACKWARD)
    {
      if(cOffset>0)
        fRet = URLVSTREAM_SEEKERROR;
    }
    else if(eDirection == URLVSTREAM_FORWARD)
    {
      pUrl->cOffset = cOffset;
    }
    else fRet = URLVSTREAM_INVALIDARGUMENT;
    break;
  case URLVSTREAM_SEEK_CUR:
    if(eDirection == URLVSTREAM_BACKWARD)
    {
      if(cOffset<=pUrl->cOffset)
        pUrl->cOffset-=cOffset;
      else
        fRet = URLVSTREAM_SEEKERROR;
    }
    else if(eDirection == URLVSTREAM_FORWARD)
    {
      pUrl->cOffset += cOffset;
    }
    else fRet =  URLVSTREAM_INVALIDARGUMENT;
    break;
  case URLVSTREAM_SEEK_END:
    if(eDirection == URLVSTREAM_FORWARD)
    {
      pUrl->cOffset = pUrl->databuffer->dataLen + cOffset;
    }
    else if(eDirection == URLVSTREAM_BACKWARD)
    {
      if(cOffset <= pUrl->databuffer->dataLen)
        pUrl->cOffset = pUrl->databuffer->dataLen - cOffset;
      else fRet = URLVSTREAM_SEEKERROR;
    }
    else fRet = URLVSTREAM_INVALIDARGUMENT;
    break;
  default: fRet = URLVSTREAM_INVALIDARGUMENT;
  }
  pUrl->LastError = fRet;
  return fRet;
}

size_t vplatform_urlstream_GetSize(URLVSTREAM_HURL hUri)
{
  URLVSTREAM_URL_T *pUrl = (URLVSTREAM_URL_T *)hUri;
  if(pUrl==NULL || pUrl->databuffer == NULL || pUrl->databuffer->data == NULL)
    return 0;
  return pUrl->databuffer->dataLen;
}

URLVSTREAM_ERROR vplatform_urlstream_Error(URLVSTREAM_HURL hUri)
{
  URLVSTREAM_URL_T *pUrl = (URLVSTREAM_URL_T *)hUri;
  if(pUrl==NULL || pUrl->databuffer == NULL || pUrl->databuffer->data == NULL)
    return URLVSTREAM_INTERNAL;
  return pUrl->LastError;
}

URLVSTREAM_BOOL  UrlIsUrl(const char *AbsoluteUrlName, URLVSTREAM_BOOL *bRemote)
{
  const struct {
    const char * const name;
    URLVSTREAM_BOOL bRemote;
  } Prefix[] = {
    {"http://",URLVSTREAM_TRUE},
    {"ftp://",URLVSTREAM_TRUE},
    {"https://",URLVSTREAM_TRUE},
    {"file://",URLVSTREAM_FALSE}
  };
  unsigned int i = 0;
  if(bRemote) *bRemote = URLVSTREAM_FALSE;
  if(AbsoluteUrlName == NULL || AbsoluteUrlName[0] == '\0')
    return URLVSTREAM_FALSE;
  for(i=0;i<sizeof(Prefix)/sizeof(Prefix[0]);i++)
  {
    if(strncmp(AbsoluteUrlName,Prefix[i].name,strlen(Prefix[i].name)) == 0)
    {
      if(bRemote) *bRemote = Prefix[i].bRemote;
      return URLVSTREAM_TRUE;
    }
  }
  return URLVSTREAM_FALSE;
}

const char *UrlVStreamGetName(URLVSTREAM_HURL hUri)
{
  URLVSTREAM_URL_T *pUrl = (URLVSTREAM_URL_T *)hUri;
  if(pUrl == NULL || pUrl->databuffer == NULL) return NULL;
  return pUrl->databuffer->pszUrl;
}

