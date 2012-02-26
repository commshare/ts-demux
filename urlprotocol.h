#ifndef _URLPROTOCOL_H_
#define _URLPROTOCOL_H_

#define URL_RDONLY 0
#define URL_WRONLY 1
#define URL_RDWR   2
#define URL_LIVESTREAM  0x10000

#define SEEK_SIZE 0x10000

/*
#include "protocol/callbackplay.h"
*/

typedef struct URLProtocol {
    /*
     * a short name for the protocol, such as http, file, https, rtmp, mms ...
     */
    const char *name;
    /*
     * check whether this protocol can handle the url
     * return 0 when it can handle the url; else return -1;
     *
     * MUST implement
     */
    int (*url_can_handle)(struct URLProtocol *h, const char *url);
    /*
     * flags - one of URL_RDONLY, URL_WRONLY, URL_RDWR
     * return 0 when opened; -1 (or other error code) on error
     * LPFNINTERRUPT lpfnQuit: return 1 quit;
     *
     * MUST implement
     */
    int (*url_open)(struct URLProtocol *h, const char *url, int flags, void* lpfnQuit);
    /*
     * return the bytes read, -1  (or other error code) on error
     *
     * MUST implement
     */
    int (*url_read)(struct URLProtocol *h, unsigned char *buf, int size);
    /*
     *
     * return the bytes written; -1 (or other error code) on error,
     */
    int (*url_write)(struct URLProtocol *h, const unsigned char *buf, int size);
    /*
     * seek by file position
     *
     * whence - one of SEEK_SET, SEEK_CUR, SEEK_END, SEEK_SIZE
     *          if whence == SEEK_SIZE, return media file length
     *
     * SHOULD implement
     */
    long long (*url_seek)(struct URLProtocol *h, long long pos, int whence);
    /*
     * 0 when closed; -1 on error
     *
     * MUST implement
     */
    int (*url_close)(struct URLProtocol *h);
    /*
     * seek by timestamp
     *
     * streamindex - audio or video stream index
     * timestamp - milliseconds
     *
     * SHOULD implement
     */
    long long (*url_time_seek)(struct URLProtocol *h, int streamindex, long long timestamp);
    /*
     * return 1 live stream, could not perform seek operation; 0, not live, can seek
     *
     * SHOULD implement
     */
    int (*url_is_live)(struct URLProtocol *h);
    struct URLProtocol *next;
    /* for protocol to save private data
     * should be initialised when url_open was called
     * should be released when url_close was called
     */
    int priv_data_size;
    void *priv_data;
}URLProtocol;

/*
 * prot's life circle should be same as the app.
 *
 */
void registerURLProtocol(URLProtocol * prot);
URLProtocol * getURLProtocolByUrl(const char * url);
/*
 * the returned string was malloced from heap, callee should free it after used
 */
char * getExtensionFromUrl(const char *url);

#endif // _URLPROTOCOL_H_
