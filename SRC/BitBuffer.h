/// @file     BitBuffer.h
/// @brief    Bit buffer
/// @author   birdyfj@gmail.com
/// @version  1.0
/// @date     2012-02-08

#ifndef BITBUFFER_H
#define BITBUFFER_H

#ifndef BASE_DEFINED
#define BASE_DEFINED
#define SUCCESS               0     ///< Return on success
#define FAIL                 -1     ///< Retrun on error
#define FALSE                 0     ///< False flag
#define TRUE                  1     ///< True flag
#endif/*BASE_DEFINED*/

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
/// @brief Bit buffer
typedef struct BitBuffer
{
    UI8* m_Data;    ///< BitBuffer data
    I32  m_Length;  ///< BitBuffer data length
    I32  m_BytePos; ///< Current read byte
    I32  m_BitLeft; ///< Current byte unread bits number
}BitBuffer;

/// @brief Initialize bit buffer
/// @note  If you wanna to initialize a bit buffer for writing data, please memset the data before
BOOL InitiBitBuffer (BitBuffer** buf, UI8* data, I32 length);
/// @brief Close and destroy bit buffer
/// @note  The member #BitBuffer::m_Data will be freed by the one who create it
void CloseBitBuffer (BitBuffer** buf);
/// @brief Check if bit buffer is availalbe before Get or Put data
BOOL CheckBitBuffer (BitBuffer* buf, I32 bits);
/// @brief Skip some bits
BOOL SkipSeverlBits (BitBuffer* buf, I32 bits);
/// @brief Get data from bit buffer
BOOL GetDataFromBitBuffer_08 (BitBuffer* buf, I32 bits, UI8 * val);
BOOL GetDataFromBitBuffer_16 (BitBuffer* buf, I32 bits, UI16* val);
BOOL GetDataFromBitBuffer_32 (BitBuffer* buf, I32 bits, UI32* val);
BOOL GetDataFromBitBuffer_64 (BitBuffer* buf, I32 bits, UI64* val);
BOOL GetDataFromBitBuffer    (BitBuffer* buf, I32 bits, void* val);
/// @brief Pub data to bit buffer
BOOL WriteDataToBitBuffer    (BitBuffer* buf, I32 bits, UI64  val);

#endif/*BITBUFFER_H*/
