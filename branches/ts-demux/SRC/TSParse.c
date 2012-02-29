#include <string.h>
#include "TSParse.h"
#include "../mp_msg.h"

BOOL TSParse_GetSection (TSDemuxer* dmx)
{
    I8 retval = FAIL;
    I8 msglev = MSGL_ERR;
    I8 msgstr[64];
    URLProtocol* pro;

    strpcy(msgstr, "Parameters error");
    if ((dmx == NULL) || ((pro = dmx->m_Pro) == NULL))
    {
        goto TSPARSE_GETSECTION_RET;
    }

    while (1)
    {

    }




TSPARSE_GETSECTION_RET:
    mp_msg(0, msglev, "DEMUX ################ TSParse_GetSection : %s\n", msgstr);
    return retval;
}


BOOL TSParse_PktHeader (TSDemuxer* dmx)
{
    I8 retval = FAIL;
    I8 msglev = MSGL_ERR;
    I8 msgstr[64];

    strcpy(msgstr, "Parameters error");
    if ()
}

