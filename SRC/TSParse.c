#include <malloc.h>
#include <string.h>
#include "TSParse.h"
#include "demux_log.h"
#include "../avformat.h"

typedef struct BitContext {
    const UI8* buffer;
    int  cur;
    int  size_in_bits;
} BitContext;
static UI8 log_tab[256] = {
    0 , 0 , 1 , 1 , 2 , 2 , 2 , 2 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /*   0 ~  15 */
    4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , /*  16 ~  31 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  32 ~  47 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  48 ~  63 */
    6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , /*  64 ~  79 */
    6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , /*  80 ~  95 */
    6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , /*  96 ~ 111 */
    6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , /* 112 ~ 127 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 128 ~ 143 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 144 ~ 159 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 160 ~ 175 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 176 ~ 191 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 192 ~ 207 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 208 ~ 223 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /* 224 ~ 239 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7   /* 240 ~ 255 */
};
static UI8 vlc_len[512] = {
    14, 13, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, /*   0 ~  15 */
    9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , 9 , /*  16 ~  31 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /*  32 ~  47 */
    7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , /*  48 ~  63 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  64 ~  79 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  80 ~  95 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  96 ~ 111 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /* 112 ~ 127 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 128 ~ 143 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 144 ~ 159 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 160 ~ 175 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 176 ~ 191 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 192 ~ 207 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 208 ~ 223 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 224 ~ 239 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /* 240 ~ 255 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 256 ~ 271 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 272 ~ 287 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 288 ~ 303 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 304 ~ 319 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 320 ~ 335 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 336 ~ 351 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 352 ~ 367 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 368 ~ 383 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 384 ~ 399 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 400 ~ 415 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 416 ~ 431 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 432 ~ 447 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 448 ~ 463 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 464 ~ 479 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 480 ~ 495 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 496 ~ 511 */
};
static UI8 vlc_cod[512] = {
    31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, /*   0 ~  15 */
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, /*  16 ~  31 */
    7 , 7 , 7 , 7 , 8 , 8 , 8 , 8 , 9 , 9 , 9 , 9 , 10, 10, 10, 10, /*  32 ~  47 */
    11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, /*  48 ~  63 */
    3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , /*  64 ~  79 */
    4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , /*  80 ~  95 */
    5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , /*  96 ~ 111 */
    6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , 6 , /* 112 ~ 127 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 128 ~ 143 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 144 ~ 159 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 160 ~ 175 */
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , /* 176 ~ 191 */
    2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , /* 192 ~ 207 */
    2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , /* 208 ~ 223 */
    2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , /* 224 ~ 239 */
    2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , /* 240 ~ 255 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 256 ~ 271 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 272 ~ 287 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 288 ~ 303 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 304 ~ 319 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 320 ~ 335 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 336 ~ 351 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 352 ~ 367 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 368 ~ 383 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 384 ~ 399 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 400 ~ 415 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 416 ~ 431 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 432 ~ 447 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 448 ~ 463 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 464 ~ 479 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 480 ~ 495 */
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , /* 496 ~ 511 */
};
static int  av_log2  (UI32 v)
{
    int n = 0;
    if (v & 0xffff0000)
    {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00)
    {
        v >>= 8;
        n += 8;
    }
    n += log_tab[v];
    return n;
}
static void AV_RL16  (UI16 *buf)
{
    UI8* temp = (UI8*)buf;
    UI16 tm3 = temp[0] << 8;
    UI16 tm4 = temp[1];
    *buf = tm3 | tm4;
}
static void AV_RL32  (UI32 *buf)
{
    UI8* temp = (UI8*)buf;
    UI32 tm1 = temp[0] << 24;
    UI32 tm2 = temp[1] << 16 ;
    UI32 tm3 = temp[2] << 8;
    UI32 tm4 = temp[3];
    *buf = tm1 | tm2 |tm3 | tm4;
}
static void init_bits(BitContext *s, const UI8 *buffer, UI32 bit_size)
{
    UI32 buffer_size= (bit_size + 7) >> 3;
    if (buffer_size < 0 || bit_size < 0)
    {
        buffer_size = bit_size = 0;
        buffer = NULL;
    }
    s->buffer = buffer;
    s->size_in_bits = bit_size;
    s->cur = 0;
}
static void Skip_ue_golomb (BitContext *s)
{
    UI32 buf;
    int log;
    int offset = s->cur >> 3;
    memcpy(&buf, s->buffer + offset, 4);
    AV_RL32(&buf);
    buf <<= (s->cur & 0x07);
    if(buf >= (1 << 27))
    {
        buf >>= 32 - 9;
        s->cur += vlc_len[buf];
    }
    else
    {
        log= 2 * av_log2(buf) - 31;
        buf >>= log;
        s->cur += 32 - log;
    }
}
static int Get_ue_golomb_31(BitContext *s)
{
    UI32 buf;
    int offset = s->cur >> 3;
    memcpy(&buf, s->buffer + offset, 4);
    AV_RL32(&buf);
    buf <<= (s->cur & 0x07);
    buf >>= (32 - 9);
    s->cur += vlc_len[buf];
    return vlc_cod[buf];
}

