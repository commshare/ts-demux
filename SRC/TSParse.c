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
    flag = pro->url_read(pro, temppkt + probebuflen, TS_PACKET_SIZE_188 - probebuflen);
    if (flag == 0)
    {
        strcpy(msg, "Stream end unexceptedly");
    }
    else
    {
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
            if (FAIL == TSParse_TSPacketHeader(node->m_Pack->m_Data, &head))
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
            if (FAIL == TSParse_TSPacketHeader(temppkt, &head))
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
        int i;
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
    if (FAIL == ret)
    {
        goto TSPARSE_GETAPACKET_RET;
    }
    dmx->m_Position += (TS_PACKET_SIZE_188 - 1);

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
    return SUCCESS;
}
BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    I8   msg[64];
    I8   ret = FAIL;
    I8   lev = MSGL_ERR;

    UI8* pack = NULL;
    TSHeader head;
    UI16 section_len = 0U;
    UI8  stream_id   = 0U;
    UI8  parse_lev   = PARSE_LEV_PAT;
    BOOL first_tspkt = TRUE;
    UI64 copied_len  = 0ULL;


    /// Appoint current parsing level
    if (dmx->m_PMTPID == 0x00)
    {
        parse_lev = PARSE_LEV_PAT;
    }
    else if (dmx->m_AudioPID == 0x00 || dmx->m_VideoPID == 0x00)
    {
        parse_lev = PARSE_LEV_PMT;
    }
    else
    {
        parse_lev = PARSE_LEV_PES;
    }

    pack = (UI8*)malloc(TS_PACKET_SIZE_188);
    if (pack == NULL)
    {
        strcpy(msg, "Allocat a TS packet failed");
        goto TSPARSE_GETSECTION_RET;
    }

    do{
        PrePacket* prev  = &dmx->m_PktList;
        PrePacket* node  = dmx->m_PktList.m_Next;

        /// Get the packet we are going to parse
        if (node != NULL)
        {
            while (node != NULL)
            {
                if (FAIL == TSParse_TSPacketHeader (node->m_Pack->m_Data, &head))
                {
                    strcpy(msg, "Parse TS packet header failed");
                    goto TSPARSE_GETSECTION_RET;
                }
                if (parse_lev == PARSE_LEV_PAT)
                {
                    if (head.m_PID == 0x00)
                    {
                        memcpy(pack, node->m_Pack->m_Data, TS_PACKET_SIZE_188);
                        TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                        break;
                    }
                    else
                    {
                        node = node->m_Next;
                        prev = prev->m_Next;
                    }
                }
                else if (parse_lev == PARSE_LEV_PMT)
                {
                    if (head.m_PID == 0x00)
                    {
                        TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                        node = prev->m_Next;
                    }
                    else if (head.m_PID == dmx->m_PMTPID)
                    {
                        memcpy(pack, node->m_Pack->m_Data, TS_PACKET_SIZE_188);
                        TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                        break;
                    }
                    else
                    {
                        node = node->m_Next;
                        prev = prev->m_Next;
                    }
                }
                else
                {
                    if ((head.m_PID == 0x00) || (head.m_PID == dmx->m_PMTPID)\
                            || (head.m_PID != dmx->m_AudioPID && head.m_PID != dmx->m_VideoPID))
                    {
                        TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                        node = prev->m_Next;
                    }
                    else
                    {
                        memcpy(pack, node->m_Pack->m_Data, TS_PACKET_SIZE_188);
                        TSParse_DelPrePack(dmx, node->m_Pack->m_Data);
                        break;
                    }
                }
            }
            if (first_tspkt == TRUE && head.m_PESPresent != 0x01)
            {
                strcpy(msg, "Error payload start");
                goto TSPARSE_GETSECTION_RET;
            }
        }
        else if (node == NULL)
        {
            while (1)
            {
                if (pack == NULL)
                {
                    if (NULL == (pack = (UI8*)malloc(TS_PACKET_SIZE_188)))
                    {
                        strcpy(msg, "Allocate a TS packet failed");
                    }
                }
                if (FAIL == TSParse_GetAPacket (dmx, pack))
                {
                    strcpy(msg, "Get a TS packet failed");
                    goto TSPARSE_GETSECTION_RET;
                }
                if (FAIL == TSParse_TSPacketHeader (pack, &head))
                {
                    strcpy(msg, "Parse TS packet failed");
                    goto TSPARSE_GETSECTION_RET;
                }
                if (parse_lev == PARSE_LEV_PAT)
                {
                    if (head.m_PID == 0x00)
                    {
                        break;
                    }
                    else
                    {
                        TSParse_AddPrePack (dmx, pack, dmx->m_Position - TS_PACKET_SIZE_188);
                        pack = NULL;
                    }
                }
                else if (parse_lev == PARSE_LEV_PMT)
                {
                    if (head.m_PID == 0x00)
                    {
                        continue;
                    }
                    else if (head.m_PID == dmx->m_PMTPID)
                    {
                        break;
                    }
                    else
                    {
                        TSParse_AddPrePack (dmx, pack, dmx->m_Position - TS_PACKET_SIZE_188);
                        pack = NULL;
                    }
                }
                else
                {
                    if ((head.m_PID == 0x00) || (head.m_PID == dmx->m_PMTPID)\
                            || (head.m_PID != dmx->m_AudioPID && head.m_PID != dmx->m_VideoPID))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if (first_tspkt == TRUE && head.m_PESPresent != 0x01)
            {
                strcpy(msg, "Error payload start");
                goto TSPARSE_GETSECTION_RET;
            }
        }

        /// Parse payload packet header if the payload_unit_start_indicator is present
        if (head.m_PESPresent == 0x01)
        {
            UI16 sub_packet_header_len;
            copied_len = 0ULL;
            if (parse_lev == PARSE_LEV_PAT || parse_lev == PARSE_LEV_PMT)
            {
                if (FAIL == TSParse_ParsePSIHeader (pack, &section_len))
                {
                    strcpy(msg, "Parse PSI packet header failed");
                    goto TSPARSE_GETSECTION_RET;
                }
                sub_packet_header_len = PSI_PACKET_HEADER_LEN;
            }
            else
            {
                if (FAIL == TSParse_ParsePESHeader (pack, &section_len, &stream_id))
                {
                    strcpy(msg, "Parse PES packet header failed");
                    goto TSPARSE_GETSECTION_RET;
                }
                sub_packet_header_len = PES_PACKET_HEADER_LEN;
            }
            if (section_len != 0)
            {
                dmx->m_Section.m_Type = head.m_PID;
                if (dmx->m_Section.m_BuffLen < section_len)
                {
                    if (dmx->m_Section.m_Data != NULL)
                    {
                        free (dmx->m_Section.m_Data);
                        dmx->m_Section.m_Data = NULL;
                    }
                    dmx->m_Section.m_Data = (UI8*)malloc(section_len);
                    if (dmx->m_Section.m_Data == NULL)
                    {
                        strcpy(msg, "Allocate section space failed");
                        goto TSPARSE_GETSECTION_RET;
                    }
                    dmx->m_Section.m_BuffLen = section_len;
                }
                dmx->m_Section.m_DataLen = section_len;
                dmx->m_Section.m_StreamID        = stream_id;
            }
            else
            {
                /// @todo PES packet length is indicated with 0, how to deal it?
                strcpy(msg, "Section length is 0");
                goto TSPARSE_GETSECTION_RET;
            }
            if (head.m_HeaderLen + sub_packet_header_len < TS_PACKET_SIZE_188)
            {
                copied_len = (TS_PACKET_SIZE_188 - head.m_HeaderLen - sub_packet_header_len);
                memcpy(dmx->m_Section.m_Data\
                    , pack + head.m_HeaderLen + sub_packet_header_len, copied_len);
            }
        }
        else
        {
            memcpy(dmx->m_Section.m_Data + copied_len, pack + head.m_HeaderLen\
                , TS_PACKET_SIZE_188 - head.m_HeaderLen);
            copied_len += (TS_PACKET_SIZE_188 - head.m_HeaderLen);
        }

        if (copied_len == dmx->m_Section.m_DataLen);
        {
            break;
        }
    }while (1);

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Get a section OK");

TSPARSE_GETSECTION_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_GetSection : %s\n", msg);
    return ret;
}
BOOL TSParse_PATSection (TSDemuxer* dmx)
{
    I8  ret = FAIL;
    I8  lev = MSGL_V;
    I8  msg[64];

    int  rows = 0;
    UI16 program_number;
    BitBuffer* buf = NULL;

    UI8* data = (UI8*)dmx->m_Section.m_Data;
    UI32 lens = (UI32)dmx->m_Section.m_DataLen;

    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_PATSECTION_RET;
    }
    if (FAIL == SkipSeverlBits (buf, 40))
    {
        strcpy(msg, "Parse bit buffer failed");
        goto TSPARSE_PATSECTION_RET;
    }

    rows = (lens - buf->m_BytePos - 4) / 4;
    while (rows > 0)
    {
        if ((FAIL == GetDataFromBitBuffer (buf, 16, &program_number))
         || (FAIL == SkipSeverlBits (buf,  3)))
        {
            strcpy(msg, "Parse bit buffer failed");
            goto TSPARSE_PATSECTION_RET;
        }
        if (program_number == 0)
        {
            if (FAIL == GetDataFromBitBuffer (buf, 13, &dmx->m_PMTPID))
            {
                strcpy(msg, "Parse bit buffer failed");
                goto TSPARSE_PATSECTION_RET;
            }
            break;
        }
        else
        {
            if (FAIL == SkipSeverlBits (buf, 13))
            {
                strcpy(msg, "Parse bit buffer failed");
                goto TSPARSE_PATSECTION_RET;
            }
        }
        --rows;
    }
    if (dmx->m_PMTPID == 0U)
    {
        strcpy(msg, "Cannot find PMT PID");
        goto TSPARSE_PATSECTION_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse PAT section OK");

TSPARSE_PATSECTION_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_PATSection : %s\n", msg);
    return ret;
}
BOOL TSParse_PMTSection (TSDemuxer* dmx, Metadata* meta)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    UI8  stream_type;
    UI16 av_pid;
    UI16 program_info_len;
    UI16 es_info_len;
    BitBuffer* buf = NULL;

    UI8* data = (UI8*)dmx->m_Section.m_Data;
    UI32 lens = (UI32)dmx->m_Section.m_DataLen;

    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_PMTSECTION_RET;
    }
    if ((FAIL == SkipSeverlBits (buf, 60))
     || (FAIL == GetDataFromBitBuffer (buf, 12, &program_info_len)))
    {
        strcpy(msg, "Parse bit buffer failed");
        goto TSPARSE_PMTSECTION_RET;
    }
    if ((FAIL == SkipSeverlBits (buf, 8 * program_info_len)))
    {
        strcpy(msg, "Parse bit buffer failed");
        goto TSPARSE_PMTSECTION_RET;
    }
    while ((lens - buf->m_BytePos - 4) > 0)
    {
        if ((FAIL == GetDataFromBitBuffer (buf,  8, &stream_type))
         || (FAIL == SkipSeverlBits (buf, 3))
         || (FAIL == GetDataFromBitBuffer (buf, 13, &av_pid))
         || (FAIL == SkipSeverlBits (buf, 4))
         || (FAIL == GetDataFromBitBuffer (buf, 12, &es_info_len))
         || (FAIL == SkipSeverlBits (buf, 8 * es_info_len)))
        {
            strcpy(msg, "Parse bit buffer failed");
            goto TSPARSE_PMTSECTION_RET;
        }
        /// @todo parse stream type and set AV PID
        switch (stream_type)
        {
        // case 0x01 : ///< MPEG Video; //Version 1
        // case 0x02 : ///< MPEG Video; //Version 2
        // case 0x03 : ///< MPEG Audio; //Version 1
        // case 0x04 : ///< MPEG Audio; //Version 2
        case 0x0F : ///< AAC;
            if (dmx->m_AudioPID != 0x00)
            {
                break;
            }
            meta->audiocodec       = CODEC_ID_AAC;
            meta->audiostreamindex = dmx->m_AudioPID = av_pid;
            break;
        // case 0x10 : ///< MPEG-4 Visual;
        case 0x11 : ///< AAC;
            if (dmx->m_AudioPID != 0x00)
            {
                break;
            }
            meta->audiocodec       = CODEC_ID_AAC;
            meta->audiostreamindex = dmx->m_AudioPID = av_pid;
            break;
        case 0x1B : ///< AVC;
            meta->videocodec     = CODEC_ID_H264;
            break;
        case 0x1C : ///< AAC;
            if (dmx->m_AudioPID != 0x00)
            {
                break;
            }
            meta->audiocodec       = CODEC_ID_AAC;
            meta->audiostreamindex = dmx->m_AudioPID = av_pid;
            break;
        // case 0x1D : ///< Timed Text;
            break;
        // case 0x1E : ///< MPEG Video; //ISO/IEC 23002-3
        case 0x1F : ///< AVC;
            meta->videocodec     = CODEC_ID_H264;
            break;
        case 0x20 : ///< AVC;
            meta->videocodec     = CODEC_ID_H264;
            break;
        }
    }
    if (dmx->m_AudioPID == 0x00 || dmx->m_VideoPID == 0x00)
    {
        strcpy(msg, "Cannot find audio or video PID");
        goto TSPARSE_PMTSECTION_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse PMT section OK");

TSPARSE_PMTSECTION_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_PMTSection : %s\n", msg);
    return ret;
}
BOOL TSParse_PESSection (TSDemuxer* dmx, AVPacket* pack)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    BitBuffer* buf = NULL;
    UI8        sid = dmx->m_Section.m_StreamID;
    UI8        pts;
    UI8        dts;
    UI8        pes_header_len;

    UI8* data = (UI8*)dmx->m_Section.m_Data;
    UI32 lens = (UI32)dmx->m_Section.m_DataLen;



    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_PESSECTION_RET;
    }
    if ((sid != STREAM_ID_PROGRAM_MAP)
     || (sid != STREAM_ID_PADDING)
     || (sid != STREAM_ID_PRIVATE_2)
     || (sid != STREAM_ID_ECM)
     || (sid != STREAM_ID_EMM)
     || (sid != STREAM_ID_PRO_DIREC))
    {
        UI64 av_data_len;
        if ((FAIL == SkipSeverlBits (buf, 8))
         || (FAIL == GetDataFromBitBuffer (buf, 1, &pts))
         || (FAIL == GetDataFromBitBuffer (buf, 1, &dts))
         || (FAIL == SkipSeverlBits (buf, 6))
         || (FAIL == GetDataFromBitBuffer (buf, 8, &pes_header_len)))
        {
            strcpy(msg, "Parse bit buffer failed");
            goto TSPARSE_PESSECTION_RET;
        }
        pack->pts = pack->dts = -1LL;
        if (pts != 0x01)
        {
            UI8  pts_1;
            UI16 pts_2, pts_3;
            I64  pts = 0LL;
            if ((FAIL == SkipSeverlBits (buf, 4))
             || (FAIL == GetDataFromBitBuffer (buf,  3, &pts_1))
             || (FAIL == SkipSeverlBits (buf, 1))
             || (FAIL == GetDataFromBitBuffer (buf, 15, &pts_2))
             || (FAIL == SkipSeverlBits (buf, 1))
             || (FAIL == GetDataFromBitBuffer (buf, 15, &pts_3))
             || (FAIL == SkipSeverlBits (buf, 1)))
            {
                strcpy(msg, "Parse bit buffer failed");
                goto TSPARSE_PESSECTION_RET;
            }
            pts = (pts_1 << 30) | (pts_2 << 15) | (pts_3);
            pack->pts = pts;
        }
        if (dts != 0x01)
        {
            UI8  dts_1;
            UI16 dts_2, dts_3;
            I64  dts = 0LL;
            if ((FAIL == SkipSeverlBits (buf, 4))
             || (FAIL == GetDataFromBitBuffer (buf,  3, &dts_1))
             || (FAIL == SkipSeverlBits (buf, 1))
             || (FAIL == GetDataFromBitBuffer (buf, 15, &dts_2))
             || (FAIL == SkipSeverlBits (buf, 1))
             || (FAIL == GetDataFromBitBuffer (buf, 15, &dts_3))
             || (FAIL == SkipSeverlBits (buf, 1)))
            {
                strcpy(msg, "Parse bit buffer failed");
                goto TSPARSE_PESSECTION_RET;
            }
            pts = (dts_1 << 30) | (dts_2 << 15) | (dts_3);
            pack->dts = dts;
        }
        if (FAIL == SkipSeverlBits (buf, pes_header_len * 8 - (pts + dts) * 40))
        {
            strcpy(msg, "Parse bit buffer failed");
            goto TSPARSE_PESSECTION_RET;
        }
        av_data_len = dmx->m_Section.m_DataLen - buf->m_BytePos;
        if (pack->bufferlength < av_data_len)
        {
            if (pack->data != NULL)
            {
                free (pack->data);
                pack->data = NULL;
            }
            pack->data = (UI8*)malloc(av_data_len);
            if (pack->data == NULL)
            {
                strcpy(msg, "Allocate AV packet data failed");
                goto TSPARSE_PESSECTION_RET;
            }
            pack->bufferlength = av_data_len;
        }
        memcpy(pack->data, dmx->m_Section.m_Data + buf->m_BytePos, av_data_len);
        pack->size = av_data_len;
        pack->stream_index = dmx->m_Section.m_Type;
    }
    else
    {
        strcpy(msg, "Wrong stream id");
        goto TSPARSE_PESSECTION_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse PES section OK");

TSPARSE_PESSECTION_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_PESSection : %s\n", msg);
    return ret;
}
BOOL TSParse_TSPacketHeader (UI8* data, TSHeader* head)
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
        goto TSPARSE_TSPACKETHEADER_RET;
    }

    strcpy(msg, "Parse bit buffer failed");
    if (FAIL == GetDataFromBitBuffer (buf, 8, &sync_byte))          ///< Sync byte
    {
        goto TSPARSE_TSPACKETHEADER_RET;
    }
    if (TS_PACKET_SYN_BYTE != sync_byte)
    {
        strcpy(msg, "Sync byte error");
        goto TSPARSE_TSPACKETHEADER_RET;
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
        goto TSPARSE_TSPACKETHEADER_RET;
    }

    if (1 == adap_flag)
    {
        UI8 adap_lens;
        UI8 pcr_flag;
        if ((FAIL == GetDataFromBitBuffer (buf, 8, &adap_lens)))    ///< Adaptation field length
        {
            goto TSPARSE_TSPACKETHEADER_RET;
        }
        if (adap_lens > 0)
        {
            if ((FAIL == SkipSeverlBits (buf, 3))
             || (FAIL == GetDataFromBitBuffer (buf, 1, &pcr_flag))
             || (FAIL == SkipSeverlBits (buf, 4)))
            {
                goto TSPARSE_TSPACKETHEADER_RET;
            }
            if (pcr_flag != 0)
            {
                if ((FAIL == GetDataFromBitBuffer (buf, 33, &head->m_PCRBase))
                 || (FAIL == SkipSeverlBits (buf,  6))
                 || (FAIL == GetDataFromBitBuffer (buf,  9, &head->m_PCRExten))
                 || (FAIL == SkipSeverlBits (buf, (adap_lens - 1) * 8)))
                {
                    goto TSPARSE_TSPACKETHEADER_RET;
                }
            }
            else
            {
                if ((FAIL == SkipSeverlBits (buf, (adap_lens - 7) * 8)))
                {
                    goto TSPARSE_TSPACKETHEADER_RET;
                }
            }
        }
    }

    head->m_HeaderLen = buf->m_BytePos;
    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse packet header OK");

