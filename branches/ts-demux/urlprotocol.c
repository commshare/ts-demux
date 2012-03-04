#include <stdio.h>
#include "urlprotocol.h"

#ifndef BASE_DEFINED
#define BASE_DEFINED
#define SUCCESS               0     ///< Return on success
#define FAIL                 -1     ///< Retrun on error
#define FALSE                 0     ///< False flag
#define TRUE                  1     ///< True flag
#endif/*BASE_DEFINED*/

#ifndef BASE_TYPEDEF
#define BASE_TYPEDEF
typedef char               I8, BOOL;
typedef short              I16;
typedef long               I32;
typedef long long          I64;
typedef unsigned char      UI8;
typedef unsigned short     UI16;
typedef unsigned long      UI32;
typedef unsigned long long UI64;
#endif/*BASE_TYPEDEF*/

typedef struct URLPrivData
{
    FILE* fp;
    UI64  filesize;
    UI64  currpos;
}URLPrivData;

int url_open (URLProtocol *h, const char* path, int flags, void* quit)
{
    URLPrivData* d = NULL;
    h->priv_data = d = (URLPrivData*)malloc(sizeof(URLPrivData));
    d->currpos   = 0ULL;
    d->filesize  = 0ULL;
    d->fp = fopen(path, "rb+");
    fseek(d->fp, 0, SEEK_END);
    d->filesize = ftell(d->fp);
    fseek(d->fp, 0, SEEK_SET);

    h->priv_data_size = sizeof(URLPrivData);
    return 0;
}
int url_close(URLProtocol* h)
{
    URLPrivData* d = (URLPrivData*)h->priv_data;
    fclose(d->fp);
    d->fp = NULL;
    return 0;
}
int url_read (URLProtocol *h, unsigned char *buf, int size)
{
    URLPrivData* d = (URLPrivData*)h->priv_data;
    int ret = fread(buf, 1, size, d->fp);
    d->currpos += ret;
    return ret;
}
I64 url_seek (URLProtocol *h, long long pos, int whence)
{
    URLPrivData* d = (URLPrivData*)h->priv_data;
    int ret ;

    if (whence == SEEK_SIZE)
    {
        return (long long)d->filesize;
    }

    ret = fseek(d->fp, (long)pos, whence);
    return (long long)ret;
}
int url_is_live (URLProtocol* h)
{
    h = NULL;
    return 0;
}

URLProtocol* CreateURLProtocol ()
{
    URLProtocol* h = (URLProtocol*)malloc(sizeof(URLProtocol));

    h->url_open         = url_open;
    h->url_close        = url_close;
    h->url_read         = url_read;
    h->url_write        = NULL;
    h->url_seek         = url_seek;
    h->url_is_live      = url_is_live;
    h->url_can_handle   = NULL;
    h->url_time_seek    = NULL;
    h->name             = "file";
    h->next             = NULL;
    h->priv_data        = NULL;
    h->priv_data_size   = 0;

    return h;
}

