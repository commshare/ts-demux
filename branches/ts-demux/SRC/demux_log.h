#ifndef DEMUX_LOG_H
#define DEMUX_LOG_H
#include <stdio.h>
#include <stdarg.h>
#include "../mp_msg.h"

void demux_log(int mod, int level, const char* fmt, ...);

#define flv_demux_log_error(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log_error(mod, level, fmt, ...)  mp_msg(mod, level, fmt, ##__VA_ARGS__)

#ifdef  DEMUX_DEBUG
#define flv_demux_log(mod, level, fmt, ...) mp_msg(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log(mod, level, fmt, ...)  mp_msg(mod, level, fmt, ##__VA_ARGS__)
#else
#define flv_demux_log(mod, level, fmt, ...) demux_log(mod, level, fmt, ##__VA_ARGS__)
#define ts_demux_log(mod, level, fmt, ...)  demux_log(mod, level, fmt, ##__VA_ARGS__)
#endif

#endif/*DEMUX_LOG_H*/