TSPARSE_TSPACKETHEADER_RET:
    CloseBitBuffer(&buf);
    mp_msg(0, lev, "DEMUX ################ TSParse_TSPacketHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_ParsePSIHeader (UI8* data, UI16* section_len)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    TSHeader  head;
    BitBuffer* buf = NULL;

    TSParse_TSPacketHeader (data, &head);

    if (InitiBitBuffer (&buf, data + head.m_HeaderLen, TS_PACKET_SIZE_188 - head.m_HeaderLen))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_PARSEPSIHEADER_RET;
    }
    if ((FAIL == SkipSeverlBits (buf, 20))
     || (FAIL == GetDataFromBitBuffer (buf, 12, section_len)))
    {
        strcpy(msg, "Parse bit buffer failed");
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse PSI packet header OK");

TSPARSE_PARSEPSIHEADER_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_ParsePSIHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_ParsePESHeader (UI8* data, UI16* section_len, UI8* stream_id)
{
    I8  ret = FAIL;
    I8  lev = MSGL_ERR;
    I8  msg[64];

    UI32 prefix;
    TSHeader  head;
    BitBuffer* buf = NULL;

    TSParse_TSPacketHeader (data, &head);

    if (InitiBitBuffer (&buf, data + head.m_HeaderLen, TS_PACKET_SIZE_188 - head.m_HeaderLen))
    {
        strcpy(msg, "Initialize bit buffer failed");
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    if ((FAIL == GetDataFromBitBuffer (buf, 24, &prefix))
     || (FAIL == GetDataFromBitBuffer (buf,  8, stream_id))
     || (FAIL == GetDataFromBitBuffer (buf, 16, section_len)))
    {
        strcpy(msg, "Parse bit buffer failed");
        goto TSPARSE_PARSEPSIHEADER_RET;
    }
    if (prefix != 0x000001)
    {
        strcpy(msg, "Wrong Prefix field");
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    strcpy(msg, "Parse PES packet header OK");

TSPARSE_PARSEPSIHEADER_RET:
    mp_msg(0, lev, "DEMUX ################ TSParse_ParsePESHeader : %s\n");
    return ret;
}
