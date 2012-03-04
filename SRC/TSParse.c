#include <malloc.h>
#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

BOOL TSParse_InitParser (TSDemuxer* dmx)
{
    I8  msg[64];
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;

    /// Packet data for synchronizing and initialize parsing start position\n
    /// Read one more byte than four longest TS packets data. Use 207 byte to find sync byte(0x47)\n
    /// at most, and the left data to check if found the start position of the 1st valid TS packet
    UI8          probebuf[(TS_PACKET_SIZE_MAX << 2) + 1];
    /// Probe buffer data length
    int          probebuflen = (TS_PACKET_SIZE_MAX << 2) + 1;
    /// TS packet length that is 188, 192, 204 or 208
    int          tspkt_len   = 0;
    /// The 1st valid sync byte position in current stream
    int          start_pos   = 0;
    /// Demuxer data IO protocol
    URLProtocol* pro         = dmx->m_Pro;
    UI8*         temppkt     = NULL;

    /// step-1 : Get probe buffer data
    int flag = pro->url_read(pro, probebuf, probebuflen);
    if (flag != probebuflen)
    {
        if (flag == 0)
        {
            strcpy(msg, "Stream end unexceptedly");
        }
        strcpy(msg, "Calling url_read failed");
        goto TSPARSE_INITPARSER_RET;
    }
    dmx->m_Position += probebuflen;

    /// step-2 : Get the first valid sync byte position
    while (1)
    {
        while (start_pos < TS_PACKET_SIZE_MAX - 1)
        {
            if (probebuf[start_pos] == 0x47)
            {
                break;
            }
            ++start_pos;
        }
        if (start_pos == TS_PACKET_SIZE_MAX - 1)
        {
            strcpy(msg, "Cannot find valid sync byte");
            goto TSPARSE_INITPARSER_RET;
        }

        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_188;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_192;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_204;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_208;
            break;
        }
        ++start_pos;
    }

    /// step-3 : Save the all probe buffer data to pre-read packet list
    probebuflen -= start_pos;
    while (probebuflen >= TS_PACKET_SIZE_188)
    {
        temppkt = (UI8*)malloc(TS_PACKET_SIZE_188);
        if (temppkt == NULL)
        {
            strcpy(msg, "Allocate a pre-read packet failed");
            goto TSPARSE_INITPARSER_RET;
        }
        memcpy(temppkt, probebuf + start_pos, TS_PACKET_SIZE_188);
        if (FAIL == TSParse_AddPrePack(dmx, temppkt, start_pos))
        {
            free (temppkt);
            strcpy(msg, "Add a pre-read packet failed");
            goto TSPARSE_INITPARSER_RET;
        }
        start_pos   += tspkt_len;
        probebuflen -= tspkt_len;
    }
    temppkt = (UI8*)malloc(TS_PACKET_SIZE_188);
    if (temppkt == NULL)
    {
        strcpy(msg, "Allocate a pre-read packet failed");
        goto TSPARSE_INITPARSER_RET;
    }
    memcpy(temppkt, probebuf + start_pos, probebuflen);
    if (flag = pro->url_read(pro, temppkt + probebuflen, TS_PACKET_SIZE_188 - probebuflen))
    {
        if (flag == 0)
        {
            strcpy(msg, "Stream end unexceptedly");
        }
        strcpy(msg, "Calling url_read failed");
        goto TSPARSE_INITPARSER_RET;
    }
    dmx->m_Position += (TS_PACKET_SIZE_188 - probebuflen);
    if (FAIL == TSParse_AddPrePack(dmx, temppkt, start_pos))
    {
        free (temppkt);
        strcpy(msg, "Add a pre-read packet failed");
        goto TSPARSE_INITPARSER_RET;
    }

    /// step-4 : Drop the all invalid TS packets which are at front of the 1st valid TS packet
    while (1)
    {
        PrePacket* node = dmx->m_PktList.m_Next;
        TSHeader   head;

        while (node != NULL)
        {
            if (FAIL == TSParse_PackHeader(&head, node->m_Pack->m_Data))
            {
                strcpy(msg, "Parse TS packet header failed");
                goto TSPARSE_INITPARSER_RET;
            }
            if (head.m_PESPresent == 0)
            {
                TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                node = dmx->m_PktList.m_Next;
            }
            else
            {
                break;
            }
        }
        if (node != NULL)
        {
            break;
        }
        flag    = FAIL;
        temppkt = (UI8*)malloc(TS_PACKET_SIZE_188);
        if (temppkt == NULL)
        {
            strcpy(msg, "Allocate a pre-read packet failed");
            goto TSPARSE_INITPARSER_RET;
        }
        while (1)
        {
            if (FAIL == TSParse_GetAPacket(dmx, temppkt))
            {
                strcpy(msg, "Get a TS packet failed");
                goto TSPARSE_INITPARSER_RET;
            }
            if (FAIL == TSParse_PackHeader(&head, temppkt))
            {
                free(temppkt);
                strcpy(msg, "Parse TS packet header failed");
                goto TSPARSE_INITPARSER_RET;
            }
            if (head.m_PESPresent != 0)
            {
                if (FAIL == TSParse_AddPrePack(dmx, temppkt, dmx->m_Position - TS_PACKET_SIZE_188))
                {
                    free (temppkt);
                    strcpy(msg, "Add a pre-read packet failed");
                    goto TSPARSE_INITPARSER_RET;
                }
                flag = SUCCESS;
                break;
            }
        }
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Successfully initialized parser");

TSPARSE_INITPARSER_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_InitParser : %s", msg);
    return ret;
}
BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8* data)
{
    I8 msg[64];
    I8 lev = MSGL_ERR;
    I8 ret = FAIL;

    URLProtocol* pro = dmx->m_Pro;
    int i = 0;

    strcpy(msg, "Calling url_seek failed");
    ret = pro->url_read(pro, data, 1);
    if (1 != ret)
    {
        if (ret == 0)
        {
            strcpy(msg, "File end");
        }
        goto TSPARSE_GETAPACKET_RET;
    }
    ++dmx->m_Position;
    /// Sync with 188 bytes
    if (TS_PACKET_SYN_BYTE != data[0])
    {
        for (i = 0; i < 4; ++i)
        {
            if (1 != pro->url_read(pro, data, 1))
            {
                goto TSPARSE_GETAPACKET_RET;
            }
        }
        dmx->m_Position += 4;
        /// Sync with 192 bytes
        if (TS_PACKET_SYN_BYTE != data[0])
        {
            for (i = 0; i < 12; ++i)
            {
                if (1 != pro->url_read(pro, data, 1))
                {
                    goto TSPARSE_GETAPACKET_RET;
                }
            }
            dmx->m_Position += 12;
            /// Sync with 204 bytes
            if (TS_PACKET_SYN_BYTE != data[0])
            {
                for (i = 0; i < 4; ++i)
                {
                    if (1 != (pro->url_read(pro, data, 1)))
                    {
                        goto TSPARSE_GETAPACKET_RET;
                    }
                }
                dmx->m_Position += 4;
                /// Sync with 208 bytes
                if (TS_PACKET_SYN_BYTE != data[0])
                {
                    strcpy(msg, "Cannot find sync byte");
                    goto TSPARSE_GETAPACKET_RET;
                }
            }
        }
    }
    ret = pro->url_read(pro, data + 1, TS_PACKET_SIZE_188 - 1);
    if (FAIL == ret) break;
    dmx->m_Position += (TS_PACKET_SIZE_188 - 1);

TSPARSE_GETAPACKET_RET:
    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Get a TS packet OK");

TSPARSE_GETAPACKET_RET:
    mp_msg (0, lev, "DEMUX ################ TSParse_GetAPacket : %s\n", msg);
    return ret;
}
BOOL TSParse_AddPrePack (TSDemuxer* dmx, UI8* data, UI64 pos)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    PrePacket* temp = &dmx->m_PktList;
    PrePacket* node;

    node = (PrePacket*)malloc(sizeof(PrePacket));
    if (node == NULL)
    {
        strcpy(msg, "Allocate new PrePacket node failed");
        goto TSPARSE_ADDPREPACK_RET;
    }
    node->m_Pack    = (TSPacket *)malloc(sizeof(TSPacket));
    node->m_Next    = NULL;
    if (node->m_Pack == NULL)
    {
        free (node);
        strcpy(msg, "Allocate new PrePacket node failed");
        goto TSPARSE_ADDPREPACK_RET;
    }
    node->m_Pack->m_Data     = data;
    node->m_Pack->m_Position = pos;
    while (temp->m_Next != NULL)
    {
        temp = temp->m_Next;
    }
    temp->m_Next = node;

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Add a pre-read packet OK");