BOOL TSParse_InitParser (TSDemuxer* dmx)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    /// Packet data for synchronizing and initialize parsing start position\n
    /// Read one more byte than four longest TS packets data. Use 207 byte to find sync byte(0x47)\n
    /// at most, and the left data to check if found the start position of the 1st valid TS packet
    UI8  probebuf[(TS_PACKET_SIZE_MAX << 2) + 1];
    int  probebuflen = (TS_PACKET_SIZE_MAX << 2) + 1;   ///< Probe buffer data length
    int  tspkt_len   = 0;                               ///< TS packet length
    int  start_pos   = 0;                               ///< The 1st TS packet position
    UI8* temp_pkt    = NULL;
    int  flag        = FAIL;                            ///< Temporary flag during process
    URLProtocol* pro = dmx->m_Pro;                      /// Demuxer data IO protocol

    /// Step-1 : Get probe buffer data
    if (probebuflen != (flag = pro->url_read(pro, probebuf, probebuflen)))
    {
        if (flag == 0)
        {
            msg = "Stream end unexpectedly";
        }
        msg = "Calling url_read failed";
        goto TSPARSE_INITPARSER_RET;
    }
    dmx->m_Position += probebuflen;

    /// Step-2 : Get the first valid sync byte position
    while (1)
    {
        while (start_pos < TS_PACKET_SIZE_MAX - 1)
        {
            if (probebuf[start_pos] == 0x47)
            {
                break;
            }
            ++start_pos;
        }
        if (start_pos == TS_PACKET_SIZE_MAX - 1)
        {
            msg = "Cannot find valid sync byte";
            goto TSPARSE_INITPARSER_RET;
        }

        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_188 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_188;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_192 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_192;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_204 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_204;
            break;
        }
        if ((TS_PACKET_SYN_BYTE == probebuf[start_pos])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208 * 2])\
         && (TS_PACKET_SYN_BYTE == probebuf[start_pos + TS_PACKET_SIZE_208 * 3]))
        {
            tspkt_len = TS_PACKET_SIZE_208;
            break;
        }
        ++start_pos;
    }

    /// Step-3 : Save the all probe buffer data to pre-read packet list
    probebuflen -= start_pos;
    while (probebuflen >= TS_PACKET_SIZE_188)
    {
        if (temp_pkt == NULL && NULL == (temp_pkt = (UI8*)malloc(TS_PACKET_SIZE_188)))
        {
            msg = "Allocate a pre-read packet failed";
            goto TSPARSE_INITPARSER_RET;
        }
        memcpy(temp_pkt, probebuf + start_pos, TS_PACKET_SIZE_188);
        if (FAIL == TSParse_AddPrePack(dmx, &temp_pkt, (UI64)start_pos))
        {
            msg = "Add a pre-read packet failed";
            goto TSPARSE_INITPARSER_RET;
        }
        start_pos   += tspkt_len;
        probebuflen -= tspkt_len;
    }
    if (temp_pkt == NULL && NULL == (temp_pkt = (UI8*)malloc(TS_PACKET_SIZE_188)))
    {
        msg = "Allocate a pre-read packet failed";
        goto TSPARSE_INITPARSER_RET;
    }
    memcpy(temp_pkt, probebuf + start_pos, probebuflen);
    flag = pro->url_read(pro, temp_pkt + probebuflen, TS_PACKET_SIZE_188 - probebuflen);
    if (flag != TS_PACKET_SIZE_188 - probebuflen)
    {
        if (flag == 0)
        {
            msg = "Stream end unexpectedly";
            goto TSPARSE_INITPARSER_RET;
        }
        msg = "Calling url_read failed";
        goto TSPARSE_INITPARSER_RET;
    }
    dmx->m_Position += (TS_PACKET_SIZE_188 - probebuflen);
    if (FAIL == TSParse_AddPrePack(dmx, &temp_pkt, start_pos))
    {
        msg = "Add a pre-read packet failed";
        goto TSPARSE_INITPARSER_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Successfully initialized parser";

TSPARSE_INITPARSER_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_InitParser : %s\n", msg);
    return ret;
}
BOOL TSParse_GetAPacket (TSDemuxer* dmx, UI8** pack, int* len)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    URLProtocol* pro = dmx->m_Pro;
    UI8* data = *pack;
    I8   flag = SUCCESS;

    if ((*pack == NULL) && (NULL == (data = *pack = (UI8*)malloc(TS_PACKET_SIZE_188))))
    {
        msg = "Allocate a TS packet failed";
        goto TSPARSE_GETAPACKET_RET;
    }

    msg = "Calling url_read failed";
    flag = pro->url_read(pro, data, 1);
    if (1 != flag)
    {
        if (flag == 0)
        {
            *len = 0;
            msg = "Stream end";
        }
        goto TSPARSE_GETAPACKET_RET;
    }
    ++dmx->m_Position;
    /// Sync with 188 bytes
    if (TS_PACKET_SYN_BYTE != data[0])
    {
        flag = pro->url_read(pro, data, 3);
        if (3  != flag)
        {
            if (flag == 0)
            {
                *len = 0;
                msg = "Stream end";
            }
            goto TSPARSE_GETAPACKET_RET;
        }
        flag = pro->url_read(pro, data, 1);
        if (1  != flag)
        {
            if (flag == 0)
            {
                *len = 0;
                msg = "Stream end";
            }
            goto TSPARSE_GETAPACKET_RET;
        }
        dmx->m_Position += 4;
        /// Sync with 192 bytes
        if (TS_PACKET_SYN_BYTE != data[0])
        {
            flag = pro->url_read(pro, data, 11);
            if (11 != flag)
            {
                if (flag == 0)
                {
                    *len = 0;
                    msg = "Stream end";
                }
                goto TSPARSE_GETAPACKET_RET;
            }
            flag = pro->url_read(pro, data,  1);
            if (1  != flag)
            {
                if (flag == 0)
                {
                    *len = 0;
                    msg = "Stream end";
                }
                goto TSPARSE_GETAPACKET_RET;
            }
            dmx->m_Position += 12;
            /// Sync with 204 bytes
            if (TS_PACKET_SYN_BYTE != data[0])
            {
                flag = pro->url_read(pro, data, 3);
                if (3  != flag)
                {
                    if (flag == 0)
                    {
                        *len = 0;
                        msg = "Stream end";
                    }
                    goto TSPARSE_GETAPACKET_RET;
                }
                flag = pro->url_read(pro, data, 1);
                if (1  != flag)
                {
                    if (flag == 0)
                    {
                        *len = 0;
                        msg = "Stream end";
                    }
                    goto TSPARSE_GETAPACKET_RET;
                }
                dmx->m_Position += 4;
                /// Sync with 208 bytes
                if (TS_PACKET_SYN_BYTE != data[0])
                {
                    msg = "Cannot find sync byte";
                    goto TSPARSE_GETAPACKET_RET;
                }
            }
        }
    }
    if (FAIL == pro->url_read(pro, data + 1, TS_PACKET_SIZE_188 - 1))
    {
        goto TSPARSE_GETAPACKET_RET;
    }
    dmx->m_Position += (TS_PACKET_SIZE_188 - 1);
    *len = TS_PACKET_SIZE_188;

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Get a TS packet OK";

