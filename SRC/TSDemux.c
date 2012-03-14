#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "TSParse.h"
#include "TSDemux.h"
#include "demux_log.h"
#include "../mp_msg.h"

#ifdef _WRITE_RAW_DATA_TO_FILE_
FILE* fp_audio;
FILE* fp_video;
void InitiAVFile ()
{
    fp_audio = fopen ("D:/Private/WorkArea/ts-sample/demo_live1/AUDIO", "wb+");
    fp_video = fopen ("D:/Private/WorkArea/ts-sample/demo_live1/VIDEO", "wb+");

    if (fp_video == NULL || fp_audio == NULL)
    {
        puts("Create File Failed\n");
        fgetc(stdin);
        exit(-1);
    }
}

void WriteAVFile (const TSDemuxer* dmx, void* data, int length)
{
    if (dmx->m_AudioPID == dmx->m_Section->m_Type)
    {
        if (length != fwrite(data, 1, length, fp_audio))
        {
            fputs("Write File Failed", stdout);
            fgetc(stdin);
            exit(1);
        }
    }
    else
    {
        if (length != fwrite(data, 1, length, fp_video))
        {
            fputs("Write File Failed", stdout);
            fgetc(stdin);
            exit(1);
        }
    }
}
#endif

#ifndef _TS_DEMUX_TEST_
/// @brief Demux Helper Initialize
static int           demux_helper_initialise();
/// @brief Demux Helper Destroy
static int           demux_helper_deinitialise();
/// @brief Demux Helper Set Exit Flag
static void          demux_helper_set_exit_flag();
/// @brief  Check if file of this type can handle
static int           can_handle(int fileformat);
/// @brief Create Demux Context
static DemuxContext* create_demux_context();
/// @brief Destroy Demux Context
static void          destroy_demux_context(DemuxContext * ctx);

DemuxContextHelper   ts_demux_helper = {
    "ts",
    demux_helper_initialise,
    demux_helper_deinitialise,
    demux_helper_set_exit_flag,
    can_handle,
    create_demux_context,
    destroy_demux_context,
    0,
    0
};
static DemuxContext* create_demux_context()
{
    DemuxContext * ctx = calloc(1, sizeof(DemuxContext));
    ctx->demux_open = TSDemux_Open;
    ctx->demux_probe = NULL;
    ctx->demux_close = TSDemux_Close;
    ctx->demux_parse_metadata = TSDemux_Mdata;
    ctx->demux_read_packet = TSDemux_ReadAV;
    ctx->demux_seek = TSDemux_Seek;

    pthread_mutex_lock(&ts_demux_helper.instance_list_mutex);
    //insert the instance to the global instance list
    if(ts_demux_helper.priv_data == 0)
    {
        ts_demux_helper.priv_data = ctx;
    }
    else
    {
        ctx->next = ts_demux_helper.priv_data;
        ts_demux_helper.priv_data = ctx;
    }
    pthread_mutex_unlock(&ts_demux_helper.instance_list_mutex);

    return ctx;
}
static void destroy_demux_context(DemuxContext * ctx)
{
    pthread_mutex_lock(&ts_demux_helper.instance_list_mutex);
    //remove the instance from the global instance list
    DemuxContext * cur = ts_demux_helper.priv_data;
    DemuxContext * prev = cur;
    while(cur && cur != ctx)
    {
        prev = cur;
        cur = cur->next;
    }
    if(cur == ctx)
    {
        if(cur == ts_demux_helper.priv_data)
        {
            //head will be free
            ts_demux_helper.priv_data = cur->next;
        }
        else
        {
            prev->next = cur->next;
        }
    }

    //destroy DemuxContext only, it's private data should be cleared in ts_demux_close
    free(ctx);
    pthread_mutex_unlock(&ts_demux_helper.instance_list_mutex);
}
static int  can_handle(int fileformat)
{
    if (fileformat == FILEFORMAT_ID_TS)
    {
        ts_demux_log(0, MSGL_V, "ts_demux_can_handle : OK\n");
        return 0;
    }
    ts_demux_log_error(0, MSGL_V, "ts_demux_can_handle : This format cannot handle\n");
    return -1;
}
static int  demux_helper_initialise()
{
    return pthread_mutex_init(&ts_demux_helper.instance_list_mutex, NULL);
}
static int  demux_helper_deinitialise()
{
    return pthread_mutex_destroy(&ts_demux_helper.instance_list_mutex);
}
static void demux_helper_set_exit_flag()
{
    pthread_mutex_lock(&ts_demux_helper.instance_list_mutex);
    DemuxContext * ctx = ts_demux_helper.priv_data;
    while(ctx)
    {
        ctx->exit_flag = 1;
        ctx = ctx->next;
    }
    pthread_mutex_unlock(&ts_demux_helper.instance_list_mutex);
}
#endif

