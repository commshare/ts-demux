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
    0,
    PTHREAD_MUTEX_INITIALIZER
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
    dmx->m_TempAVPkt  = NULL;
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
            mp_msg(0, MSGL_INFO, "DEMUX dmx->m_Section->m_Data=%p \n",dmx->m_Section->m_Data);
            free (dmx->m_Section->m_Data);
            dmx->m_Section->m_Data = NULL;
        }
        mp_msg(0, MSGL_INFO, "DEMUX dmx->m_Section=%p \n",dmx->m_Section);
        free (dmx->m_Section);
        dmx->m_Section = NULL;
    }
    if (dmx->m_PreListHeader->m_Next != NULL)
    {
        TSParse_ClrPrePack(dmx);
    }
    if (dmx->m_TempAVPkt != NULL)
    {
        if (dmx->m_TempAVPkt->data != NULL)
        {
            free (dmx->m_TempAVPkt->data);
            dmx->m_TempAVPkt->data = NULL;
        }
        free (dmx->m_TempAVPkt);
        dmx->m_TempAVPkt = NULL;
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
    if (dmx->m_Section->m_DataLen == 0)
    {
        msg = "Stream End";
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
    if (dmx->m_Section->m_DataLen == 0)
    {
        msg = "Stream End";
        goto TSDEMUX_MDATA_RET;
    }
    if (FAIL == TSParse_PMTSection (dmx, meta))
    {
        msg = "Parse PMT section failed";
        goto TSDEMUX_MDATA_RET;
    }

    if (dmx->m_Pro->url_is_live(dmx->m_Pro) == 1)
    {
        meta->duation   = 0LL;
        dmx->m_Duration = 0ULL;
    }
    else
    {
        if (FAIL == TSParse_GetTSFDuration (dmx))
        {
            msg = "Get TS file duration failed";
            goto TSDEMUX_MDATA_RET;
        }
        meta->duation   = (int)dmx->m_Duration;
    }
    /// @todo Get file duration

    ret = SUCCESS;
    lev = MSGL_INFO;
    msg = "Parse metadata OK";

TSDEMUX_MDATA_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSDemux_Mdata : %s\n", msg);
#if 1
    ts_demux_log(0, lev, "\tAudio :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->audiocodec, meta->subaudiocodec, meta->audiostreamindex);
    ts_demux_log(0, lev, "\tVideo :::: Codec ID = 0x%-5X Sub Codec ID = %-2d Stream ID = %d\n"\
        , meta->videocodec, meta->subvideocodec, meta->videostreamindex);
    ts_demux_log(0, lev, "\tDuration = %d(ms)\n", meta->duation);
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

    if (dmx->m_TempAVPkt != NULL)
    {
        free (pack->data);
        pack->data         = dmx->m_TempAVPkt->data;dmx->m_TempAVPkt->data = NULL;
        pack->size         = dmx->m_TempAVPkt->size;
        pack->bufferlength = dmx->m_TempAVPkt->bufferlength;
        pack->pts          = dmx->m_TempAVPkt->pts;
        pack->dts          = dmx->m_TempAVPkt->dts;
        pack->stream_index = dmx->m_TempAVPkt->stream_index;
        free (dmx->m_TempAVPkt);
        dmx->m_TempAVPkt = NULL;
        msg = "Read a A/V packet OK";
        lev = MSGL_V;
        ret = pack->size;
        goto TSDEMUX_READAV_RET;
    }

    while (1)
    {
        if (FAIL == TSParse_GetSection (dmx))
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

    ret = pack->size;
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
int TSDemux_Seek  (DemuxContext* ctx, long long tms)
{
    I8 ret = FAIL;
    I8 lev = MSGL_ERR;
    const I8* msg;

    UI64 pos;
    
    TSDemuxer* dmx = (TSDemuxer*)ctx->priv_data;

    if (dmx->m_Pro->url_is_live(dmx->m_Pro) == 1)
    {
        msg = "Live Stream Cannot Seek";
        goto TSDEMUX_SEEK_RET;
    }
    if (dmx->m_PMTPID == 0 || dmx->m_AudioPID == 0 || dmx->m_VideoPID == 0)
    {
        msg = "Metadata hasn't been parsed cannot seek";
        goto TSDEMUX_SEEK_RET;
    }
    if (dmx->m_SupportSeek == FALSE)
    {
        msg = "Doesn't support seeking";
        goto TSDEMUX_SEEK_RET;
    }
    if (dmx->m_Duration == 0 || dmx->m_FileSize == 0)
    {
        msg = "File size or duration doesn't indicate cannot seek";
        goto TSDEMUX_SEEK_RET;
    }
    if (dmx->m_Duration - (UI64)tms < 3000)
    {
        msg = "Close to file end, cannot seek";
        goto TSDEMUX_SEEK_RET;
    }
    TSParse_ClrPrePack(dmx);
    pos = dmx->m_FileSize * (UI64)tms / dmx->m_Duration;
    if (FAIL == dmx->m_Pro->url_seek(dmx->m_Pro, pos, SEEK_SET))
    {
        msg = "Calling url_seek failed";
        goto TSDEMUX_SEEK_RET;
    }
    if (FAIL == TSParse_InitParser(dmx))
    {
        msg = "Initialize parsing failed";
        goto TSDEMUX_SEEK_RET;
    }

    while (1)
    {
        if (FAIL == TSParse_GetSection (dmx))
        {
            msg = "Get a section failed";
            goto TSDEMUX_SEEK_RET;
        }
        if (dmx->m_Section->m_DataLen == 0)
        {
            msg = "Stream End";
            goto TSDEMUX_SEEK_RET;
        }
        if (dmx->m_Section->m_Type != dmx->m_VideoPID)
        {
            continue;
        }
        if (dmx->m_TempAVPkt == NULL)
        {
            dmx->m_TempAVPkt = (AVPacket*)malloc(sizeof(AVPacket));
            memset(dmx->m_TempAVPkt, 0, sizeof(AVPacket));
        }
        if (FAIL == TSParse_PESSection(dmx, dmx->m_TempAVPkt))
        {
            msg = "Parse PES section failed";
            goto TSDEMUX_SEEK_RET;
        }
        if (dmx->m_TempAVPkt->pts == -1)
        {
            continue;
        }
        if (FAIL == TSParse_CheckPESKFrame(dmx->m_TempAVPkt->data, (UI32)dmx->m_TempAVPkt->size))
        {
            continue;
        }
        msg = "Seek OK";
        ret = SUCCESS;
        lev = MSGL_INFO;
        break;
    }
TSDEMUX_SEEK_RET:
    ts_demux_log(0, lev, "DEMUX ################ TSDemux_Seek : Timestamp = %-5d(s) %s"\
        , (int)(tms/1000), msg);
    if (ret == SUCCESS)
    {
        ts_demux_log(0, lev, " Current Position = %llu", dmx->m_Position);
    }
    ts_demux_log(0, lev, "\n");
    return ret;
}