TSPARSE_GETAPACKET_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_GetAPacket : %s\n", msg);
    if (ret != SUCCESS && *pack != NULL)
    {
        free (*pack);
        *pack = NULL;
    }
    return ret;
}
BOOL TSParse_AddPrePack (TSDemuxer* dmx, UI8** pack, UI64 pos)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSPacket* node;
    TSPacket* temp = dmx->m_PreListHeader;

    while (temp->m_Next != NULL)
    {
        if (temp->m_Next->m_Data == *pack)
        {
            ret = SUCCESS;
            lev = MSGL_V;
            msg = "The packet is already in list";
            goto TSPARSE_ADDPREPACK_RET;
        }
        temp = temp->m_Next;
    }

    node = (TSPacket*)malloc(sizeof(TSPacket));
    if (node == NULL)
    {
        free (*pack);
        *pack = NULL;
        msg = "Allocate new TSPacket node failed";
        goto TSPARSE_ADDPREPACK_RET;
    }
    node->m_Data     = *pack; *pack = NULL;
    node->m_Position = pos;
    node->m_Next     = NULL;
    temp->m_Next     = node;

    ++dmx->m_PreListLen;
    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Add a pre-read packet OK";

TSPARSE_ADDPREPACK_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_AddPrePack : %s\n", msg);
    return ret;
}
BOOL TSParse_DelPrePack (TSDemuxer* dmx, UI8** pack, UI64 pos)
{
    I8 ret = SUCCESS;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSPacket* prev = dmx->m_PreListHeader;
    TSPacket* node = prev->m_Next;

    while (node != NULL && (node->m_Data != *pack || node->m_Position != pos))
    {
        node = node->m_Next;
        prev = prev->m_Next;
    }

    if (node != NULL)
    {
        free (*pack);
        *pack = NULL;
        prev->m_Next = node->m_Next;
        free (node);
        node = NULL;

        --dmx->m_PreListLen;
        ret = SUCCESS;
        lev = MSGL_V;
        msg = "Pre-read node is deleted";
    }
    else
    {
        ret = FAIL;
        lev = MSGL_ERR;
        msg = "No this packet in list";
    }

    ts_demux_log(0, lev, "DEMUX ################ TSParse_DelPrePack : %s\n", msg);
    return ret;
}
BOOL TSParse_ClrPrePack (TSDemuxer* dmx)
{
    TSPacket* prev = dmx->m_PreListHeader;
    TSPacket* node = dmx->m_PreListHeader->m_Next;

    while (node != NULL)
    {
        if (node->m_Data != NULL)
        {
            free (node->m_Data);
            node->m_Data = NULL;
        }
        prev->m_Next = node->m_Next;
        free (node);
        node = prev->m_Next;
    }

    dmx->m_PreListLen = 0;
    return SUCCESS;
}
BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    UI8  parse_lev    = 0x00;   ///< Parse level
    UI16 parse_pid    = 0x00;   ///< Request PID apart between audio section and video section
    UI8* temp_pack    = NULL;   ///< Temporary packet reference
    UI64 copy_lens    = 0x00;   ///< Copied section payload data length
    UI16 section_len  = 0x00;   ///< Section length
    BOOL stream_valid = FALSE;  ///< Indicate if PES payload is valid A/V data

    TSPacket* prev  = dmx->m_PreListHeader;
    TSPacket* node  = prev->m_Next;

    /// Step-1 : Appoint current parsing level
    if (dmx->m_PMTPID == 0x00)
    {
        parse_lev = PARSE_LEV_PAT;
        parse_pid = 0x00;
    }
    else if (dmx->m_AudioPID == 0x00 || dmx->m_VideoPID == 0x00)
    {
        parse_lev = PARSE_LEV_PMT;
        parse_pid = dmx->m_PMTPID;
    }
    else
    {
        parse_lev = PARSE_LEV_PES;
    }

    ts_demux_log(0, MSGL_V, "DEMUX ################ TSParse_GetSection : Start\n");

    do{
        int  len                = 0;
        UI16 pack_pid           = 0x00;     ///< PID of current TS packet
        UI16 pack_head_size     = 0x00;     ///< Offset of payload data in TS packet
        UI8* parse_pkt          = NULL;     ///< packet we wanna parse
        BOOL pes_flag           = FALSE;    ///< Indicate present PES packet header
        UI8* payload_pos        = NULL;     ///< PES payload position
        UI16 payload_len        = 0x00;     ///< PES payload length
        UI16 payload_head_size  = 0x00;     ///< PES payload packet head size
        enum{
            PARSE, KEEP, DROP, PES_END
        }pack_handle;

        if (temp_pack == NULL && NULL == (temp_pack = (UI8*)malloc(TS_PACKET_SIZE_188)))
        {
            msg = "Allocate temporary TS packet failed";
            goto TSPARSE_GETSECTION_RET;
        }


        /// Step-2 : Get the packet we are going to parse
        if (node != NULL)
        {
            parse_pkt = node->m_Data;
        }
        else
        {
            if (FAIL == TSParse_GetAPacket(dmx, &temp_pack, &len) || len == 0)
            {
                if (len == 0)
                {
                    msg = "Stream End";
                    dmx->m_Section->m_DataLen = 0ULL;
                    break;
                }
                else
                {
                    msg = "Get a TS packet failed";
                }
                goto TSPARSE_GETSECTION_RET;
            }
            parse_pkt = temp_pack;
        }

        /// Step-3 Filter TS packet
        if (FAIL == TSParse_TSPacketHeader(parse_pkt, &pack_pid, &pack_head_size, &pes_flag))
        {
            msg = "Parse TS packet header failed";
            goto TSPARSE_GETSECTION_RET;
        }
        if (parse_lev == PARSE_LEV_PAT)
        {
            if (pack_pid == 0x00)
            {
                pack_handle = PARSE;
            }
            else
            {
                pack_handle = KEEP;
            }
        }
        else if (parse_lev == PARSE_LEV_PMT)
        {
            if (pack_pid == dmx->m_PMTPID)
            {
                pack_handle = PARSE;
            }
            else
            {
                if (pack_pid == 0x00)
                {
                    pack_handle = DROP;
                }
                else
                {
                    pack_handle = KEEP;
                }
            }
        }
        else if (pack_pid == dmx->m_AudioPID || pack_pid == dmx->m_VideoPID)
        {
            if (parse_pid == 0x00)
            {
                if (pes_flag != TRUE)
                {
                    pack_handle = DROP;
                }
                else
                {
                    parse_pid = pack_pid;
                    pack_handle = PARSE;
                }
            }
            else if (pack_pid == parse_pid)
            {
                if (pes_flag != TRUE)
                {
                    pack_handle = PARSE;
                }
                else
                {
                    pack_handle = PES_END;
                }
            }
            else
            {
                pack_handle = KEEP;
            }
        }
        else
        {
            pack_handle = DROP;
        }

        switch (pack_handle)
        {
        case PARSE  :
            {
                if (parse_pkt != temp_pack)
                {
                    memcpy(temp_pack, parse_pkt, TS_PACKET_SIZE_188);
                    if (TSParse_DelPrePack(dmx, &parse_pkt, node->m_Position))
                    {
                        msg = "Delete a pre-read packet failed";
                        goto TSPARSE_GETSECTION_RET;
                    }
                    parse_pkt = temp_pack;
                    node = prev->m_Next;
                }
                break;
            }
        case DROP   :
            {
                if (parse_pkt != temp_pack)
                {
                    if (TSParse_DelPrePack(dmx, &parse_pkt, node->m_Position))
                    {
                        msg = "Delete a pre-read packet failed";
                        goto TSPARSE_GETSECTION_RET;
                    }
                    node = prev->m_Next;
                }
                continue;
            }
        case KEEP   :
            {
                if (parse_pkt == temp_pack)
                {
                    if (TSParse_AddPrePack(dmx, &parse_pkt, dmx->m_Position - TS_PACKET_SIZE_188))
                    {
                        msg = "Delete a pre-read packet failed";
                        goto TSPARSE_GETSECTION_RET;
                    }
                    temp_pack = NULL;
                }
                else
                {
                    node = node->m_Next;
                    prev = prev->m_Next;
                }
                continue;
            }
        case PES_END:
            {
                if (parse_pkt == temp_pack)
                {
                    if (TSParse_AddPrePack(dmx, &parse_pkt, dmx->m_Position - TS_PACKET_SIZE_188))
                    {
                        msg = "Delete a pre-read packet failed";
                        goto TSPARSE_GETSECTION_RET;
                    }
                    temp_pack = NULL;
                }
                if (section_len == 0)
                {
                    dmx->m_Section->m_DataLen = copy_lens;
                }
                ret = SUCCESS;
                lev = MSGL_V;
                msg = "Get a section OK";
                goto TSPARSE_GETSECTION_RET;
                break;
            }
        }

        /// Step-4 : Parse payload packet header
        if (pes_flag == TRUE && pack_head_size != TS_PACKET_SIZE_188)
        {
            payload_pos = parse_pkt + pack_head_size;
            payload_len = TS_PACKET_SIZE_188 - pack_head_size;
            payload_head_size = 0x00U;

            if (parse_lev == PARSE_LEV_PAT || parse_lev == PARSE_LEV_PMT)
            {
                if (FAIL == TSParse_ParsePSIHeader (payload_pos, payload_len, &section_len))
                {
                    msg = "Parse PSI packet header failed";
                    goto TSPARSE_GETSECTION_RET;
                }
                payload_head_size = PSI_PACKET_HEADER_LEN;
            }
            else
            {
                if (FAIL == TSParse_ParsePESHeader (payload_pos, payload_len, &section_len\
                    , &stream_valid))
                {
                    msg = "Parse PES packet header failed";
                    goto TSPARSE_GETSECTION_RET;
                }
                payload_head_size = PES_PACKET_HEADER_LEN;
            }
            if (section_len != 0)
            {
                if (section_len > dmx->m_Section->m_BuffLen)
                {
                    if (dmx->m_Section->m_Data != NULL)
                    {
                        free (dmx->m_Section->m_Data);
                    }
                    dmx->m_Section->m_Data = (UI8*)malloc(section_len);
                    if (dmx->m_Section->m_Data == NULL)
                    {
                        msg = "Allocate the section data space failed";
                        goto TSPARSE_GETSECTION_RET;
                    }
                    dmx->m_Section->m_BuffLen = section_len;
                }
                dmx->m_Section->m_DataLen = section_len;
            }
            dmx->m_Section->m_Type    = pack_pid;
            dmx->m_Section->m_Valid   = stream_valid;
            dmx->m_Section->m_Positon =\
                (node == NULL) ? dmx->m_Position - TS_PACKET_SIZE_188 : node->m_Position;
        }

        /// Step-5 : Copy payload data from TS packet to section space
        payload_pos = parse_pkt + pack_head_size + payload_head_size;
        payload_len = TS_PACKET_SIZE_188 - pack_head_size - payload_head_size;
        if (section_len != 0 && payload_len + copy_lens > section_len)
        {
            payload_len = (UI16)(section_len - copy_lens);
        }
        if (dmx->m_Section->m_BuffLen < copy_lens + payload_len)
        {
            dmx->m_Section->m_Data\
                = (UI8*)realloc (dmx->m_Section->m_Data, (UI32)((copy_lens + payload_len) << 1));
            if (dmx->m_Section->m_Data == NULL)
            {
                msg = "Allocate the section data space failed";
                goto TSPARSE_GETSECTION_RET;
            }
            dmx->m_Section->m_BuffLen = (copy_lens + payload_len) << 1;
        }
        memcpy (dmx->m_Section->m_Data + copy_lens, payload_pos, payload_len);
        copy_lens += payload_len;
        if (section_len != 0)
        {
            if (section_len <= copy_lens)
            {
                /// PES packet end
                break;
            }
        }
    }while (1);

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Get a section OK";

