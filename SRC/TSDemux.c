#include <malloc.h>
#include <string.h>
#include "TSParse.h"
#include "TSDemux.h"
#include "../mp_msg.h"

int TSDemux_Open  (DemuxContext* ctx, URLProtocol* h)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = NULL;

    if (ctx == NULL || h == NULL)
    {
        msg = "Parameter Error";
        goto TSDEMUX_OPEN_RET;
    }

    ctx->priv_data      = (void*)malloc(sizeof(TSDemuxer));
    ctx->priv_data_size = sizeof(TSDemuxer);
    if (ctx->priv_data == NULL)
    {
        msg = "Allocate private data failed";
        goto TSDEMUX_OPEN_RET;
    }

    dmx = (TSDemuxer*)ctx->priv_data;

    dmx->m_Section = (TSection*)malloc(sizeof(TSection));
    dmx->m_PreListHeader = (TSPacket*)malloc(sizeof(TSPacket));
    if (dmx->m_Section == NULL)
    {
        msg = "Allocate a section structure failed";
        goto TSDEMUX_OPEN_RET;
    }
    if (dmx->m_PreListHeader == NULL)
    {
        msg = "Allocate a Pre-read packet list header failed";
        goto TSDEMUX_OPEN_RET;
    }
    memset(dmx->m_Section,       0, sizeof(TSection));
    memset(dmx->m_PreListHeader, 0, sizeof(TSPacket));
    dmx->m_PreListLen = 0;
    dmx->m_Duration   = 0ULL;
    dmx->m_FileSize   = (h->url_is_live(h) == 1 ? 0ULL : h->url_seek(h, 0, SEEK_SIZE));
    dmx->m_Position   = 0ULL;
    dmx->m_PMTPID     = 0U;
    dmx->m_AudioPID   = 0U;
    dmx->m_VideoPID   = 0U;
    dmx->m_Pro        = h;
    if (FAIL == TSParse_InitParser(dmx))
    {
        free (dmx->m_Section);
        dmx->m_Section = NULL;
        msg = "Initialize parser failed";
        goto TSDEMUX_OPEN_RET;
    }

    ret = SUCCESS;
    lev = MSGL_INFO;
    msg = "Open TS demux OK";

TSDEMUX_OPEN_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_Open : %s File Size = %lld\n"\
        , msg, dmx->m_FileSize);
    return ret;
}
int TSDemux_Close (DemuxContext* ctx)
{
    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    dmx->m_Section = NULL;
    if (dmx->m_Section != NULL)
    {
        if (dmx->m_Section->m_Data != NULL)
        {
            free (dmx->m_Section->m_Data);
            dmx->m_Section->m_Data = NULL;
        }
        free (dmx->m_Section);
        dmx->m_Section = NULL;
    }
    if (dmx->m_PreListHeader->m_Next != NULL)
    {
        TSPacket* prev = dmx->m_PreListHeader;
        TSPacket* node = dmx->m_PreListHeader->m_Next;
        while (node != NULL)
        {
            if (node->m_Data != NULL)
            {
                free (node->m_Data);
                node->m_Data = NULL;
            }
            free (node);
            node = prev->m_Next;
        }
    }

    mp_msg(0, MSGL_INFO, "DEMUX ################ TSDemux_Close : TS demux is closed OK\n");
    return SUCCESS;
}
int TSDemux_Mdata (DemuxContext* ctx, Metadata* meta)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    if (FAIL == TSParse_GetSection (dmx))
    {
        msg = "Get section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_PATSection (dmx))
    {
        msg = "Parse PAT section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_GetSection (dmx))
    {
        msg = "Get section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_PMTSection (dmx, meta))
    {
        msg = "Parse PMT section failed";
        goto TSDEMUX_MDATA_RET;
    }

    /// @todo Get file duration

    ret = SUCCESS;
    lev = MSGL_INFO;
    msg = "Parse metadata OK";

TSDEMUX_MDATA_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_Mdata : %s\n", msg);
#if 1
    mp_msg(0, lev, "\t Audio :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->audiocodec, meta->subaudiocodec, meta->audiostreamindex);
    mp_msg(0, lev, "\t Video :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->videocodec, meta->subvideocodec, meta->videostreamindex);
#endif
    return ret;
}
int TSDemux_ReadAV(DemuxContext* ctx, AVPacket* pack)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    while (1)
    {
        if (FAIL == TSParse_GetSection(dmx))
        {
            msg = "Get a section failed";
            goto TSDEMUX_READAV_RET;
        }
        if (dmx->m_Section->m_DataLen == 0ULL)
        {
            msg = "Stream End";
            ret = SUCCESS;
            lev = MSGL_INFO;
            pack->size = 0;
            goto TSDEMUX_READAV_RET;
        }
        if (dmx->m_Section->m_Valid == FALSE)
        {
            continue;
        }
        if (FAIL == TSParse_PESSection (dmx, pack))
        {
            msg = "Parse PES section failed";
            goto TSDEMUX_READAV_RET;
        }
        break;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Read a A/V section OK";

TSDEMUX_READAV_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_ReadAV : %s\n", msg);
    if (ret == SUCCESS && dmx->m_Section->m_DataLen != 0ULL)
    {
#if 0
        mp_msg(0, MSGL_INFO, "\t %s PTS = %-8lld DTS = %-8lld SIZE = %-6d POS = %lld\n"\
            , pack->stream_index == dmx->m_AudioPID ? "Audio" : "Video", pack->pts, pack->dts\
            , pack->size, dmx->m_Section->m_Positon);
#endif
    }
    return pack->size;
}
int TSDemux_Probe (DemuxContext* ctx)
{
    return SUCCESS;
}
int TSDemux_Seek  (DemuxContext* ctx)
{
    /// @todo
    return FAIL;
}



