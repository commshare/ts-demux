#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    I8 retval = FAIL;
    I8 msglev = MSGL_ERR;
    I8 errstr[64];
    URLProtocol* pro;

    strpcy(errstr, "Parameters error");
    if ((dmx == NULL) || ((pro = dmx->m_Pro) == NULL))
    {
        goto TSPARSE_GETSECTION_RET;
    }





TSPARSE_GETSECTION_RET:
    mp_msg(0, msglev, "DEMUX ################ TSParse_GetSection : %s\n", errstr);
    return retval;
}

