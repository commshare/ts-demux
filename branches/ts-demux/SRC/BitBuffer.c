#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include "BitBuffer.h"

static UI8 bytemask[9] =
{
    0x00,
    0x01, 0x03, 0x07, 0x0F,
    0x1F, 0x3F, 0x7F, 0xFF
};
BOOL InitiBitBuffer (BitBuffer** buf, const UI8* data, I32 length)
{
    if (length == 0L || data == NULL)
    {
        return FAIL;
    }

    CloseBitBuffer(buf);

    *buf = (BitBuffer*)malloc(sizeof(BitBuffer));
    if (*buf == NULL)
    {
        return FAIL;
    }

    (*buf)->m_Data    = data;
    (*buf)->m_Length  = length;
    (*buf)->m_BytePos = 0L;
    (*buf)->m_BitLeft = 8L;

    return SUCCESS;
}
void CloseBitBuffer (BitBuffer** buf)
{
    if (*buf != NULL)
    {
        free (*buf);
        *buf = NULL;
    }
}
BOOL CheckBitBuffer (BitBuffer* buf, I32 bits)
{
    I32 bitpos  = 0L;
    I32 bytepos = 0L;

    if ((buf == NULL) || (buf->m_Data == NULL) || (buf->m_Length == buf->m_BytePos))
    {
        return FAIL;
    }

    bytepos = buf->m_BitLeft + bits;
    bytepos = buf->m_BytePos + (bitpos / 8);
    bitpos  = bitpos % 8;

    if ((bytepos > buf->m_Length) || (bytepos == buf->m_Length && bitpos != 0))
    {
        return FAIL;
    }

    return SUCCESS;
}
BOOL SkipSeverlBits (BitBuffer* buf, I32 bits)
{
    if (CheckBitBuffer(buf, bits) == FAIL)
    {
        return FAIL;
    }

    while (bits > 0)
    {
        if (bits >= buf->m_BitLeft)
        {
            bits -= buf->m_BitLeft;
            buf->m_BitLeft  = 8;
            buf->m_BytePos += 1;
        }
        else
        {
            buf->m_BitLeft -= bits;
            bits = 0;
        }
    }

    return SUCCESS;
}
BOOL GetDataFromBitBuffer_08 (BitBuffer *buf, I32 bits, UI8 * val)
{
    UI8 d = 0U;

    if (CheckBitBuffer(buf, bits) == FAIL)
    {
        return FAIL;
    }

    while (bits > 0)
    {
        if(bits >= buf->m_BitLeft)
        {
            d |= (buf->m_Data[buf->m_BytePos] & bytemask[buf->m_BitLeft])\
                << (bits - buf->m_BitLeft);
            bits -= buf->m_BitLeft;
            buf->m_BitLeft  = 8;
            buf->m_BytePos += 1;
        }
        else
        {
            d |= ((buf->m_Data[buf->m_BytePos] >> (buf->m_BitLeft - bits)) & bytemask[bits]);
            buf->m_BitLeft -= bits;
            bits = 0;
        }
    }

    *val = d;

    return SUCCESS;
}
BOOL GetDataFromBitBuffer_16 (BitBuffer *buf, I32 bits, UI16* val)
{
    UI16 d = 0U;

    if (CheckBitBuffer(buf, bits) == FAIL)
    {
        return FAIL;
    }

    while (bits > 0)
    {
        if(bits >= buf->m_BitLeft)
        {
            d |= (buf->m_Data[buf->m_BytePos] & bytemask[buf->m_BitLeft])\
                << (bits - buf->m_BitLeft);
            bits -= buf->m_BitLeft;
            buf->m_BitLeft  = 8;
            buf->m_BytePos += 1;
        }
        else
        {
            d |= ((buf->m_Data[buf->m_BytePos] >> (buf->m_BitLeft - bits)) & bytemask[bits]);
            buf->m_BitLeft -= bits;
            bits = 0;
        }
    }

    *val = d;

    return SUCCESS;
}
BOOL GetDataFromBitBuffer_32 (BitBuffer *buf, I32 bits, UI32* val)
{
    UI32 d = 0UL;

    if (CheckBitBuffer(buf, bits) == FAIL)
    {
        return FAIL;
    }

    while (bits > 0)
    {
        if(bits >= buf->m_BitLeft)
        {
            d |= (buf->m_Data[buf->m_BytePos] & bytemask[buf->m_BitLeft])\
                << (bits - buf->m_BitLeft);
            bits -= buf->m_BitLeft;
            buf->m_BitLeft  = 8;
            buf->m_BytePos += 1;
        }
        else
        {
            d |= ((buf->m_Data[buf->m_BytePos] >> (buf->m_BitLeft - bits)) & bytemask[bits]);
            buf->m_BitLeft -= bits;
            bits = 0;
        }
    }

    *val = d;

    return SUCCESS;
}
BOOL GetDataFromBitBuffer_64 (BitBuffer *buf, I32 bits, UI64* val)
{
    UI64 d = 0ULL;

    if (CheckBitBuffer(buf, bits) == FAIL)
    {
        return FAIL;
    }

    while (bits > 0)
    {
        if(bits >= buf->m_BitLeft)
        {
            d |= (buf->m_Data[buf->m_BytePos] & bytemask[buf->m_BitLeft])\
                << (bits - buf->m_BitLeft);
            bits -= buf->m_BitLeft;
            buf->m_BitLeft  = 8;
            buf->m_BytePos += 1;
        }
        else
        {
            d |= ((buf->m_Data[buf->m_BytePos] >> (buf->m_BitLeft - bits)) & bytemask[bits]);
            buf->m_BitLeft -= bits;
            bits = 0;
        }
    }

    *val = d;

    return SUCCESS;
}
BOOL GetDataFromBitBuffer (BitBuffer* buf, I32 bits, void* val)
{
    if (bits >= 1 && bits <= 8)
    {
        return GetDataFromBitBuffer_08 (buf, bits, (UI8 *)val);
    }
    else if (bits >=  9 && bits <= 16)
    {
        return GetDataFromBitBuffer_16 (buf, bits, (UI16*)val);
    }
    else if (bits >= 17 && bits <= 32)
    {
        return GetDataFromBitBuffer_32 (buf, bits, (UI32*)val);
    }
    else if (bits >= 33 && bits <= 64)
    {
        return GetDataFromBitBuffer_64 (buf, bits, (UI64*)val);
    }
    else
    {
        return FAIL;
    }
}