TSPARSE_GETSECTION_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_GetSection : %s\n", msg);
    if (ret != SUCCESS && dmx->m_Section->m_Data != NULL)
    {
        free (dmx->m_Section->m_Data);
        dmx->m_Section->m_Data = NULL;
    }
    if (temp_pack != NULL)
    {
        free (temp_pack);
        temp_pack = NULL;
    }
    return ret;
}
BOOL TSParse_PATSection (TSDemuxer* dmx)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    int  rows = 0;
    UI16 program_number;
    UI16 pid;
    BitBuffer* buf = NULL;

    UI8* data = (UI8*)dmx->m_Section->m_Data;
    UI32 lens = (UI32)dmx->m_Section->m_DataLen;

    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_PATSECTION_RET;
    }

    /// Skipped fields
    /// transport_stream_id	        16
    /// reserved                    2
    /// version_number              5
    /// current_next_indicator      1
    /// section_number              8
    /// last_section_number         8
    if (FAIL == SkipSeverlBits (buf, 40))
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PATSECTION_RET;
    }

    rows = (lens - buf->m_BytePos - 4) / 4;
    msg = "Parse bit buffer failed";
    while (rows > 0)
    {
        if ((FAIL == GetDataFromBitBuffer (buf, 16, &program_number))   ///< Program number       16
         || (FAIL == SkipSeverlBits (buf,  3)))                         ///< reserved              3
        {
            goto TSPARSE_PATSECTION_RET;
        }
        if (FAIL == GetDataFromBitBuffer (buf, 13, &pid))               ///< PID                  13
        {
            goto TSPARSE_PATSECTION_RET;
        }
        if (program_number != 0)
        {
            break;
        }
        --rows;
    }
    if (rows == 0)
    {
        msg = "Cannot find PMT PID";
        goto TSPARSE_PATSECTION_RET;
    }
    dmx->m_PMTPID = pid;

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse PAT section OK";

