#include "mp_msg.h"
#include "urlprotocol.h"
#include "SRC/TSParse.h"
#include "SRC/TSDemux.h"

URLProtocol* h;

int main(int argc, char** argv)
{
    int ret;
    DemuxContext  ctx;
    DemuxContext* c = &ctx;
    Metadata      m;
    Metadata*     meta = &m;
    AVPacket      p;
    AVPacket*     pack = &p;

    pack->data = NULL;
    pack->bufferlength = 0;
    pack->size = 0;

    h = CreateURLProtocol();
    h->url_open(h\
        , (argc != 2 ? "D:/Private/WorkArea/ts-demux/branches/ts-demux/sample.ts" : argv[1])\
        , 0, NULL);

    /// Test
    if (TSDemux_Open (c, h))
    {
        return -1;
    }
    /// Test Parse Metadata
    if (TSDemux_Mdata (c, meta))
    {
        return -1;
    }
    /// Test Packet Reading
    while ((ret = TSDemux_ReadAV (c, pack)) > 0);    
    if (ret == FAIL)
    {
        fputs("Read failed", stdout);
    }
    /// Test Seek
    ret = TSDemux_Seek(c, 10000);
    if (ret == FAIL)
    {
        fputs("seek failed", stdout);
    }
    ret = TSDemux_Seek(c, 30000);
    if (ret == FAIL)
    {
        fputs("seek failed", stdout);
    }
    ret = TSDemux_Seek(c, 100000);
    if (ret == FAIL)
    {
        fputs("seek failed", stdout);
    }
    TSDemux_Close(c);
    fgetc(stdin);
    return 0;
}
