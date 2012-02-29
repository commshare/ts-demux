#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8* tspkt)
{
}

BOOL TSParse_AddAPacket (TSDemuxer* dmx, UI8* tspkt)
{
}

BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    if (dmx->m_PMTPID == 0U)
    {
        /// get pat packet and parse pmt pid
        /// other packet will be saved in pre-read list, return it
    }
    else if (dmx->m_AudioPID == 0U || dmx->m_VideoPID == 0U)
    {
        /// read pakcet from pre-read list
        /// drop pat and get pmt packet and parse audio and video pid, return it
        /// other packet will be saved in pre-read list
    }
    else
    {
        /// read packet from pre-read list
        /// drop pat, pmt and other type packet and then get a audio or video section, return it
    }
}


BOOL TSParse_PackHeader (TSDemuxer* dmx, TSHeader* head)
{
    I8   retval = FAIL;
    I8   msglev = MSGL_ERR;
    I8   msgstr[64];
    UI8  syncbyte;
    BitBuffer* buf = NULL;


    strcpy(msgstr, "Parameters error");
    if (dmx == NULL)
    {}

    buf = &(dmx->m_Section.m_BitBuf);

    if ()
}

BOOL TSParse_PATSection (TSDemuxer* dmx);