TSPARSE_PATSECTION_RET:
    CloseBitBuffer(&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_PATSection : %s\n", msg);
    return ret;
}
BOOL TSParse_PMTSection (TSDemuxer* dmx, Metadata* meta)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    UI8  stream_type;                               ///< stream type for each table
    UI16 av_pid;                                    ///< PID for each table
    UI16 program_info_len;                          ///< program info length
    UI16 es_info_len;                               ///< ES info length for each table
    const char* audiocodec = NULL;
    const char* videocodec = NULL;

    BitBuffer* buf = NULL;

    UI8* data = (UI8*)dmx->m_Section->m_Data;       ///< PMT packet data
    UI32 lens = (UI32)dmx->m_Section->m_DataLen;    ///< PMT packet data length

    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_PMTSECTION_RET;
    }

    /// Skipped fields
    /// program_number              16
    /// reserved                    2
    /// version_number              5
    /// current_next_indicator      1
    /// section_number              8
    /// last_section_number         8
    /// reserved                    3
    /// PCR_PID                     13
    /// reserved                    4
    if ((FAIL == SkipSeverlBits (buf, 60))
     || (FAIL == GetDataFromBitBuffer (buf, 12, &program_info_len)))    ///< program info length  12
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PMTSECTION_RET;
    }
    if ((FAIL == SkipSeverlBits (buf, 8 * program_info_len)))           ///< descriptor            ?
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PMTSECTION_RET;
    }
    while ((lens - buf->m_BytePos - 4) > 0)
    {
        if ((FAIL == GetDataFromBitBuffer (buf,  8, &stream_type))      ///< stream type           8
         || (FAIL == SkipSeverlBits (buf, 3))                           ///< reserved              3
         || (FAIL == GetDataFromBitBuffer (buf, 13, &av_pid))           ///< PID                  13
         || (FAIL == SkipSeverlBits (buf, 4))                           ///< reserved              4
         || (FAIL == GetDataFromBitBuffer (buf, 12, &es_info_len))      ///< ES info length       12
         || (FAIL == SkipSeverlBits (buf, 8 * es_info_len)))            ///< descriptor            ?
        {
            msg = "Parse bit buffer failed";
            goto TSPARSE_PMTSECTION_RET;
        }
        if (dmx->m_VideoPID != 0 && dmx->m_AudioPID != 0)
        {
            break;
        }
        switch (stream_type)
        {
        case 0x01 : ///< MPEG Video Version 1
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_MPEG1VIDEO;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "MPEG-1 Video";
            }
            break;
        case 0x02 : ///< MPEG Video Version 2
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_MPEG2VIDEO;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "MPEG-2 Video";
            }
            break;
        case 0x03 : ///< MPEG Audio Version 1
            if (dmx->m_AudioPID == 0x00)
            {
                meta->audiocodec      = CODEC_ID_MP1;
                meta->subaudiocodec   = -1;
                meta->audiostreamindex= av_pid;
                dmx->m_AudioPID       = av_pid;
                audiocodec = "MPEG-1 Audio";
            }
            break;
        case 0x04 : ///< MPEG Audio Version 2
            if (dmx->m_AudioPID == 0x00)
            {
                meta->audiocodec      = CODEC_ID_MP2;
                meta->subaudiocodec   = -1;
                meta->audiostreamindex= av_pid;
                dmx->m_AudioPID       = av_pid;
                audiocodec = "MPEG-2 Audio";
            }
            break;
        case 0x0F : ///< AAC
            if (dmx->m_AudioPID == 0x00)
            {
                meta->audiocodec      = CODEC_ID_AAC;
                meta->subaudiocodec   = AAC_SUBTYPE_ADTS;
                meta->audiostreamindex= av_pid;
                dmx->m_AudioPID       = av_pid;
                audiocodec = "AAC";
            }
            break;
        case 0x10 : ///< MPEG-4 Visual
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_MPEG4;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "MPEG-4 Visual";
            }
            break;
        case 0x11 : ///< AAC
            if (dmx->m_AudioPID == 0x00)
            {
                meta->audiocodec      = CODEC_ID_AAC_LATM;
                meta->subaudiocodec   = AAC_SUBTYPE_LATM;
                meta->audiostreamindex= av_pid;
                dmx->m_AudioPID       = av_pid;
                audiocodec = "AAC";
            }
            break;
        case 0x1B : ///< AVC
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_H264;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "AVC";
            }
            break;
        case 0x1C : ///< AAC
            if (dmx->m_AudioPID == 0x00)
            {
                meta->audiocodec      = CODEC_ID_AAC;
                meta->subaudiocodec   = AAC_SUBTYPE_ADIF;   ///< @todo not sure
                meta->audiostreamindex= av_pid;
                dmx->m_AudioPID       = av_pid;
                audiocodec = "AAC";
            }
            break;
        case 0x1E : ///< MPEG Video ISO/IEC 23002-3
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_MPEG4;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "MPEG Video ISO/IEC 23002-3";
            }
            break;
        case 0x1F : ///< AVC
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_H264;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "AVC";
            }
            break;
        case 0x20 : ///< AVC
            if (dmx->m_VideoPID == 0x00)
            {
                meta->videocodec      = CODEC_ID_H264;
                meta->subvideocodec   = -1;
                meta->videostreamindex= av_pid;
                dmx->m_VideoPID       = av_pid;
                videocodec = "AVC";
            }
            break;
        default:
            break;
        }
    }
    if (dmx->m_AudioPID == 0x00 || dmx->m_VideoPID == 0x00)
    {
        msg = "Cannot find audio or video PID";
        goto TSPARSE_PMTSECTION_RET;
    }
    if (meta->videocodec != CODEC_ID_H264)
    {
        dmx->m_SupportSeek = FALSE;
    }
    else
    {
        dmx->m_SupportSeek = TRUE;
    }
    meta->fileformat = FILEFORMAT_ID_TS;

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse PMT section OK";

