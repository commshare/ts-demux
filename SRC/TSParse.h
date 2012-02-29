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
    STREAM_ID_PROGRAM_MAP,
    STREAM_ID_PRIV_1,
    STREAM_ID_PADDING_STREAM,
    STREAM_ID_PRIV_2,
    STREAM_ID_ECM,
    STREAM_ID_EMM
}StreamID;
/// @brief Stream type
typedef enum   _StreamType
{
    STREAM_TYPE_M_11172     = 0x01,
    STREAM_TYPE_M_H262      = 0x02,
    STREAM_TYPE_M_11172     = 0x03,
    STREAM_TYPE_M_13818_1   = 0x04,
    STREAM_TYPE_O_H222_1    = 0x05,
    STREAM_TYPE_O_PES       = 0x06,
    STREAM_TYPE_O_13522     = 0x07,
    STREAM_TYPE_O_DSMCC     = 0x08,
    STREAM_TYPE_O_H222_2    = 0x09,
    STREAM_TYPE_O_13818_2   = 0x0A,
    STREAM_TYPE_O_13818_3   = 0x0B,
    STREAM_TYPE_O_13818_4   = 0x0C,
    STREAM_TYPE_O_13818_5   = 0x0D,
    STREAM_TYPE_O_13818_5   = 0x0E,
    STREAM_TYPE_M_MPEG2_AAC = 0x0F,
    STREAM_TYPE_M_MPEG4     = 0x10,
    STREAM_TYPE_M_MPEG4_AAC = 0x11,
    STREAM_TYPE_M_H264      = 0x1B,
    STREAM_TYPE_M_AVS       = 0x42,
    STREAM_TYPE_M_AC3       = 0x81
}StreamType;
/// @brief PSI packet header
typedef struct _PATHeader
{
}PATHeader;
typedef struct _PMTHeader
{}PMTHeader;
/// @brief PES packet header
typedef struct _PESHeader
{}PESHeader;
/// @brief TS Packet Header Infromation\n
typedef struct _TSHeader
{
    UI16  m_PID;            ///< PID
    UI8   m_PESPresent;     ///< Indicate if packet contains Packetized Elementary Stream(PES)
    UI8   m_PLDPresent;     ///< Indicate if packet contains payload data(PLD)
}TSHeader;
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
    TSection*    m_Section;
    struct _PreNode* m_Next;
}PreNode;
/// @brief TS Demuxer Information
typedef struct _TSDemuxer
{
    UI16         m_PMTPID;  ///< PMT PID, initialized with 0U
    UI16         m_AudioPID;///< Audio PID, initialized with 0U
    UI16         m_VideoPID;///< Video PID, initialized with 0U
    TSection     m_Section; ///< Current section
    PreNode      m_PreList; ///< Pre-read list
    URLProtocol* m_Pro;     ///< Protocol interface for data IO
    UI64         m_DmxPos;  ///< Current demux position
    UI64         m_FileSize;///< File size
    UI64         m_Duration;///< Duration, If it's a live stream set as 0
}TSDemuxer;


/// @brief Get a TS audio or video section from current demux position
/// @pre Have parsed PAT and PMT section and got audio and video PID
/// @note This method will skip useless TS packet and modify demux position
BOOL TSParse_GetSection (TSDemuxer* dmx);
/// @brief Get a TS packet
/// @note This method won't modify demux position
BOOL TSParse_GetAPacket (TSDemuxer* dmx);
/// @brief Add a pre-read TS packet
/// @note This method won't modify demux position
BOOL TSParse_AddAPacket (TSDemuxer* dmx);
/// @brief Parse TS packet header(4Byte)
/// @pre Have get a TS packet(with 188 byte) and initialized bit buffer
BOOL TSParse_PackHeader (TSDemuxer* dmx);
/// @brief Parse PAT Section
BOOL TSParse_PATSection (TSDemuxer* dmx);
/// @brief Parse PMT section and get audio and video PID
/// @pre Have parsed PAT section and got PMT PID
BOOL TSParse_PMTSection (TSDemuxer* dmx);

#endif // TS_PARSE_H
