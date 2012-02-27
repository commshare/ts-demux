#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

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
        UI8 adap_len;
        if (GetDataFromBitBuffer (buf,  8, &adap_len))                 /*adaptation field length*/
        {
            goto TS_PARSE_PKTHEADER_RET;
        }
        if ((buf->m_Length - buf->m_BytePos) >= adap_len)
        {
            buf->m_BytePos -= adap_len;
        }
    }

    retval = SUCCESS;
    errtyp = MSGL_V;
    strcpy(errstr, "Parse TS packet header OK");

TS_PARSE_PKTHEADER_RET:
    mp_msg(0, errtyp, "DEMUX ################ TSParse_PktHeader %s\n", errstr);
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

TSDmxErr TSParse_PESPacket (TSDemuxer* dmx, BitBuffer* buf)
{
}