TSPARSE_PMTSECTION_RET:
    CloseBitBuffer(&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_PMTSection : %s\n", msg);
#if 1
    if (ret == SUCCESS)
    {
        ts_demux_log(0, MSGL_INFO, "DEMUX ################ VideoCodec = %s AudioCodec = %s\n",
            videocodec, audiocodec);
    }
#endif
    return ret;
}
BOOL TSParse_PESSection (TSDemuxer* dmx, AVPacket* pack)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    BitBuffer* buf = NULL;
    UI8        pts;
    UI8        dts;
    UI8        pes_header_len;
    UI64       av_data_len;

    UI8* data = (UI8*)dmx->m_Section->m_Data;
    UI32 lens = (UI32)dmx->m_Section->m_DataLen;

    if (FAIL == InitiBitBuffer(&buf, data, lens))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_PESSECTION_RET;
    }

    /// Fields
    /// '10'                        2
    /// PES_scrambling_control      2
    /// PES_priority                1
    /// data_alignment_indicator    1
    /// copyright                   1
    /// original_or_copy            1
    /// PTS_flag                    1
    /// DTS_flag                    1
    /// ESCR_flag                   1
    /// ES_rate_flag                1
    /// DSM_trick_mode_flag         1
    /// additional_copy_info_flag   1
    /// PES_CRC_flag                1
    /// PES_extension_flag1         1
    /// PES_header_data_length      8
    if ((FAIL == SkipSeverlBits (buf, 8))
     || (FAIL == GetDataFromBitBuffer (buf, 1, &pts))                   ///< PTS flag
     || (FAIL == GetDataFromBitBuffer (buf, 1, &dts))                   ///< DTS flag
     || (FAIL == SkipSeverlBits (buf, 6))
     || (FAIL == GetDataFromBitBuffer (buf, 8, &pes_header_len)))       ///< PES header data length
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PESSECTION_RET;
    }
    pack->pts = pack->dts = -1LL;
    if (pts == 0x01)
    {
        UI8  pts_1;
        UI16 pts_2, pts_3;
        I64  pts = 0LL;
        if ((FAIL == SkipSeverlBits (buf, 4))
         || (FAIL == GetDataFromBitBuffer (buf,  3, &pts_1))
         || (FAIL == SkipSeverlBits (buf, 1))
         || (FAIL == GetDataFromBitBuffer (buf, 15, &pts_2))
         || (FAIL == SkipSeverlBits (buf, 1))
         || (FAIL == GetDataFromBitBuffer (buf, 15, &pts_3))
         || (FAIL == SkipSeverlBits (buf, 1)))
        {
            msg = "Parse bit buffer failed";
            goto TSPARSE_PESSECTION_RET;
        }
        pts = ((I64)pts_1 << 30) | ((I64)pts_2 << 15) | ((I64)pts_3);
        pack->pts = pts;
    }
    if (dts == 0x01)
    {
        UI8  dts_1;
        UI16 dts_2, dts_3;
        I64  dts = 0LL;
        if ((FAIL == SkipSeverlBits (buf, 4))
         || (FAIL == GetDataFromBitBuffer (buf,  3, &dts_1))
         || (FAIL == SkipSeverlBits (buf, 1))
         || (FAIL == GetDataFromBitBuffer (buf, 15, &dts_2))
         || (FAIL == SkipSeverlBits (buf, 1))
         || (FAIL == GetDataFromBitBuffer (buf, 15, &dts_3))
         || (FAIL == SkipSeverlBits (buf, 1)))
        {
            msg = "Parse bit buffer failed";
            goto TSPARSE_PESSECTION_RET;
        }
        dts = ((I64)dts_1 << 30) | ((I64)dts_2 << 15) | ((I64)dts_3);
        pack->dts = dts;
    }
    if (FAIL == SkipSeverlBits (buf, pes_header_len * 8 - (pts + dts) * 40))
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PESSECTION_RET;
    }
    av_data_len = dmx->m_Section->m_DataLen - buf->m_BytePos;
    if (pack->bufferlength < (int)av_data_len)
    {
        if (pack->data != NULL)
        {
            free (pack->data);
            pack->data = NULL;
        }
        pack->data = (UI8*)malloc((UI32)av_data_len);
        if (pack->data == NULL)
        {
            msg = "Allocate AV packet data failed";
            goto TSPARSE_PESSECTION_RET;
        }
        pack->bufferlength = (int)av_data_len;
    }
    memcpy(pack->data, dmx->m_Section->m_Data + buf->m_BytePos, (UI32)av_data_len);
    pack->size = (int)av_data_len;
    pack->stream_index = dmx->m_Section->m_Type;

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse PES section OK";

