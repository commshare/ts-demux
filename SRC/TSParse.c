#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

/// @brief  Find sync byte or resynchronize
/// @param  dmx TS demuxer
/// @note This method will modify current read position(in demuxer),  but just move to the nearest\n
/// position where sync byte is. And all packets have read will be store in pre-read list and used\n
/// by metadata parsing routine or packet reading routine.
BOOL TSParse_StartSync (TSDemuxer* dmx)
{
    I8  errtyp;
    I8  errstr[64];
    UI8 prob_buf[SYN_PROBE_MAX_SIZE];
    int sync_pos = 0;
    int ret = SUCCESS;
    URLProtocol* pro = NULL;

    /// XXX Check Parameters
    if ((dmx == NULL) || (pro = dmx->m_Pro) == NULL)
    {
        errtyp = FATAL_ERROR;
        strcpy(errstr, "Parameter Error");
        goto TS_PARSE_SYNCHRONIZE_ERROR;
    }

    /// XXX Get Probe Data
    ret = pro->url_read(pro, prob_buf, SYN_PROBE_MAX_SIZE);
    if (0 == ret)
    {
        errtyp = COMMON_ERROR;
        strcpy(errstr, "File End Unexpected");
        goto TS_PARSE_SYNCHRONIZE_ERROR;
    }
    else if ((ret < 0) || (SYN_PROBE_MAX_SIZE != ret))
    {
        errtyp = FATAL_ERROR;
        strcpy(errstr, "Calling url_read Failed");
        goto TS_PARSE_SYNCHRONIZE_ERROR;
    }

    /// XXX Find Sync Byte Position
    while (sync_pos < SYN_PROBE_MAX_SIZE)
    {
        if (prob_buf[sync_pos] != TS_PACKET_SYN_BYTE)
        {
            if (sync_pos < TS_PACKET_SIZE_MAX)
            {
                ++sync_pos;
            }
            else
            {
                errtyp = PARSE_CANNOT_FIND_SYNC_BYTE;
                strcpy(errstr, "Cannot Find Sync Byte");
                goto TS_PARSE_SYNCHRONIZE_ERROR;
            }
        }

        if ((prob_buf[sync_pos + TS_PACKET_SIZE_188] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_188 * 2] == TS_PACKET_SYN_BYTE)\
            /*&& (prob_buf[sync_pos + TS_PACKET_SIZE_188 * 3] == TS_PACKET_SYN_BYTE)*/)
        {
            dmx->m_PacketLens = TS_PACKET_SIZE_188;
            break;
        }
        #ifdef  TS_SUPPORT_192_PKT
        if ((prob_buf[sync_pos + TS_PACKET_SIZE_192] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_192 * 2] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_192 * 3] == TS_PACKET_SYN_BYTE))
        {
            dmx->m_PacketLens = TS_PACKET_SIZE_192;
            break;
        }
        #endif/*TS_SUPPORT_192_PKT*/
        #ifdef  TS_SUPPORT_204_PKT
        if ((prob_buf[sync_pos + TS_PACKET_SIZE_204] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_204 * 2] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_204 * 3] == TS_PACKET_SYN_BYTE))
        {
            dmx->m_PacketLens = TS_PACKET_SIZE_204;
            break;
        }
        #endif/*TS_SUPPORT_204_PKT*/
        #ifdef  TS_SUPPORT_208_PKT
        if ((prob_buf[sync_pos + TS_PACKET_SIZE_204] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_204 * 2] == TS_PACKET_SYN_BYTE)\
            && (prob_buf[sync_pos + TS_PACKET_SIZE_204 * 3] == TS_PACKET_SYN_BYTE))
        {
            dmx->m_PacketLens = TS_PACKET_SIZE_204;
            break;
        }
        #endif/*TS_SUPPORT_208_PKT*/
        ++ sync_pos;
    }



    return PARSE_NO_ERROR;
TS_PARSE_SYNCHRONIZE_ERROR:
    mp_msg(0, MSGL_ERR, "DEMUX ################ %s\n", errstr);
    return errtyp;
}

