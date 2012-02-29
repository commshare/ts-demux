#ifndef TS_PARSE_H
#define TS_PARSE_H

#include "bitbuffer.h"
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
#define TS_PACKET_SIZE_MAX  208     ///< Maximum packet size
#define SYN_PROBE_MAX_SIZE  832     ///< Maximum probe size, 4 times the length of maximum pakcet
#define TS_PACKET_SYN_BYTE   71     ///< TS sync byte
#define MAX_PREREAD_PACKET  100     ///< Maximum pre-read packets
#define MAX_ROWS_IN_PSI_PKT  32     ///< Maximum rows in PSI packet

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
/// @brief TS Packet Header Infromation\n
typedef struct _TSHeader
{
    UI16  m_PID;            ///< PID
    UI8   m_PESPresent;     ///< Indicate if packet contains Packetized Elementary Stream(PES)
    UI8   m_PLDPresent;     ///< Indicate if packet contains payload data(PLD)
}TSHeader;
typedef struct _TSPSIPkt
{
    UI8   m_TableID;

}TSPATPkt;
typedef struct _TSPESPkt
{

}TSPESPkt;
/// @brief TS section
typedef struct _TSection
{
    UI16         m_SectionType; ///< Packet type
    void*        m_SectionHead;
    UI8*         m_SectionData; ///< Packet data
    BitBuffer    m_BitBuf;  ///< Bit buffer
}TSection;
/// @brief Pre-read section node
typedef struct _PreNode
{
    UI8*
    struct _PreNode* m_Next;
}PreNode, PreList;
/// @brief TS Demuxer Information
typedef struct _TSDemuxer
{
    UI16         m_PMTPID;  ///< PMT PID, initialized with 0U
    UI16         m_AudioPID;///< Audio PID, initialized with 0U
    UI16         m_VideoPID;///< Video PID, initialized with 0U
    TSection     m_Section; ///< Current section
    PreList      m_PreList; ///< Pre-read list
    URLProtocol* m_Pro;     ///< Protocol interface for data IO
    UI64         m_DmxPos;  ///< Current demux position
    UI64         m_FileSize;///< File size
    UI64         m_Duration;///< Duration, If it's a live stream set as 0
}TSDemuxer;


/// @brief Get a TS audio or video section from current demux position
/// @pre Have parsed PAT and PMT section and got audio and video PID
/// @note This method will skip useless TS packet and modify demux position
BOOL TSParse_GetSection (TSDemuxer* dmx);
BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8* tspkt);
BOOL TSParse_AddAPacket (TSDemuxer* dmx, UI8* tspkt);
/// @brief Parse TS packet header(4Byte)
/// @pre Have get a TS packet(with 188 byte) and initialized bit buffer
BOOL TSParse_PackHeader (TSDemuxer* dmx, TSHeader* header);
/// @brief Parse PAT Section
BOOL TSParse_PATSection (TSDemuxer* dmx, TSPATPkt* patpkt);
/// @brief Parse PMT section and get audio and video PID
/// @pre Have parsed PAT section and got PMT PID
BOOL TSParse_PMTSection (TSDemuxer* dmx, TSPMTPkt* pmtpkt);

#endif // TS_PARSE_H