TSPARSE_PESSECTION_RET:
    CloseBitBuffer(&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_PESSection : %s\n", msg);
    if (ret != SUCCESS && pack->data != NULL)
    {
        free (pack->data);
        pack->data = NULL;
    }
    return ret;
}
BOOL TSParse_TSPacketHeader (const UI8* data, UI16* pkt_PID, UI16* ofs, BOOL* pes)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    UI8 sync_byte    = 0U;  ///< Sync byte
    UI8 adap_flag    = 0U;  ///< Indicate if this packet contains adapation field
    UI8 adap_lens    = 0U;  ///< Indicate adapation field length
    UI8 pes_start    = 0U;  ///< Indicate if this packet contains PES header
    UI8 contains_pld = 0U;  ///< Indicate if this packet contains valid payload data

    BitBuffer* buf = NULL;

    if (FAIL == InitiBitBuffer (&buf, data, TS_PACKET_SIZE_188))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_TSPACKETHEADER_RET;
    }

    msg = "Parse bit buffer failed";
    if (FAIL == GetDataFromBitBuffer (buf, 8, &sync_byte))          ///< Sync byte                8
    {
        goto TSPARSE_TSPACKETHEADER_RET;
    }
    if (TS_PACKET_SYN_BYTE != sync_byte)
    {
        msg = "Sync byte error";
        goto TSPARSE_TSPACKETHEADER_RET;
    }

    if ((FAIL == SkipSeverlBits (buf, 1))                           ///< Transport error flag     1
     || (FAIL == GetDataFromBitBuffer (buf, 1, &pes_start))         ///< Payload data start flag  1
     || (FAIL == SkipSeverlBits (buf, 1))                           ///< Transport priority       1
     || (FAIL == GetDataFromBitBuffer (buf,13,  pkt_PID))           ///< PID of packet            13
     || (FAIL == SkipSeverlBits (buf, 2))                           ///< Scrambling flag          2
     || (FAIL == GetDataFromBitBuffer (buf, 1, &adap_flag))         ///< Adaptation field flag    1
     || (FAIL == GetDataFromBitBuffer (buf, 1, &contains_pld))      ///< Valid payload flag       1
     || (FAIL == SkipSeverlBits (buf, 4)))                          ///< Continuity counter       4
    {
        goto TSPARSE_TSPACKETHEADER_RET;
    }

    if (1 == adap_flag)
    {
        if (FAIL == GetDataFromBitBuffer (buf, 8, &adap_lens))      ///< Adaptation field length
        {
            goto TSPARSE_TSPACKETHEADER_RET;
        }
    }

    /// Skip adaptation field
    if (contains_pld)
    {
        *ofs = 4 + adap_flag + adap_lens;
    }
    else
    {
        *ofs = TS_PACKET_SIZE_188;
    }
    *pes = pes_start == 0x00 ? FALSE : TRUE;

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse packet header OK";

