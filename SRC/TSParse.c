#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8* tspkt)
{
    I8 msg[64];
    I8 lev = MSGL_ERR;
    I8 ret = FAIL;
    URLProtocol* pro = dmx->m_Pro;

    do{
        if (NULL == pro)
        {
            strcpy(msg, "Parameter error");
            break;
        }

        ret = pro->url_read(pro, tspkt, 1);
        ++dmx->m_Position;
        if (FAIL == ret)
        {
            strcpy(msg, "Calling url_seek failed");
            break;
        }
        if (TS_PACKET_SYN_BYTE != tspkt[0])                             ///< Sync with 188 bytes
        {
            int i;
            for (i = 0; i < 4; ++i)
            {
                ret = pro->url_read(pro, tspkt, 1);
                if (FAIL == ret)
                {
                    strcpy(msg, "Calling url_seek failed");
                    break;
                }
            }
            dmx->m_Position += 4;
            if (TS_PACKET_SYN_BYTE != tspkt[0])                         ///< Sync with 192 bytes
            {
                for (i = 0; i < 12; ++i)
                {
                    ret = pro->url_read(pro, tspkt, 1);
                    if (FAIL == ret)
                    {
                        strcpy(msg, "Calling url_seek failed");
                        break;
                    }
                }
                dmx->m_Position += 12;
                if (TS_PACKET_SYN_BYTE != tspkt[0])                     ///< Sync with 204 bytes
                {
                    for (i = 0; i < 4; ++i)
                    {
                        ret = pro->url_read(pro, tspkt, 1);
                        if (FAIL == ret)
                        {
                            strcpy(msg, "Calling url_seek failed");
                            break;
                        }
                    }
                    dmx->m_Position += 4;
                    if (TS_PACKET_SYN_BYTE != tspkt[0])                 ///< Sync with 208 bytes
                    {
                        strcpy(msg, "Cannot find sync byte");
                        break;
                    }
                }
            }
        }
        ret = pro->url_read(pro, tspkt + 1, 187);
        if (FAIL == ret)
        {
            strcpy(msg, "Calling url_read failed");
            break;
        }
        dmx->m_Position += 187;

        ret = SUCCESS;
        lev = MSGL_V;
        strcpy(msg, "Get a TS packet OK");
    }while (0);

    mp_msg (0, lev, "DEMUX ################ TSParse_GetAPacket : %s\n", msg);
    return ret;
}

BOOL TSParse_AddAPacket (TSDemuxer* dmx, UI8* tspkt)
{
}

BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    I8  msg[64];
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;

    BOOL finished   = false;
    UI16 cursection = 0U;
    UI8  tempkt[TS_PACKET_SIZE_188];
    TSHeader   head;
    BitBuffer* buf  = NULL;

    while ((dmx->m_PMTPID != 0) && (dmx->m_AudioPID != 0) && (dmx->m_VideoPID != 0))
    {
        UI16 pid = 0U;
        if (FAIL == TSParse_GetAPacket (dmx, tempkt))
        {
            strcpy(msg, "Get a packet failed");
            break;
        }
        if (FAIL == InitiBitBuffer (&buf, tempkt, TS_PACKET_SIZE_188))
        {
            strcpy(msg, "Initialize bit buffer failed");
            break;
        }
        if (FAIL == TSParse_PackHeader (buf, &head))
        {
            strcpy(msg, "Parse packet header failed");
            break;
        }
        if (0U == dmx->m_PMTPID)
        {
            if (0U == head.m_PID)
            {
                TSParse_PATSection ();
            }
            else
            {
                TSParse_AddAPacket ();
            }
            /// get pat packet and parse pmt pid
            /// other packet will be saved in pre-read list, return it
        }
        else if (dmx->m_AudioPID == 0U || dmx->m_VideoPID == 0U)
        {
            if (0U == head.m_PID)
            {
                continue;
            }
            else if (dmx->m_PMTPID == head.m_PID)
            {
                TSParse_PMTSection ();
            }
            else
            {
                TSParse_AddAPacket ();
            }
        }
        else
        {
            if ((0U == head.m_PID) || (dmx->m_PMTPID == head.m_PID)\
             || (dmx->m_AudioPID != head.m_PID && dmx->m_VideoPID != head.m_PID))
            {
                continue;
            }
            else if ()
        }
    }
}

