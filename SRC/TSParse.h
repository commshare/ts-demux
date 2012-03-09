#ifndef TS_PARSE_H
#define TS_PARSE_H

#include "BitBuffer.h"
#include "../avformat.h"
#include "../urlprotocol.h"

#ifndef BASE_DEFINED
#define BASE_DEFINED
#define SUCCESS               0     ///< Return on success
#define FAIL                 -1     ///< Retrun on error
#define FALSE                 0     ///< False flag
#define TRUE                  1     ///< True flag
#endif/*BASE_DEFINED*/
#define TS_PACKET_SIZE_188  188     ///< 188 bytes ts packet
#define TS_PACKET_SIZE_192  192     ///< 192 bytes ts packet
#define TS_PACKET_SIZE_204  204     ///< 204 bytes ts packet
#define TS_PACKET_SIZE_208  208     ///< 208 bytes ts packet
#define TS_PACKET_SIZE_MAX  208     ///< Maximum ts packet length
#define TS_PACKET_SYN_BYTE 0x47     ///< TS sync byte
#define PSI_PACKET_HEADER_LEN 4     ///< PSI packet header length
#define PES_PACKET_HEADER_LEN 6     ///< PES packet header length

#ifndef BASE_TYPEDEF
#define BASE_TYPEDEF
typedef char               I8, BOOL;
typedef short              I16;
typedef long               I32;
typedef long long          I64;
typedef unsigned char      UI8;
typedef unsigned short     UI16;
typedef unsigned long      UI32;
typedef unsigned long long UI64;
#endif/*BASE_TYPEDEF*/
/// @brief Stream ID
typedef enum   _StreamID
{
    STREAM_ID_PROGRAM_MAP = 0xBC,   ///< Program stream map
    STREAM_ID_PRIVATE_1   = 0xBD,   ///< Private stream 1
    STREAM_ID_PADDING     = 0xBE,   ///< Padding stream
    STREAM_ID_PRIVATE_2   = 0xBF,   ///< Private stream 2
    STREAM_ID_ECM         = 0xF0,   ///< ECM stream
    STREAM_ID_EMM         = 0xF1,   ///< EMM stream
    STREAM_ID_DSM_CC      = 0xF2,   ///< DSM_CC stream
    STREAM_ID_13522       = 0xF3,   ///< ISO/IEC 13522 stream
    STREAM_ID_PRO_DIREC   = 0xFF    ///< Progream stream directory
}StreamID;
/// @brief Parsing level
typedef enum   _ParseLev
{
    PARSE_LEV_PAT,                  ///< PAT has not been parsed
    PARSE_LEV_PMT,                  ///< PMT has not been parsed
    PARSE_LEV_PES                   ///< PAT and PMT have been parsed
}ParseLev;
/// @brief TS section
typedef struct _TSection
{
    BOOL         m_Valid;           ///< PES stream id
    UI16         m_Type;            ///< Section type
    UI8*         m_Data;            ///< Section data
    UI64         m_Positon;         ///< Section Positon;
    UI64         m_DataLen;         ///< Section data length
    UI64         m_BuffLen;         ///< Section data buffer length
}TSection;
/// @brief TS packet
typedef struct _TSPacket
{
    UI8*              m_Data;       ///< Packet data
    UI64              m_Position;   ///< Packet position
    struct _TSPacket* m_Next;
}TSPacket;
/// @brief TS Demuxer Information demux
typedef struct _TSDemuxer
{
    UI64         m_Duration;        ///< Current demux position
    UI64         m_FileSize;        ///< File size
    UI64         m_Position;        ///< Duration, If it's a live stream set as 0
    UI16         m_PMTPID;          ///< PMT PID
    UI16         m_AudioPID;        ///< Audio PID
    UI16         m_VideoPID;        ///< Video PID
    TSection*    m_Section;         ///< Current section
    TSPacket*    m_PreListHeader;   ///< Pre-read list
    UI32         m_PreListLen;      ///< Pre-read list length;
    URLProtocol* m_Pro;             ///< Protocol interface for data IO
}TSDemuxer;

/// @brief Skip invalid TS packet and synchronize sync byte to initialize parsing start position
/// @note  This method is just called once at the beginning of parsing or after seeking
BOOL TSParse_InitParser (TSDemuxer* dmx);
/// @brief Get a TS packet from transport stream
/// @note  The demux position will be modified
/// @pre   The pre-read packet list is null
/// @param data Allocated space for storing a TS packet
BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8** pack, int* len);
/// @brief Add a pre-read packet
/// @param data Space storing a TS packet
/// @param pos  Start position of the TS packet assigned by data
/// @note  Parameter data will be freed by method #TSParse_DelPrePack , the reference of 'pack' to\n
/// previous packet data will be no longer in force\n
BOOL TSParse_AddPrePack (TSDemuxer* dmx, UI8** pack, UI64 pos);
/// @brief Delete a pre-read packet
/// @pre   The packet in pre-read packet list is used
/// @param pos Start position of the pre-packet which will be deleted
/// @note  The packet which is assigned by 'pack' will be freed.
BOOL TSParse_DelPrePack (TSDemuxer* dmx, UI8** pack, UI64 pos);
/// @brief Get a section
BOOL TSParse_GetSection (TSDemuxer* dmx);
/// @brief Get and parse PAT section to set PMT PID
BOOL TSParse_PATSection (TSDemuxer* dmx);
/// @brief Get and parse PMT section to set audio and video PID
/// @pre   PMT PID have been set
BOOL TSParse_PMTSection (TSDemuxer* dmx, Metadata* meta);
/// @brief Get an audio or a video section
/// @pre   PAT's PID, PMT's PID audio's PID and video's PID have been set
BOOL TSParse_PESSection (TSDemuxer* dmx, AVPacket* pack);
/// @brief Parse TS packet header(4Byte)
/// @pre   Have get a TS packet(with 188 byte) and initialized bit buffer
/// @param ofs Offset to payload data
/// @param pes Indicate if PES packet header is present
BOOL TSParse_TSPacketHeader (const UI8* data, UI16* pkt_PID, UI16* ofs, BOOL* pes);
/// @brief Parse PES section header
/// @param len Indicate section length
/// @param val Indicate if this section is valid(by stream ID)
BOOL TSParse_ParsePESHeader (const UI8* data, UI16  datalen, UI16* len, BOOL* val);
/// @brief Parse PSI(PAT/PMT) section header
/// @param len Indicate section length
BOOL TSParse_ParsePSIHeader (const UI8* data, UI16  datalen, UI16* len);
#endif/*TS_PARSE_H*/
