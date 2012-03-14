#ifndef DEMUX_LOG_H
#define DEMUX_LOG_H
#include "../mp_msg.h"

#define flv_demux_log_error(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log_error(mod, level, fmt, ...)  mp_msg(mod, level, fmt, ##__VA_ARGS__)

#ifdef DEMUX_DEBUG
#define flv_demux_log(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#else
#define                                             \
    flv_demux_log(mod, level, fmt, ...)             \
    do                                              \
    {                                               \
        char tmp[MSGSIZE_MAX];                      \
        va_list args;                               \
        va_start (args, fmt);                       \
        vsnprintf(tmp, MSGSIZE_MAX, fmt, args);     \
        if (level >= MSGL_V)                        \
        {                                           \
            break;                                  \
        }                                           \
        if (level <= MSGL_ERR)                      \
        {                                           \
            flv_demux_log_error(mod, level, tmp);   \
            break;                                  \
        }                                           \
        fprintf (stdout, tmp);                      \
        va_end   (args);                            \
    }while(0);
#define ts_demux_log(mod, level, fmt, ...) flv_demux_log(mod, level, fmt, ##__VA_ARGS__)
#endif

#endif/*DEMUX_LOG_H*/
