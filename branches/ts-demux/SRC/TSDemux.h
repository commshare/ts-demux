#ifndef TS_DEMUX_H
#define TS_DEMUX_H

#include "../demux.h"
#include "TSParse.h"




int TSDemux_Initalize (DemuxContext* ctx, URLProtocol* h);
int TSDemux_Destroy   (DemuxContext* ctx);
int TSDemux_Probe     (DemuxContext* ctx);
int TSDemux_Metadata  ();
int TSDemux_Seek      ();

#endif // TS_DEMUX_H
