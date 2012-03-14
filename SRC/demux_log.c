#include "demux_log.h"
void demux_log(int mod, int level, const char* fmt, ...)
{
    char tmp[MSGSIZE_MAX];
    va_list args;
    va_start (args, fmt);
    vsnprintf(tmp, MSGSIZE_MAX, fmt, args);
    va_end   (args);
    tmp[MSGSIZE_MAX-2] = '\n';
    tmp[MSGSIZE_MAX-1] = 0;
    if (level >= MSGL_V)
    {
        return;
    }
    if (level <= MSGL_ERR)
    {
        flv_demux_log_error(mod, level, tmp);
        return;
    }
    printf ("%s", tmp);fflush(stdout);
}