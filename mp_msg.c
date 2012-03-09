#include "mp_msg.h"

void mp_msg(int mod, int lev, const char* format, ...)
{
    va_list args;

    if (lev >= MSGL_V)
    {
        return;
    }
    va_start (args, format);
    vfprintf (stdout, format, args);
    va_end   (args);

    mod = 0;
    lev = 0;
}
