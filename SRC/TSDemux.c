#include <malloc.h>

#include "TSParse.h"
#include "TSDemux.h"
#include "../mp_msg.h"

static BOOL ts_demux_get_ts_packet ();

int ts_demux_open  (DemuxContext* ctx, URLProtocol* h)
{
    I8  errtyp = MSGL_ERR;
    I8  errstr[64];
    TSDemuxer* dmx = NULL;

    if (ctx == NULL || h == NULL)
    {
        strcpy(errstr, "Parameter Error");
        goto TS_DEMUX_OPEN_ERROR;
    }

    ctx->priv_data      = (void*)malloc(sizeof(TSDemuxer));
    ctx->priv_data_size = sizeof(TSDemuxer);

    dmx = (TSDemuxer*)ctx->priv_data;
    dmx->m_Pro = h;

TS_DEMUX_OPEN_ERROR:
    mp_msg(0, errtyp, "DEMUX ################ %s\n", errstr);
    return FAIL;
TS_DEMUX_OPEN_OK:
    mp_msg(0, errtyp, "DEMUX ################ %s\n", errstr);
}
int ts_demux_close (DemuxContext* ctx);
int ts_demux_probe (DemuxContext* ctx)
{
    I8  errtyp;
    I8  errstr[64];
    int syncpos = 0;
    UI8 flag    = 0U;
    int ret     = 0;

    TSDemuxer  * dmx = NULL;
    URLProtocol* pro = NULL;

    I8  prob_buf[5 * TS_PACKET_SIZE_MAX + 1];

    /// Check Parameters
    errtyp = MSGL_ERR;
    if ((ctx == NULL) || ((dmx = (TSDemuxer*)ctx->priv_data) == NULL)\
        || ((pro = dmx->m_Pro) == NULL))
    {
        strcpy(errstr, "Parameter Errror");
        goto TS_DEMUX_PROBE_ERROR;
    }

    /// Find The 1st Sync Byte
    for (syncpos = 0; syncpos < TS_PACKET_SIZE_MAX; ++syncpos)
    {
        if ((pro->url_read(pro, &flag, 1)) < 0)
        {
            strcpy(errstr, "Calling url_read Failed");
            goto TS_DEMUX_PROBE_ERROR;
        }
        if (flag == 0x47)
        {
            break;
        }
    }
    dmx->m_StartPos = syncpos;


    /// Probe Packet Size
    prob_buf[0] = flag;
    ret = pro->url_read(pro, prob_buf + 1, (3 * TS_PACKET_SIZE_MAX));
    if (0 == ret)
    {
        strcpy(errstr, "File End Unexpected");
        goto TS_DEMUX_PROBE_ERROR;
    }

    if ((prob_buf[TS_PACKET_SIZE_188] == 0x47)\
        || (prob_buf[2 * TS_PACKET_SIZE_188] == 0x47)\
        || (prob_buf[3 * TS_PACKET_SIZE_188] == 0x47))
    {
        dmx->m_PacketLens = TS_PACKET_SIZE_188;
    }
    else if ((prob_buf[TS_PACKET_SIZE_192] == 0x47)\
         || (prob_buf[2 * TS_PACKET_SIZE_192] == 0x47)\
         || (prob_buf[3 * TS_PACKET_SIZE_192] == 0x47))
    {
        dmx->m_PacketLens = TS_PACKET_SIZE_192;
    }
    else if ((prob_buf[TS_PACKET_SIZE_204] == 0x47)\
         || (prob_buf[2 * TS_PACKET_SIZE_204] == 0x47)\
         || (prob_buf[3 * TS_PACKET_SIZE_204] == 0x47))
    {
        dmx->m_PacketLens = TS_PACKET_SIZE_204;
    }
    else
    {
        strcpy(errstr, "Probe Failed, Not a TS File");
        goto TS_DEMUX_PROBE_ERROR;
    }

TS_DEMUX_PROBE_OK:
    mp_msg(0, MSGL_V, "DEMUX ################ ts_demux_probe : Probe Successful\n");
    return 0;
TS_DEMUX_PROBE_ERROR:
    mp_msg(0, errtyp, "DEMUX ################ ts_demux_probe : %s\n", errstr);
}