BOOL TSParse_PackHeader (BitBuffer* buf, TSHeader* head)
{
    I8  msg[64];
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;

    UI8 sync_byte;
    UI8 adap_flag;

    do{
        strcpy(msg, "Parse bit buffer failed");
        if (FAIL == GetDataFromBitBuffer (buf, 8, &sync_byte))          ///< Sync byte
        {
            break;
        }
        if (TS_PACKET_SYN_BYTE != sync_byte)
        {
            strcpy(msg, "Sync byte error");
            break;
        }

        if ((FAIL == SkipSeverlBits (buf, 1))                           ///< Transport error flag
         || (FAIL == GetDataFromBitBuffer (buf, 1, &head->m_PLDPresent))///< Payload data start flag
         || (FAIL == SkipSeverlBits (buf, 1))                           ///< Transport priority
         || (FAIL == GetDataFromBitBuffer (buf,13, &head->m_PID))       ///< PID
         || (FAIL == SkipSeverlBits (buf, 2))                           ///< Scrambling flag
         || (FAIL == GetDataFromBitBuffer (buf, 1, &adap_flag))         ///< Adaptation field flag
         || (FAIL == GetDataFromBitBuffer (buf, 1, &head->m_PLDPresent))///< Valid payload flag
         || (FAIL == SkipSeverlBits (buf, 4)))                          ///< Continuity counter
        {
            break;
        }

        if (1 == adap_flag)
        {
            UI8 adap_lens;
            if ((FAIL == GetDataFromBitBuffer (buf, 8, &adap_lens))     ///< Adaptation field length
             || (FAIL == SkipSeverlBits (buf, adap_lens * 8)))          ///< Skip adaptation field
            {
                break;
            }
        }

        ret = SUCCESS;
        lev = MSGL_V;
        strcpy(msg, "Parse packet header OK");
    }while(0);

    mp_msg(0, lev, "DEMUX ################ TSParse_PackHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_PATSection (BitBuffer* buf, TSDemuxer* dmx)
{
}
BOOL TSParse_PMTSection (BitBuffer* buf, TSDemuxer* dmx)
{
    I8   msg[64];
    I8   ret = FAIL;
    I8   lev = MSGL_ERR;

    UI8  pointer        = 0U;   ///< PSI pointer field fixed with 0
    UI8  table_id       = 0U;   ///< table ID fixed with 2
    UI8  sect_len       = 0U;   ///< section length, the length of data behind this field
    UI16 info_len       = 0U;   ///< program info length, the length of the discriptor
    UI8  stream_type    = 0U;   ///< stream type
    UI16 elementary_pid = 0U;   ///< elementary PID
    UI8* pes_data       = NULL; ///< PES data
    BitBuffer* newbuf   = NULL; ///< Bit buffer for parse PES data

    do{
        strcpy(msg, "Parse bit buffer failed");
        if ((FAIL == GetDataFromBitBuffer (buf, 8, &pointer))           ///< pointer
         || (FAIL == GetDataFromBitBuffer (buf, 8, &table_id)))         ///< table ID
        {
            break;
        }
        if (pointer != 0x00 || table_id != 0x02)
        {
            strcpy(msg, "Point and table_id error");
            break;
        }

        if ((FAIL == SkipSeverlBits (buf, 4))
         || (FAIL == GetDataFromBitBuffer (buf, 12, &sect_len)))        ///< section length
        {
            break;
        }

        pes_data = (UI8*)malloc(sect_len);
        if (NULL == pes_data)
        {
            strcpy(msg, "Allocate PES data space failed");
            break;
        }
        if (sect_len > (buf->m_Length - buf->m_BytePos))
        {
            /// @todo
        }
        else
        {
            memcpy (pes_data, (buf->m_Data + buf->m_Length), sect_len);
        }

        if (InitiBitBuffer())

    }while (0);
}