int TSDemux_Open  (DemuxContext* ctx, URLProtocol* h)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = NULL;

    if (ctx == NULL || h == NULL)
    {
        msg = "Parameter Error";
        goto TSDEMUX_OPEN_RET;
    }

    ctx->priv_data      = (void*)malloc(sizeof(TSDemuxer));
    ctx->priv_data_size = sizeof(TSDemuxer);
    if (ctx->priv_data == NULL)
    {
        msg = "Allocate private data failed";
        goto TSDEMUX_OPEN_RET;
    }

    dmx = (TSDemuxer*)ctx->priv_data;

    dmx->m_Section = (TSection*)malloc(sizeof(TSection));
    dmx->m_PreListHeader = (TSPacket*)malloc(sizeof(TSPacket));
    if (dmx->m_Section == NULL)
    {
        msg = "Allocate a section structure failed";
        goto TSDEMUX_OPEN_RET;
    }
    if (dmx->m_PreListHeader == NULL)
    {
        msg = "Allocate a Pre-read packet list header failed";
        goto TSDEMUX_OPEN_RET;
    }
    memset(dmx->m_Section,       0, sizeof(TSection));
    memset(dmx->m_PreListHeader, 0, sizeof(TSPacket));
    dmx->m_PreListLen = 0;
    dmx->m_Duration   = 0ULL;
    dmx->m_FileSize   = ((h->url_is_live(h)||!h->url_is_live))? 0 : (h->url_seek(h, 0, SEEK_SIZE));
    dmx->m_Position   = 0ULL;
    dmx->m_PMTPID     = 0U;
    dmx->m_AudioPID   = 0U;
    dmx->m_VideoPID   = 0U;
    dmx->m_Pro        = h;
    if (FAIL == TSParse_InitParser(dmx))
    {
        free (dmx->m_Section);
        dmx->m_Section = NULL;
        msg = "Initialize parser failed";
        goto TSDEMUX_OPEN_RET;
    }

    ret = SUCCESS;
    lev = MSGL_INFO;
    msg = "Open TS demux OK";

TSDEMUX_OPEN_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSDemux_Open : %s File Size = %lld\n"\
        , msg, dmx->m_FileSize);
    return ret;
}
int TSDemux_Close (DemuxContext* ctx)
{
    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    dmx->m_Section = NULL;
    if (dmx->m_Section != NULL)
    {
        if (dmx->m_Section->m_Data != NULL)
        {
            free (dmx->m_Section->m_Data);
            dmx->m_Section->m_Data = NULL;
        }
        free (dmx->m_Section);
        dmx->m_Section = NULL;
    }
    if (dmx->m_PreListHeader->m_Next != NULL)
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
    }

    ts_demux_log(0, MSGL_INFO, "DEMUX ################ TSDemux_Close : TS demux is closed OK\n");
    return SUCCESS;
}
int TSDemux_Mdata (DemuxContext* ctx, Metadata* meta)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    if (FAIL == TSParse_GetSection (dmx))
    {
        msg = "Get section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_PATSection (dmx))
    {
        msg = "Parse PAT section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_GetSection (dmx))
    {
        msg = "Get section failed";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_PMTSection (dmx, meta))
    {
        msg = "Parse PMT section failed";
        goto TSDEMUX_MDATA_RET;
    }

    /// @todo Get file duration

    ret = SUCCESS;
    lev = MSGL_INFO;
    msg = "Parse metadata OK";

TSDEMUX_MDATA_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSDemux_Mdata : %s\n", msg);
#if 1
    ts_demux_log(0, lev, "\t Audio :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->audiocodec, meta->subaudiocodec, meta->audiostreamindex);
    ts_demux_log(0, lev, "\t Video :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->videocodec, meta->subvideocodec, meta->videostreamindex);
#endif
#ifdef _WRITE_RAW_DATA_TO_FILE_
    InitiAVFile();
#endif
    return ret;
}
int TSDemux_ReadAV(DemuxContext* ctx, AVPacket* pack)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    while (1)
    {
        if (FAIL == TSParse_GetSection(dmx))
        {
            msg = "Get a section failed";
            goto TSDEMUX_READAV_RET;
        }
        if (dmx->m_Section->m_DataLen == 0ULL)
        {
            msg = "Stream End";
            ret = SUCCESS;
            lev = MSGL_INFO;
            pack->size = 0;
            goto TSDEMUX_READAV_RET;
        }
        if (dmx->m_Section->m_Valid == FALSE)
        {
            continue;
        }
        if (FAIL == TSParse_PESSection (dmx, pack))
        {
            msg = "Parse PES section failed";
            goto TSDEMUX_READAV_RET;
        }
        break;
    }

    ret = SUCCESS;
    lev = MSGL_V;
    msg = "Read a A/V section OK";

TSDEMUX_READAV_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSDemux_ReadAV : %s\n", msg);
    if (ret == SUCCESS && dmx->m_Section->m_DataLen != 0ULL)
    {
#if _OUTPUT_EACH_AV_PACKET_INFO_
        ts_demux_log(0, MSGL_INFO, "\t%s PTS = 0x%016llX DTS = 0x%016llX SIZE = %-6d POS = %lld\n"\
            , pack->stream_index == dmx->m_AudioPID ? "Audio" : "Video", pack->pts, pack->dts\
            , pack->size, dmx->m_Section->m_Positon);
#endif
#ifdef _WRITE_RAW_DATA_TO_FILE_
        WriteAVFile(dmx, pack->data, pack->size);
#endif
    }
    return pack->size;
}
int TSDemux_Seek  (DemuxContext* ctx)
{
    /// @todo
    return FAIL;
}