/// @brief Parse TS packet header(4 Bytes)
BOOL TSParse_PktHeader (TSDemuxer* dmx, BitBuffer* buf)
{
    I8 retval = FAIL;
    I8 errtyp = MSGL_ERR;
    I8 errstr[64];
    I8 adaptation = 0;

    strcpy(errstr, "Parameter Error");
    if ((dmx == NULL) || (buf == NULL))
    {
        goto TS_PARSE_PKTHEADER_RET;
    }

    strcpy(errstr, "Parse Bit Buffer Failed");
    if (   (SkipSeverlBits (buf, 9))                             /*Transport Error and Sync Byte*/
        || (GetDataFromBitBuffer (buf,  1, &dmx->m_Header.m_PESPresent))      /*Payload Indicate*/
        || (SkipSeverlBits (buf, 1))                                        /*Transport priority*/
        || (GetDataFromBitBuffer (buf, 13, &dmx->m_Header.m_PID))                /*PID of packet*/
        || (SkipSeverlBits (buf, 2))                                                /*Scrambling*/
        || (GetDataFromBitBuffer (buf,  1, &adaptation))                       /*Adaptation flag*/
        || (GetDataFromBitBuffer (buf,  1, &dmx->m_Header.m_PLDPresent))          /*Payload flag*/
        || (SkipSeverlBits (buf, 4)))                                       /*Continuity counter*/
    {
        goto TS_PARSE_PKTHEADER_RET;
    }

    if (adaptation != 0U)
    {
        if (TSParse_AdapField (dmx, buf))
        {
            strcpy(errstr, "Parse Adaptation Field Failed");
            goto TS_PARSE_PKTHEADER_RET;
        }
    }

    retval = SUCCESS;
    errtyp = MSGL_V;
    strcpy(errstr, "Parse TS packet header OK");

TS_PARSE_PKTHEADER_RET:
    mp_msg(0, errtyp, "DEMUX ################ TSParse_PktHeader %s\n", errstr);
    return retval;
}
/// @brief Parse adaptation field control
BOOL TSParse_AdapField (TSDemuxer* dmx, BitBuffer* buf)
{
    I8 retval = FAIL;
    I8 errtyp = MSGL_ERR;
    I8 errstr[64];

    UI8 adap_field_len = 0U;
    UI8 adap_field_ext = 0U;

    strcpy(errstr, "Parameter Error");
    if ((dmx == NULL) || (buf == NULL))
    {
        goto TS_PARSE_ADAPFIELD_RET;
    }

    strcpy(errstr, "Parse Bit Buffer Failed");
    if (FAIL == GetDataFromBitBuffer (buf, 8, &adap_field_len))        /*Adaptation field length*/
    {
        goto TS_PARSE_ADAPFIELD_RET;
    }
    if (adap_field_len == 0)
    {
        retval = SUCCESS;
        errtyp = MSGL_V;
        strcpy(errstr, "No adaptation field data");
        goto TS_PARSE_ADAPFIELD_RET;
    }

    if (   (SkipSeverlBits (buf, 3)) /*Discontinuity, Random access & Elementary stream priority*/
        || (GetDataFromBitBuffer (buf, 1, &dmx->m_Header.m_PCRPresent))               /*PCR flag*/
        || (SkipSeverlBits (buf, 3))        /*OPCR, Splicing point & Transport private data flag*/
        || (GetDataFromBitBuffer (buf, 1, &adap_field_ext)))   /*Adaptation filed extension flag*/
    {
        goto TS_PARSE_ADAPFIELD_RET;
    }

    if (1 == dmx->m_Header.m_PCRPresent)
    {
        UI64 pcrbase;
        UI16 pcrextension;
        if (   (GetDataFromBitBuffer (buf, 33, &pcrbase))                             /*PCR base*/
            || (SkipSeverlBits (buf, 6))                                              /*reserved*/
            || (GetDataFromBitBuffer (buf,  9, &pcrextension)))                  /*PCR extension*/
        {
            goto TS_PARSE_ADAPFIELD_RET;
        }
        dmx->m_Header.m_PCRValue = (pcrbase * 300 + pcrextension);
    }

    /// Here need to skip the all left bits in adaptation field
    buf->m_BytePos = 4 + 1 + adap_field_len;
    buf->m_BitLeft = 8;

    retval = SUCCESS;
    errtyp = MSGL_V;
    strcpy(errstr, "Parse adaptation field OK");

TS_PARSE_ADAPFIELD_RET:
    mp_msg(0, errtyp, "DEMUX ################ TSParse_AdapField %s\n", errstr);
    return retval;
}
/// @brief Parse PSI filter
BOOL TSParse_PSIFilter (TSDemuxer* dmx, BitBuffer* buf)
{
    I8 retval = FAIL;
    I8 errtyp = MSGL_ERR;
    I8 errstr[64];
    PSISection* sec = NULL;

    strcpy(errstr, "Parameters Error");
    if ((dmx == NULL) || (buf = NULL))
    {
        goto TS_PARSE_PSIFILTER_RET;
    }

    sec = &dmx->m_Sections;

    strcpy(errstr, "Parse Bit Buffer Failed");
    if (   (SkipSeverlBits (buf,  8))                                            /*pointer field*/
        || (GetDataFromBitBuffer (buf,  8, &sec->m_TableID))                          /*table ID*/
        || (SkipSeverlBits (buf,  4))
        || (GetDataFromBitBuffer (buf, 12, &sec->m_SectionLength))              /*section length*/
        || (SkipSeverlBits (buf, 18))
        || (GetDataFromBitBuffer (buf,  5, &sec->m_VersionNum))                 /*version number*/
        || (GetDataFromBitBuffer (buf,  1, &sec->m_NextIndicator))      /*current next indicator*/
        || (GetDataFromBitBuffer (buf,  8, &sec->m_SectionNum))                 /*section number*/
        || (GetDataFromBitBuffer (buf,  8, &sec->m_LastSectionNum)))       /*last section number*/
    {
        goto TS_PARSE_PSIFILTER_RET;
    }

    retval = SUCCESS;
    errstr = MSGL_V;
    strcpy(errstr, "Parse PSI filter OK");

TS_PARSE_PSIFILTER_RET:
    mp_msg (0, errtyp, "DEMUX ################ TSParse_PSIFilter %s\n");
    return retval;
}
BOOL TSParse_PATPacket (TSDemuxer* dmx, BitBuffer* buf)
{
    I8   retval = FAIL;
    I8   errtyp = MSGL_V;
    I8   errstr[64];
    PSISection* sec = NULL;
    PATTable*   pat = NULL;
    int rows    = 0;
    int rownum  = 0;
    int index   = 0;

    strcpy(errstr, "Parameters Error");
    if ((dmx == NULL) || (buf == NULL))
    {
        goto TS_PARSE_PATPACKET_RET;
    }

    sec = &dmx->m_Sections;
    pat = &dmx->m_PATTable;

    if (sec->m_VersionNum != pat->m_VersionNum)
    {
        pat->m_TableDone  = FALSE;
        pat->m_SectionNum = 0;
        pat->m_VersionNum = 0;
        pat->m_PATRows    = 0;
    }

    if (pat->m_TableDone == TRUE)
    {
        retval = SUCCESS;
        errtyp = MSGL_V;
        strcpy(errstr, "Parse PAT OK");
        goto TS_PARSE_PATPACKET_RET;
    }

    rows   = pat->m_RowNumber;
    rownum = (sec->m_SectionLength - 9) / 4;

    strcpy(errstr, "Parse Bit Buffer Failed");
    for (index = 0; index < rownum; ++index)
    {

    }



TS_PARSE_PATPACKET_RET:
}

TSDmxErr TSParse_PMTPacket (TSDemuxer* dmx, BitBuffer* buf)
{
    I8   retval = FAIL;
    I8   errtyp = MSGL_V;
    I8   errstr[64];

    UI8  tableid;
    UI16 section_len;
    UI16 transport_stream_id;
    UI16 descriptor_len;

    strcpy(errstr, "Parameters Error");
    if ((dmx == NULL) || (buf == NULL))
    {
        goto TS_PARSE_PATPACKET_RET;
    }

    strcpy(errstr, "Parse Bit Buffer Failed");
    if (GetDataFromBitBuffer (buf, 8, &tableid))                                      /*table ID*/
    {
        goto TS_PARSE_PATPACKET_RET;
    }
    if (tableid != 0x02)
    {
        strcpy(errstr, "PAT table ID is invalid");
        goto TS_PARSE_PATPACKET_RET;
    }


}

TSDmxErr TSParse_PESPacket ();

TSDmxErr TSParse_PSIFilter ();

TSDmxErr TSParse_CRC_Check ();
