#include "logger.h"
#include "ts_parse.h"
#include "bit_buffer.h"

int main()
{
    UI8          data[TS_PACKET_LENGTH];
    int          size   = TS_PACKET_LENGTH;
    BitBuffer*   bitbuf = NULL;
    TSHeaderInfo header;

    FILE* fp = fopen("D:\\Desktop\\TSSample\\adts\\sample.ts", "r");
    fread(data, 1, 188, fp);

    /// XXX Create Bit Buffer
    if ((bitbuf = InitiBitBuffer(data, size)) == NULL)
    {
        return 0;
    }

    TSPacketParse (bitbuf, data, &size, &header);

    return 0;
}