TSPARSE_TSPACKETHEADER_RET:
    CloseBitBuffer (&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_TSPacketHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_ParsePESHeader (const UI8* data, UI16  datalen, UI16* len, BOOL* val)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    UI32 prefix = 0U;
    UI8  stream = 0U;
    BitBuffer* buf = NULL;

    if (InitiBitBuffer (&buf, data, datalen))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    if ((FAIL == GetDataFromBitBuffer (buf, 24, &prefix))           ///< PES start code prefix
     || (FAIL == GetDataFromBitBuffer (buf,  8, &stream))           ///< Stream ID
     || (FAIL == GetDataFromBitBuffer (buf, 16, len)))              ///< Section length
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PARSEPSIHEADER_RET;
    }
    if (prefix != 0x000001)
    {
        msg =  "Wrong Prefix field";
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    if ((stream != STREAM_ID_PROGRAM_MAP)
     || (stream != STREAM_ID_PADDING)
     || (stream != STREAM_ID_PRIVATE_2)
     || (stream != STREAM_ID_ECM)
     || (stream != STREAM_ID_EMM)
     || (stream != STREAM_ID_PRO_DIREC))
    {
        *val = TRUE;
    }
    else
    {
        *val = FALSE;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse PES packet header OK";

TSPARSE_PARSEPSIHEADER_RET:
    CloseBitBuffer(&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_ParsePESHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_ParsePSIHeader (const UI8* data, UI16  datalen, UI16* len)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    BitBuffer* buf = NULL;

    if (InitiBitBuffer (&buf, data, datalen))
    {
        msg = "Initialize bit buffer failed";
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    /// Skipped field
    /// table_id                 8
    /// section_syntax_indicator 1
    /// '0'                      1
    /// reserved                 2
    if ((FAIL == SkipSeverlBits (buf, 20))
     || (FAIL == GetDataFromBitBuffer (buf, 12, len)))              ///< section length           12
    {
        msg = "Parse bit buffer failed";
        goto TSPARSE_PARSEPSIHEADER_RET;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Parse PSI packet header OK";

TSPARSE_PARSEPSIHEADER_RET:
    CloseBitBuffer (&buf);
    ts_demux_log(0, lev, "DEMUX ################ TSParse_ParsePSIHeader : %s\n", msg);
    return ret;
}
BOOL TSParse_CheckPESKeyFrame_H246 (const UI8* data, UI32  datalen)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg = NULL;

    UI32 i ;
    for (i = 0; i + 3 + 9 <= datalen; ++i)
    {
        if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1)
        {
            UI8  unit_type;
            UI32 slice_type;
            i += 3;
            unit_type = data[i] & 0x1F;
            if (unit_type == 5 || unit_type == 1)
            {
                BitContext buf;
                init_bits(&buf, data + i + 1, (datalen - i - 1) * 8);
                Skip_ue_golomb(&buf);
                slice_type = Get_ue_golomb_31 (&buf);

                if (slice_type > 4)
                {
                    slice_type -= 5;
                }
                if (slice_type == 2)
                {
                    ret = SUCCESS;
                    lev = MSGL_V;
                    msg = "Find Key Frame";
                    goto TSPARSE_CHECKPESKEYFRAME_H264_RET;
                }
                else
                {
                    continue;
                }
                break;
            }
        }
    }

    lev = MSGL_V;
    msg = "Not Key Frame";

TSPARSE_CHECKPESKEYFRAME_H264_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_CheckPESKFrame : %s\n", msg);
    return ret;
}
BOOL TSParse_GetTSFDuration (TSDemuxer* dmx)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    AVPacket pkt;
    I64 start_tms;
    I64 end_tms;

    memset(&pkt, 0, sizeof(pkt));

    while (1)
    {
        if (FAIL == TSParse_GetSection (dmx))
        {
            msg = "Get a section failed";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (dmx->m_Section->m_DataLen == 0)
        {
            msg = "Stream End";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (FAIL == TSParse_PESSection (dmx, &pkt))
        {
            msg = "Parse PES section failed";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (pkt.pts == -1)
        {
            continue;
        }
        start_tms = pkt.pts;
        break;
    }


    dmx->m_Position = dmx->m_FileSize - (TS_PACKET_SIZE_188 << 10);
    if (FAIL == dmx->m_Pro->url_seek(dmx->m_Pro, dmx->m_Position, SEEK_SET))
    {
        msg = "Calling url_seek Failed";
        goto TSPARSE_GETTSFDURATION_RET;
    }
    TSParse_ClrPrePack(dmx);
    if (FAIL == TSParse_InitParser(dmx))
    {
        msg = "Initialize parsing failed";
        goto TSPARSE_GETTSFDURATION_RET;
    }
    while (1)
    {
        if (FAIL == TSParse_GetSection (dmx))
        {
            msg = "Get a section failed";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (dmx->m_Section->m_DataLen == 0)
        {
            msg = "Stream End";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (FAIL == TSParse_PESSection(dmx, &pkt))
        {
            msg = "Parse PES section failed";
            goto TSPARSE_GETTSFDURATION_RET;
        }
        if (pkt.pts == -1)
        {
            continue;
        }
        end_tms = pkt.pts;
        break;
    }

    if (start_tms == -1 || end_tms == -1)
    {
        msg = "Get duration failed";
        goto TSPARSE_GETTSFDURATION_RET;
    }
    
    dmx->m_Position = 0ULL;
    if (FAIL == dmx->m_Pro->url_seek(dmx->m_Pro, dmx->m_Position, SEEK_SET))
    {
        msg = "Calling url_seek Failed";
        goto TSPARSE_GETTSFDURATION_RET;
    }
    if (FAIL == TSParse_InitParser(dmx))
    {
        msg = "Initialize parsing failed";
        goto TSPARSE_GETTSFDURATION_RET;
    }

    dmx->m_Duration = (end_tms - start_tms) / 90;
    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Get duration OK";

TSPARSE_GETTSFDURATION_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSParse_GetTSFDuration : %s\n");
    return ret;
}
