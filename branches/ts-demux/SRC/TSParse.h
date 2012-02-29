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


/// @brief Get a TS section
BOOL TSParse_GetSection (TSDemuxer* dmx);

/// @brief Parse TS packet header(4Byte)
/// @pre Have get a TS packet(with 188 byte) and initialized bit buffer
BOOL TSParse_PktHeader  (TSDemuxer* dmx);

#endif // TS_PARSE_H
