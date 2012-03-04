#include <malloc.h>
#include <string.h>
#include "TSParse.h"
#include "TSDemux.h"
#include "../mp_msg.h"

/// @brief Get stream duration
/// @pre   Audio's PID and video's PID have been set
static BOOL TSDemux_GetDuration (TSDemuxer* dmx)
{
    return SUCCESS;
}

int TSDemux_Open  (DemuxContext* ctx, URLProtocol* h)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    TSDemuxer* dmx = NULL;

    if (ctx == NULL || h == NULL)
    {
        strcpy(msg, "Parameter Error");
        goto TSDEMUX_OPEN_RET;
    }

    ctx->priv_data      = (void*)malloc(sizeof(TSDemuxer));
    ctx->priv_data_size = sizeof(TSDemuxer);
    if (ctx->priv_data == NULL)
    {
        strcpy(msg, "Allocate private data failed");
        goto TSDEMUX_OPEN_RET;
    }

    dmx = (TSDemuxer*)ctx->priv_data;

    dmx->m_Duration           = 0ULL;
    dmx->m_FileSize           = (h->url_is_live(h) == 1 ? 0ULL : h->url_seek(h, 0, SEEK_SIZE));
    dmx->m_Position           = 0ULL;
    dmx->m_PMTPID             = 0U;
    dmx->m_AudioPID           = 0U;
    dmx->m_VideoPID           = 0U;
    dmx->m_Section.m_StreamID = 0U;
    dmx->m_Section.m_Type     = 0U;
    dmx->m_Section.m_Data     = NULL;
    dmx->m_Section.m_DataLen  = 0ULL;
    dmx->m_Section.m_BuffLen  = 0ULL;
    dmx->m_PktList.m_Next     = NULL;
    dmx->m_PktList.m_Pack     = NULL;
    dmx->m_Pro                = h;

    if (FAIL == TSParse_InitParser(dmx))
    {
        strcpy(msg, "Initialize parser failed");
        goto TSDEMUX_OPEN_RET;
    }

    ret = SUCCESS;
    lev = MSGL_INFO;
    strcpy(msg, "Open TS demux OK");

TSDEMUX_OPEN_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_Open : %s\n", msg);
    return ret;
}
int TSDemux_Close (DemuxContext* ctx)
{
    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    if (dmx->m_Section.m_Data != NULL)
    {
        free (dmx->m_Section.m_Data);
        dmx->m_Section.m_Data = NULL;
    }
    if (dmx->m_PktList.m_Next != NULL)
    {
        PrePacket* prev = &dmx->m_PktList;
        PrePacket* node = dmx->m_PktList.m_Next;
        while (node != NULL)
        {
            if (node->m_Pack != NULL)
            {
                if (node->m_Pack->m_Data != NULL)
                {
                    free (node->m_Pack->m_Data);
                    node->m_Pack->m_Data = NULL;
                }
                free (node->m_Pack);
                node->m_Pack = NULL;
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
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    while (dmx->m_PMTPID != 0x00)
    {
        if (FAIL == TSParse_GetSection (dmx))
        {
            strcpy(msg, "Get section failed");
            goto TSDEMUX_MDATA_RET;
        }
    }
    if (FAIL == TSParse_PMTSection (dmx, meta))
    {
        strcpy(msg, "Parse PMT section failed");
        goto TSDEMUX_MDATA_RET;
    }

    /// @todo Get file duration
    if (FAIL == TSDemux_GetDuration(dmx))
    {
        strcpy(msg, "Get duration failed");
        goto TSDEMUX_MDATA_RET;
    }

    ret = SUCCESS;
    lev = MSGL_INFO;
    strcpy(msg, "Parse metadata OK");

TSDEMUX_MDATA_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_Mdata : %s\n", msg);
    return ret;
}
int TSDemux_ReadAV(DemuxContext* ctx, AVPacket* pack)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    while (1)
    {
        UI8 sid;
        if (FAIL == TSParse_GetSection(dmx))
        {
            strcpy(msg, "Get a section failed");
            goto TSDEMUX_READAV_RET;
        }
        sid = dmx->m_Section.m_StreamID;
        if ((sid != STREAM_ID_PROGRAM_MAP)
         || (sid != STREAM_ID_PADDING)
         || (sid != STREAM_ID_PRIVATE_2)
         || (sid != STREAM_ID_ECM)
         || (sid != STREAM_ID_EMM)
         || (sid != STREAM_ID_PRO_DIREC))
        {
            if (FAIL == TSParse_PESSection (dmx, pack))
            {
                strcpy(msg, "Parse PES section failed");
                goto TSDEMUX_READAV_RET;
            }
            break;
        }
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Read a A/V section OK");

TSDEMUX_READAV_RET:
    mp_msg(0, lev, "DEMUX ################ TSDemux_ReadAV : %s\n", msg);
    return ret;
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