TSPARSE_ADDPREPACK_RET:
    mp_msg (0, lev, "DEMUX ################ TSParse_AddPrePack : %s\n", msg);
    return ret;
}
BOOL TSParse_DelPrePack (TSDemuxer* dmx, UI8* data)
{
    PrePacket* prev = &dmx->m_PktList;
    PrePacket* node = dmx->m_PktList.m_Next;

    if (data == NULL)
    {
        return FAIL;
    }

    while (node != NULL)
    {
        if(node->m_Pack->m_Data == data)
        {
            free (data);
            data = node->m_Pack->m_Data = NULL;
            free (node->m_Pack);
            node->m_Pack = NULL;
            prev->m_Next = node->m_Next;
            free (node);
            node = NULL;
            break;
        }
        node = node->m_Next;
        prev = prev->m_Next;
    }
}
BOOL TSParse_PackHeader (TSHeader* head, UI8* data)
{
    I8  msg[64];
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;

    UI8 sync_byte;
    UI8 adap_flag;
    BitBuffer* buf = NULL;

    if (FAIL == InitiBitBuffer (&buf, data, TS_PACKET_SIZE_188))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_GETAPACKET_RET;
    }

    strcpy(msg, "Parse bit buffer failed");
    if (FAIL == GetDataFromBitBuffer (buf, 8, &sync_byte))          ///< Sync byte
    {
        goto TSPARSE_GETAPACKET_RET;
    }
    if (TS_PACKET_SYN_BYTE != sync_byte)
    {
        strcpy(msg, "Sync byte error");
        goto TSPARSE_GETAPACKET_RET;
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
        goto TSPARSE_GETAPACKET_RET;
    }

    if (1 == adap_flag)
    {
        UI8 adap_lens;
        UI8 pcr_flag;
        if ((FAIL == GetDataFromBitBuffer (buf, 8, &adap_lens)))    ///< Adaptation field length
        {
            goto TSPARSE_GETAPACKET_RET;
        }
        if (adap_lens > 0)
        {
            if ((FAIL == SkipSeverlBits (buf, 3))
             || (FAIL == GetDataFromBitBuffer (buf, 1, &pcr_flag))
             || (FAIL == SkipSeverlBits (buf, 4)))
            {
                goto TSPARSE_GETAPACKET_RET;
            }
            if (pcr_flag != 0)
            {
                if ((FAIL == GetDataFromBitBuffer (buf, 33, &head->m_PCRBase))
                 || (FAIL == SkipSeverlBits (buf,  6))
                 || (FAIL == GetDataFromBitBuffer (buf,  9, &head->m_PCRExten))
                 || (FAIL == SkipSeverlBits ((adap_lens - 1) * 8)))
                {
                    goto TSPARSE_GETAPACKET_RET;
                }
            }
            else
            {
                if ((FAIL == SkipSeverlBits ((adap_lens - 7) * 8)))
                {
                    goto TSPARSE_GETAPACKET_RET;
                }
            }
        }
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse packet header OK");

TSPARSE_PACKHEADER_RET:
    CloseBitBuffer(&buf);
    mp_msg(0, lev, "DEMUX ################ TSParse_PackHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_PATSection (TSDemuxer* dmx)
{
    I8   msg[64];
    I8   ret = FAIL;
    I8   lev = MSGL_ERR;

    UI8* pkt = NULL;
    TSHeader head;

    while (1)
    {
        pkt = (UI8*)malloc(TS_PACKET_SIZE_188);
        TSParse_GetAPacket (dmx , pkt);
        TSParse_PackHeader (head, pkt);
        if (head.m_PID == 0)
        {
            break;
        }
        TSParse_AddPrePack (dmx, pkt, dmx->m_Position - TS_PACKET_SIZE_188);
    }



}
BOOL TSParse_PMTSection (TSDemuxer* dmx)
{}
BOOL TSParse_PESSection (TSDemuxer* dmx)
{}
