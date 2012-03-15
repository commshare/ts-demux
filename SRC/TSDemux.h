#ifndef TS_DEMUX_H
#define TS_DEMUX_H

#include "TSParse.h"
#include "../demux.h"
#include "../mp_msg.h"
#include "../avformat.h"

int TSDemux_Open    (DemuxContext* ctx, URLProtocol* h);
int TSDemux_Close   (DemuxContext* ctx);
int TSDemux_Mdata   (DemuxContext* ctx, Metadata* meta);
int TSDemux_ReadAV  (DemuxContext* ctx, AVPacket* pack);
int TSDemux_Probe   (DemuxContext* ctx);
int TSDemux_Seek    (DemuxContext* ctx, long long pos);

#endif/*TS_DEMUX_H*/
