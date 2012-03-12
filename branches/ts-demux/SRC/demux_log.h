#ifndef DEMUX_LOG_H
#define DEMUX_LOG_H
#include "../mp_msg.h"

#ifdef FLV_DEMUX_DEBUG
#define flv_demux_log(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#else
#define flv_demux_log(mod, level, fmt, ...)
#define ts_demux_log(mod, level, fmt, ...)
#endif

#define flv_demux_log_error(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log_error(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)

#endif/*DEMUX_LOG_H*/
