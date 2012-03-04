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
typedef enum   _StreadmID
{
    STREAM_ID_PROGRAM_MAP = 0xBC,   ///< Program stream map
    STREAM_ID_PRIVATE_1   = 0xBD,   ///< Private stream 1
    STREAM_ID_PADDING     = 0xBE,   ///< Padding stream
    STREAM_ID_PRIVATE_2   = 0xBF,   ///< Private stream 2
    STREAM_ID_ECM         = 0xF0,   ///< ECM stream
    STREAM_ID_EMM         = 0xF1    ///< EMM stream
}StreamID;
/// @brief Stream type
typedef enum   _StreamType
{
    STREAM_TYPE_MPEG_1V   = 0x01,   ///< MPEG-1 video subtype unknown
    STREAM_TYPE_MPEG_2V_1 = 0x02,   ///< MPEG-2 video subtype unknown
    STREAM_TYPE_MPEG_1A   = 0x03,   ///< MPEG-1 audio subtype unknown
    STREAM_TYPE_MPEG_2A   = 0x04,   ///< MPEG-2 audio subtype unknown
    STREAM_TYPE_AAC_ADTS  = 0x0F,   ///< AAC audio subtype ADTS
    STREAM_TYPE_MPEG_4V   = 0x10,   ///< MPEG-4 video subtype unknown
    STREAM_TYPE_AAC_LATM  = 0x11,   ///< AAC audio subtype LATM
    STREAM_TYPE_AVC_1     = 0x1B,   ///< AVC(H264) video subtype unknown
    STREAM_TYPE_AAC_UNKN  = 0x1C,   ///< AAC audio subtype unknown
    STREAM_TYPE_TEXT      = 0x1D,   ///< Text
    STREAM_TYPE_MPEG_2V_2 = 0x1E,   ///< MPEG-2 video subtype unknown
    STREAM_TYPE_AVC_2     = 0x1F,   ///< AVC(H264) video subtype unknown
    STREAM_TYPE_AVC_3     = 0x20    ///< AVC(H264) video subtype unknown
}StreamType;
/// @brief TS Packet Header Information\n
typedef struct _TSHeader
{
    UI16         m_PID;         ///< PID
    UI8          m_PESPresent;  ///< Indicate if packet contains PES or PSI
    UI8          m_PLDPresent;  ///< Indicate if packet contains payload data(PLD)
    UI8          m_PCRPresent;  ///< Indicate if packet contains PCR
    UI64         m_PCRBase;     ///< PCR base
    UI16         m_PCRExten;    ///< PCR extension
}TSHeader;
/// @brief TS section
typedef struct _TSection
{
    UI16         m_SectionType; ///< Section type
    UI8*         m_SectionData; ///< Section data
}TSection;
/// @brief TS packet
typedef struct _TSPacket
{
    UI8*         m_Data;        ///< Packet data
    UI64         m_Position;    ///< Packet position
}TSPacket;
/// @brief Pre-read section node
typedef struct _PackNode
{
    struct _TSPacket* m_Pack;
    struct _PackNode* m_Next;
}PrePacket, PacketList;
/// @brief TS Demuxer Information
typedef struct _TSDemuxer
{
    UI64         m_Duration;    ///< Current demux position
    UI64         m_FileSize;    ///< File size
    UI64         m_Position;    ///< Duration, If it's a live stream set as 0
    UI16         m_PMTPID;      ///< PMT PID, initialized with 0U
    UI16         m_AudioPID;    ///< Audio PID, initialized with 0U
    UI16         m_VideoPID;    ///< Video PID, initialized with 0U
    UI32         m_AudioCodec;  ///< Audio Codec
    UI32         m_VideoCodec;  ///< Video Codec
    TSection     m_Section;     ///< Current section
    PacketList   m_PktList;     ///< Pre-read list
    URLProtocol* m_Pro;         ///< Protocol interface for data IO
}TSDemuxer;

/// @brief Skip invalid TS packet and synchronize sync byte to initialize parsing start position
/// @note  This method is just called once at the beginning of parsing transport stream
BOOL TSParse_InitParser (TSDemuxer* dmx);
/// @brief Get a TS packet from transport stream
/// @note  The demux position will be modified
/// @pre   The pre-read packet list is null
/// @param data Allocated space for storing a TS packet
BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8* data);
/// @brief Add a pre-read packet
/// @param data Space storing a TS packet
/// @param pos  Start position of the TS packet assigned by data
/// @note  Parameter data will be freed by method #TSParse_DelPrePack
BOOL TSParse_AddPrePack (TSDemuxer* dmx, UI8* data, UI64 pos);
/// @brief Delete a pre-read packet
/// @pre   The packet in pre-read packet list is used
/// @param pos Start position of the pre-packet which will be deleted
BOOL TSParse_DelPrePack (TSDemuxer* dmx, UI8* data);
/// @brief Parse TS packet header(4Byte)
/// @pre   Have get a TS packet(with 188 byte) and initialized bit buffer
BOOL TSParse_PackHeader (TSHeader* head, UI8* data);
/// @brief Get and parse PAT section to set PMT PID
BOOL TSParse_PATSection (TSDemuxer* dmx);
/// @brief Get and parse PMT section to set audio and video PID
/// @pre   PMT PID have been set
BOOL TSParse_PMTSection (TSDemuxer* dmx);
/// @brief Get stream duration
/// @pre   Audio's PID and video's PID have been set
BOOL TSParse_TSDuration (TSDemuxer* dmx);
/// @brief Get an audio or a video section
/// @pre   PAT's PID, PMT's PID audio's PID and video's PID have been set
BOOL TSParse_PESSection (TSDemuxer* dmx);
#endif/*TS_PARSE_H*/
