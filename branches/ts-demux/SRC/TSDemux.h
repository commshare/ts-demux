#ifndef TS_DEMUX_H
#define TS_DEMUX_H

#include "TSParse.h"
#include "../demux.h"
#include "../mp_msg.h"
#include "../avformat.h"

int TSDemux_Open    (DemuxContext* ctx, URLProtocol* h);
int TSDemux_Close   (DemuxContext* ctx);
int TSDemux_Probe   (DemuxContext* ctx);

/// @brief Parse metadata
///
/// From file start, find and parse PAT and PMT in order , to get  informations of audio and video\n
/// If the protocol is VOD but not LIVE, get the first keyframe PTS and last keyframe to calculate\n
/// file duration.
int TSDemux_Mdata   (DemuxContext* ctx, Metadata* meta)
{
    I8  retval = FAIL;
    I8  msglev = MSGL_ERR;
    UI8 msgstr[64];
    TSDemuxer*   dmx = NULL;
    URLProtocol* pro = NULL;
    UI64 pts_0 = 0ULL;
    UI64 pts_1 = 0ULL;

    if ((ctx == NULL)\
        || ((dmx = (TSDemuxer*)ctx->priv_data) == NULL) || ((pro = dmx->m_Pro) == NULL))
    {
        strcpy(msgstr, "Parameter error");
        goto TSDEMUX_MDATA_RET;
    }

    while ((dmx->m_PMTPID != 0) && (dmx->m_AudioPID != 0) && (dmx->m_VideoPID != 0))
    {
        /// Get a section
        if (TSParse_GetSection (dmx) == FAIL)
        {
            strcpy(msgstr, "Get section failed");
            goto TSDEMUX_MDATA_RET;
        }

        /// Parse PAT section
        if (dmx->m_Section.m_SectionType == 0U && dmx->m_PMTPID == 0U)
        {
            if (TSParse_PATSection (dmx) == FAIL)
            {
                strcpy(msgstr, "Parse PAT section failed");
                goto TSDEMUX_MDATA_RET;
            }
            continue;
        }
        /// Parse PMT section
        else if ((dmx->m_Section.m_SectionType != 0U)\
            && (dmx->m_PMTPID == dmx->m_Section.m_SectionType))
        {
            if (TSParse_PMTSection (dmx, meta) == FAIL)
            {
                strcpy(msgstr, "Parse PMT section failed");
                goto TSDEMUX_MDATA_RET;
            }
            continue;
        }

        /// Save parsed section to pre-read list
        if (TSParse_AddSection (dmx) == FAIL)
        {
            strcpy(msgstr, "Add section failed");
            goto TSDEMUX_MDATA_RET;
        }
    }

    /// Get duration
    if (pro->url_is_live(pro) == 1)
    {
        meta->duation = 0;
    }
    else
    {
        if (pro->url_seek (pro, 0, SEEK_SET) < 0)
        {
            strcpy(msgstr, "Calling url_seek failed");
            goto TSDEMUX_MDATA_RET;
        }

        while (1)
        {
            if (TSParse_GetSection (dmx) == FAIL)
            {
                strcpy(msgstr, "Get section failed");
                goto TSDEMUX_MDATA_RET;
            }
            if (dmx->m_Section.m_SectionType == dmx->m_VideoPID)
            {

            }
        }
    }


    retval = SUCCESS;
    msglev = MSGL_INFO;
    strcpy(msgstr, "Demux metadata OK");
TSDEMUX_MDATA_RET:
    mp_msg(0, msglev, "DEMUX ################ TSDemux_Mdata : %s\n", msgstr);
    return retval;
}
int TSDemux_GetPkt  (DemuxContext* ctx, AVPacket* pack)
{
    I8  retval = FAIL;
    I8  msglev = MSGL_ERR;
    UI8 msgstr[64];

}
int TSDemux_Seek    (DemuxContext* ctx);

#endif // TS_DEMUX_H
