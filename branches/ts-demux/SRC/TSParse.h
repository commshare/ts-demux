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
/// @brief Describe the state of current demuxing
typedef enum   _TSDmxState
{
    PARSE_NOT_START = 0x00, ///< Not start
    PARSED_PAT      = 0x01, ///< PAT has parsed
    PARSED_PMT      = 0x02, ///< PMT has parsed
    PARSING_AV      = 0x03  ///< Parsing AV data
}TSDmxState;
typedef enum   _TSPSIType
{
    PAT_SECTION     = 0x00, ///< PAT section, fixed with 0x00
    CAT_SECTION     = 0x01, ///< CAT section, fixed with 0x01
    FORBIDDEN       = 0xFF  ///< PID is 0xFF is forbidden, and other PID is for private sections
}TSPSIType;
/// @brief PSI Section Information
typedef struct _PSISection
{
    UI8   m_TableID;        ///< Table ID
    UI16  m_SectionLength;  ///< Section length
    UI8   m_VersionNum;     ///< Version number
    UI8   m_NextIndicator;  ///< Current next indicator
    UI8   m_SectionNum;     ///< Section number
    UI8   m_LastSectionNum; ///< Last section number
}PSISection;
/// @brief PMT Information
typedef struct _PMTInfor
{
    UI8   m_StreamType;
    UI16  m_ElementaryPID;
}PMTInfor;
/// @brief PMT Table
typedef struct _PMTTable
{
    PMTInfor m_Video;
    PMTInfor m_Audio;
}PMTTable;
/// @brief TS Packet Header Infromation\n
typedef struct _TSHeader
{
    UI16  m_PID;            ///< PID
    UI8   m_PESPresent;     ///< Indicate if packet contains Packetized Elementary Stream(PES)
    UI8   m_PLDPresent;     ///< Indicate if packet contains payload data(PLD)
}TSHeader;
/// @brief TS Demuxer Information
typedef struct _TSDemuxer
{
    TSDmxState   m_DmxState;///< Current demuxing status
    UI8*         m_PktData; ///< Current packet data
    TSHeader     m_Header;  ///< Current TS header information
    UI16         m_PMTPID;  ///< PMT PID
    PSISection   m_Sections;///< PSI sections
    PMTTable     m_PMTTable;///< PMT table
    URLProtocol* m_Pro;     ///< Protocol interface for data IO
    UI64         m_DmxPos;  ///< Current demux position
    UI64         m_Duration;///< Duration, If it's a live stream set as 0
}TSDemuxer;


/// @brief Parse TS packet header(4Byte)
BOOL TSParse_PktHeader (TSDemuxer* dmx, BitBuffer* buf);

#endif // TS_PARSE_H
