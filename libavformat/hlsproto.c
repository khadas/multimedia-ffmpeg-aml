/*
 * Apple HTTP Live Streaming Protocol Handler
 * Copyright (c) 2010 Martin Storsjo
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Apple HTTP Live Streaming Protocol Handler
 * https://www.rfc-editor.org/rfc/rfc8216.txt
 */

#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include "avio_internal.h"
#include "internal.h"
#include "url.h"
#include "version.h"
#ifdef AMFFMPEG
#include <stdbool.h>
#ifdef __ANDROID__
#include <cutils/properties.h>
#endif
#endif
/*
 * An apple http stream consists of a playlist with media segment files,
 * played sequentially. There may be several playlists with the same
 * video content, in different bandwidth variants, that are played in
 * parallel (preferably only one bandwidth variant at a time). In this case,
 * the user supplied the url to a main playlist that only lists the variant
 * playlists.
 *
 * If the main playlist doesn't point at any variants, we still create
 * one anonymous toplevel variant for this, to maintain the structure.
 */
enum KeyType {
    KEY_NONE,
    KEY_AES_128,
    KEY_SAMPLE_AES
};

struct key_info {
     char uri[MAX_URL_SIZE];
     char method[11];
     char iv[35];
};

struct segment {
    int64_t duration;
    char url[MAX_URL_SIZE];
    char * seg_kurl;
    uint8_t seg_iv[33];
};

struct variant {
    int bandwidth;
    char url[MAX_URL_SIZE];
};

typedef struct HLSContext {
    const AVClass *class;
    char playlist_url[MAX_URL_SIZE];
    int64_t target_duration;
    int start_seq_no;
    int finished;
    int n_segments;
    struct segment **segments;
    int n_variants;
    struct variant **variants;
    int cur_seq_no;
    int cur_seq_size;
#ifdef AMFFMPEG
    int64_t durations;
#else
    int durations;
#endif
    char* cur_kurl;
    uint8_t *cur_iv;
    int cur_iv_len;
    URLContext *seg_hd;
    int64_t last_load_time;
    int is_encrypted;
#ifdef AMFFMPEG
    int index_variants;
    int mReadByte;
    int64_t mReadTime;
    int64_t mBandWidth[3];
    int bandwidth_index;
    int reconnect;
#endif
} HLSContext;

static const AVOption hls_options[] = {
    {"cur_seq_no", "current segment index number", offsetof(HLSContext, cur_seq_no), AV_OPT_TYPE_INT,{.i64 = 0}, 0, INT_MAX, AV_OPT_FLAG_EXPORT},
    {"cur_seq_size", "current segment size", offsetof(HLSContext, cur_seq_size), AV_OPT_TYPE_INT,{.i64 = 0}, 0, INT_MAX, AV_OPT_FLAG_EXPORT},
    {"cur_kurl", "current key url", offsetof(HLSContext,cur_kurl), AV_OPT_TYPE_STRING, {.str=""}},
    {"cur_iv",  "current iv", offsetof(HLSContext,cur_iv),  AV_OPT_TYPE_BINARY, .flags = AV_OPT_FLAG_DECODING_PARAM|AV_OPT_FLAG_ENCODING_PARAM },
#ifdef AMFFMPEG
    {"durations",  "durations", offsetof(HLSContext,durations),  AV_OPT_TYPE_INT64,{.i64 = 0}, 0, INT64_MAX, AV_OPT_FLAG_EXPORT},
    {"reconnect", "auto reconnect after ts-segment disconnect before EOF", offsetof(HLSContext,reconnect), AV_OPT_TYPE_BOOL, { .i64 = 0 }, 0, 1, AV_OPT_FLAG_DECODING_PARAM },
#else
    {"durations",  "durations", offsetof(HLSContext,durations),  AV_OPT_TYPE_INT,{.i64 = 0}, 0, INT_MAX, AV_OPT_FLAG_EXPORT},
#endif
    { NULL },
};

static const AVClass hls_context_class = {
    .class_name = "hls protocol",
    .item_name  = av_default_item_name,
    .option     = hls_options,
    .version    = LIBAVUTIL_VERSION_INT,
};
static int read_chomp_line(AVIOContext *s, char *buf, int maxlen)
{
    int len = ff_get_line(s, buf, maxlen);
    while (len > 0 && av_isspace(buf[len - 1]))
        buf[--len] = '\0';
    return len;
}

static void free_segment_list(HLSContext *s)
{
    int i;
    for (i = 0; i < s->n_segments; i++) {
        if (s->segments[i]->seg_kurl)
        {
            av_free (s->segments[i]->seg_kurl);
            s->segments[i]->seg_kurl = NULL;
        }
        av_freep(&s->segments[i]);
    }
    av_freep(&s->segments);
    s->n_segments = 0;
}

static void free_variant_list(HLSContext *s)
{
    int i;
    for (i = 0; i < s->n_variants; i++)
        av_freep(&s->variants[i]);
    av_freep(&s->variants);
    s->n_variants = 0;
    if (s->cur_kurl)
    {
        av_free(s->cur_kurl);
        s->cur_kurl = NULL;
    }
    if (s->cur_iv)
    {
        av_free(s->cur_iv);
        s->cur_iv = NULL;
    }
}

struct variant_info {
    char bandwidth[20];
};

static void handle_variant_args(struct variant_info *info, const char *key,
                                int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "BANDWIDTH=", key_len)) {
        *dest     =        info->bandwidth;
        *dest_len = sizeof(info->bandwidth);
    }
}

static void handle_key_args(struct key_info *info, const char *key,
                            int key_len, char **dest, int *dest_len)
{
    if (!strncmp(key, "METHOD=", key_len)) {
        *dest     =        info->method;
        *dest_len = sizeof(info->method);
    } else if (!strncmp(key, "URI=", key_len)) {
        *dest     =        info->uri;
        *dest_len = sizeof(info->uri);
    } else if (!strncmp(key, "IV=", key_len)) {
        *dest     =        info->iv;
        *dest_len = sizeof(info->iv);
    }
}

#ifdef AMFFMPEG
static void sortByBandwidth(HLSContext *s)
{
    int i, j;
    struct variant *tmp;
    for (i = 0; i < s->n_variants - 1; i++) {
        for (j = i+1; j < s->n_variants; j++) {
            if (s->variants[i]->bandwidth > s->variants[j]->bandwidth) {

                tmp = s->variants[j];
                s->variants[j] = s->variants[i];
                s->variants[i] = tmp;

            }
        }
    }

    for (i = 0; i < s->n_variants; i++) {
        av_log(NULL, AV_LOG_ERROR, "[%s %d]bandwidth=%d\n", __FUNCTION__,__LINE__,s->variants[i]->bandwidth);
    }
}

static int parse_playlist(URLContext *h, const char *url,uint64_t *best_bandwidth)
#else
static int parse_playlist(URLContext *h, const char *url)
#endif
{
    HLSContext *s = h->priv_data;
    AVIOContext *in;
    int ret = 0, is_segment = 0, is_variant = 0, bandwidth = 0;
    int64_t duration = 0;
    char line[1024];
    const char *ptr;
    enum KeyType key_type = KEY_NONE;
    uint8_t iv[16] = "";
    int has_iv = 0;
    char * kurl = NULL;
    uint8_t kiv[33]= {0};
#ifdef AMFFMPEG
    int playlist_size = 0;
    int64_t start_time = av_gettime_relative();
    int64_t end_time = 0;
#endif
    if ((ret = ffio_open_whitelist(&in, url, AVIO_FLAG_READ,
                                   &h->interrupt_callback, NULL,
                                   h->protocol_whitelist, h->protocol_blacklist)) < 0)
        return ret;
#ifdef AMFFMPEG
    playlist_size = read_chomp_line(in, line, sizeof(line));
#else
    ff_get_chomp_line(in, line, sizeof(line));
#endif
    if (strcmp(line, "#EXTM3U")) {
        ret = AVERROR_INVALIDDATA;
        goto fail;
    }

    free_segment_list(s);
    s->finished = 0;
    while (!avio_feof(in)) {
#ifdef AMFFMPEG
        playlist_size += read_chomp_line(in, line, sizeof(line));
#else
        ff_get_chomp_line(in, line, sizeof(line));
#endif
        av_log(NULL, AV_LOG_ERROR, "%s\n",line);
        if (av_strstart(line, "#EXT-X-STREAM-INF:", &ptr)) {
            struct variant_info info = {{0}};
            is_variant = 1;
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_variant_args,
                               &info);
            bandwidth = atoi(info.bandwidth);
        } else if (av_strstart(line, "#EXT-X-KEY:", &ptr)) {
            struct key_info info = {{0}};
            s->is_encrypted = 1;
            ff_parse_key_value(ptr, (ff_parse_key_val_cb) handle_key_args,
                               &info);
            has_iv = 0;
            if (!strcmp(info.method, "AES-128"))
                key_type = KEY_AES_128;
            if (!strcmp(info.method, "SAMPLE-AES"))
                key_type = KEY_SAMPLE_AES;
            if (!strncmp(info.iv, "0x", 2) || !strncmp(info.iv, "0X", 2)) {
                ff_hex_to_data(iv, info.iv + 2);
                has_iv = 1;
            }
            kurl = av_strdup (info.uri);
            memcpy(kiv,info.iv+2,32);
        } else if (av_strstart(line, "#EXT-X-TARGETDURATION:", &ptr)) {
            s->target_duration = atoi(ptr) * AV_TIME_BASE;
        } else if (av_strstart(line, "#EXT-X-MEDIA-SEQUENCE:", &ptr)) {
            s->start_seq_no = atoi(ptr);
        } else if (av_strstart(line, "#EXT-X-ENDLIST", &ptr)) {
            s->finished = 1;
        } else if (av_strstart(line, "#EXTINF:", &ptr)) {
            is_segment = 1;
            duration = atof(ptr) * AV_TIME_BASE;
        } else if (av_strstart(line, "#", NULL)) {
            continue;
        } else if (line[0]) {
            if (is_segment) {
                struct segment *seg = av_malloc(sizeof(struct segment));
                if (!seg) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                seg->duration = duration;
                if (key_type != KEY_NONE)
                {
                    seg->seg_kurl =  av_strdup(kurl);
                    memcpy(seg->seg_iv,kiv,32);
                    seg->seg_iv[32]='\0';
                }
                ff_make_absolute_url(seg->url, sizeof(seg->url), url, line);
                dynarray_add(&s->segments, &s->n_segments, seg);
                is_segment = 0;
            } else if (is_variant) {
                struct variant *var = av_malloc(sizeof(struct variant));
                if (!var) {
                    ret = AVERROR(ENOMEM);
                    goto fail;
                }
                var->bandwidth = bandwidth;
                ff_make_absolute_url(var->url, sizeof(var->url), url, line);
                dynarray_add(&s->variants, &s->n_variants, var);
                is_variant = 0;
            }
        }
    }
    s->last_load_time = av_gettime_relative();
#ifdef AMFFMPEG
    end_time = av_gettime_relative();
    if (end_time != start_time)
    {
        *best_bandwidth = ((uint64_t)(8 * playlist_size * 1000) /((end_time - start_time)/1000));
        av_log(NULL, AV_LOG_WARNING, "start_time %lld end %lld playlist_size %d *best_bandwidth %lld\n",start_time,end_time,playlist_size,*best_bandwidth);
    }
#endif
fail:
    if (kurl)
    {
        av_free (kurl);
        kurl = NULL;
    }
    avio_close(in);
    return ret;
}

static int hls_close(URLContext *h)
{
    HLSContext *s = h->priv_data;

    free_segment_list(s);
    free_variant_list(s);
    ffurl_closep(&s->seg_hd);
    return 0;
}

static int hls_open(URLContext *h, const char *uri, int flags)
{
    HLSContext *s = h->priv_data;
    int ret, i;
    const char *nested_url;
    if (flags & AVIO_FLAG_WRITE)
        return AVERROR(ENOSYS);

    h->is_streamed = 1;

    if (av_strstart(uri, "hls+", &nested_url)) {
        av_strlcpy(s->playlist_url, nested_url, sizeof(s->playlist_url));
    } else if (av_strstart(uri, "hls://", &nested_url)) {
        av_log(h, AV_LOG_ERROR,
               "No nested protocol specified. Specify e.g. hls+http://%s\n",
               nested_url);
        ret = AVERROR(EINVAL);
        goto fail;
    } else {
        av_log(h, AV_LOG_ERROR, "Unsupported url %s\n", uri);
        ret = AVERROR(EINVAL);
        goto fail;
    }
    av_log(h, AV_LOG_WARNING,
           "Using the hls protocol is discouraged, please try using the "
           "hls demuxer instead. The hls demuxer should be more complete "
           "and work as well as the protocol implementation. (If not, "
           "please report it.) To use the demuxer, simply use %s as url.\n",
           s->playlist_url);
#ifdef AMFFMPEG
    uint64_t bw = 0;
    if ((ret = parse_playlist(h, s->playlist_url, &bw)) < 0)
        goto fail;

    if (s->n_segments == 0 && s->n_variants > 0) {
        int max_bandwidth = 0;
        s->index_variants = 0;
        char value[92] = {0};

#ifdef __ANDROID__
        if (property_get("media.amffmpeg.start.bw", value, NULL))
        {
            bw = atoi((const char*)value);
            av_log(NULL, AV_LOG_WARNING, "bw=%lld\n",bw);
        } else
#endif
            bw = 0;
        for (i = 0; i < s->n_variants; i++) {
            if (s->variants[i]->bandwidth <= bw) {
                s->index_variants = i;
            }
        }

        av_strlcpy(s->playlist_url, s->variants[s->index_variants]->url,
                   sizeof(s->playlist_url));
        if ((ret = parse_playlist(h, s->playlist_url, &bw)) < 0)
            goto fail;
    }
#else
    if ((ret = parse_playlist(h, s->playlist_url)) < 0)
        goto fail;

    if (s->n_segments == 0 && s->n_variants > 0) {
        int max_bandwidth = 0, max_bandwidth_index = -1;
        for (i = 0; i < s->n_variants; i++) {
            if (s->variants[i]->bandwidth > max_bandwidth || i == 0) {
                max_bandwidth = s->variants[i]->bandwidth;
                max_bandwidth_index = i;
            }
        }
        av_strlcpy(s->playlist_url, s->variants[max_bandwidth_index]->url,
                   sizeof(s->playlist_url));
        if ((ret = parse_playlist(h, s->playlist_url)) < 0)
            goto fail;
    }
#endif
    if (s->n_segments == 0) {
        av_log(h, AV_LOG_WARNING, "Empty playlist\n");
        ret = AVERROR(EIO);
        goto fail;
    }
    s->cur_seq_no = s->start_seq_no;
    if (!s->finished && s->n_segments >= 3)
        s->cur_seq_no = s->start_seq_no + s->n_segments - 3;
    if (s->finished)
    {
       for (i = 0 ; i < s->n_segments ; i++)
       {
         s->durations += s->segments[i]->duration;
       }
       //if (av_opt_set_int(s, "durations", s->durations, 0) < 0)
       //     av_log(s, AV_LOG_ERROR, "set s->duration error!\n");
    }
#ifdef AMFFMPEG
    memset(s->mBandWidth, 0, sizeof(s->mBandWidth));
    s->bandwidth_index = 0;
#endif
    return 0;

fail:
    hls_close(h);
    return ret;
}

static int hls_read(URLContext *h, uint8_t *buf, int size)
{
    HLSContext *s = h->priv_data;
    const char *url;
    int ret;
    int64_t reload_interval;
    int64_t cur_no = 0;
    int64_t seg_size = 0;
#ifdef AMFFMPEG
    int64_t bw = 0 ;
    int64_t bandwidth = 0 ;
#endif
start:
    if (s->seg_hd) {
#ifdef AMFFMPEG
        int64_t start = av_gettime_relative();
        ret = ffurl_read(s->seg_hd, buf, size);
        int64_t delay = av_gettime_relative() - start;
#else
        ret = ffurl_read(s->seg_hd, buf, size);
#endif

        if (s->start_seq_no && (s->cur_seq_no >= s->start_seq_no))
            cur_no = s->cur_seq_no - s->start_seq_no ;
        else
            cur_no = s->cur_seq_no;

#ifndef AMFFMPEG
        cur_no = s->cur_seq_no;
#endif
        //if (av_opt_set_int(s, "cur_seq_no", cur_no, 0) < 0)
        //    av_log(s, AV_LOG_WARNING, "set seg_no error!\n");

        if (av_opt_get_int(s->seg_hd->priv_data, "http_file_size", 0, &seg_size) < 0)
        {
            av_log(s, AV_LOG_WARNING,  "get http_file_size fail\n");
        }
        else
        {
            s->cur_seq_size = seg_size;
            //if (av_opt_set_int(s, "cur_seq_size", seg_size, 0) < 0)
            //    av_log(s, AV_LOG_WARNING, "set seg_size error!\n");
        }
#ifdef AMFFMPEG
        if (s->is_encrypted && (cur_no < s->n_segments && s->n_segments > 0))
        {
            // memcpy(s->cur_iv , s->segments[cur_no]->seg_iv, 33);
            // memcpy(s->cur_kurl , s->segments[cur_no]->seg_kurl, strlen(s->segments[cur_no]->seg_kurl)+1);
            if (s->segments[cur_no]->seg_iv[0] == '\0')
            {
                sprintf(s->segments[cur_no]->seg_iv, "%032x", s->cur_seq_no);
                av_log(s, AV_LOG_INFO,  "get cur_no=%lld,s->start_seq_no=%d, cur_seq_no=%d. iv=%s\n", cur_no, s->start_seq_no, s->cur_seq_no,s->segments[cur_no]->seg_iv);
            }
#else
        if (s->is_encrypted)
        {
#endif
            if (av_opt_set(s, "cur_iv", s->segments[cur_no]->seg_iv, 0) < 0)
            {
              av_log(s, AV_LOG_WARNING, "set cur_iv error\n");
            }
            if ((s->n_segments > 0) && av_opt_set(s, "cur_kurl", s->segments[cur_no]->seg_kurl, 0) < 0)
            {
                av_log(s, AV_LOG_WARNING, "set cur_kurl error!\n");
            }
        }
        if (ret > 0) {
            /*av_log(s, AV_LOG_ERROR, "[%s %d] size=%d delay=%lld\n",
                    __FUNCTION__, __LINE__, ret, delay);
                    */
#ifdef AMFFMPEG
            s->mReadTime += delay;
            s->mReadByte += ret;
#endif
            return ret;
        }
    }
    if (s->seg_hd) {
#ifdef AMFFMPEG
        if (s->n_variants > 0) {
            bandwidth = (int64_t)s->mReadByte * 8E6 / s->mReadTime;
            av_log(s, AV_LOG_ERROR, "[%s %d] %d bandwidth=%lld current_bw=%d size=%d delay=%lld ret=%d\n",
                    __FUNCTION__, __LINE__, s->cur_seq_size, bandwidth, s->variants[s->index_variants]->bandwidth, s->mReadByte, s->mReadTime, ret);
            s->mReadByte = 0;
            s->mReadTime = 0;
            if (s->bandwidth_index ==3) {
                s->bandwidth_index = 0;
            }
            if (bandwidth > 0)
                s->mBandWidth[s->bandwidth_index++] = bandwidth;
        }
#endif
        ffurl_closep(&s->seg_hd);
        s->cur_seq_no++;
    }
    reload_interval = s->n_segments > 0 ?
        s->segments[s->n_segments - 1]->duration :
        s->target_duration;
#ifdef AMFFMPEG

    int i;
    int needSwitch = 0;
    int temp;
    int64_t minEstimate = -1, maxEstimate = -1;
    if (s->n_variants > 0) {
        for (i=0; i<3; i++) {
            if (minEstimate < 0 || minEstimate > s->mBandWidth[i]) {
                minEstimate = s->mBandWidth[i];
            }
            if (maxEstimate < 0 || maxEstimate < s->mBandWidth[i]) {
                maxEstimate = s->mBandWidth[i];
            }
            av_log(NULL, AV_LOG_ERROR, "estimate=%lld\n",s->mBandWidth[i]);
        }
        bool mIsStable = (maxEstimate <= minEstimate * 4 / 3)
            && bandwidth > minEstimate * 7 / 10;
        av_log(NULL, AV_LOG_ERROR, "minEstimate=%lld maxEstimate=%lld stable=%d\n",
                minEstimate, maxEstimate, mIsStable);
        temp = s->index_variants;
        if ((bandwidth < s->variants[s->index_variants]->bandwidth)) {
            for (i = 0; i < s->n_variants; i++) {
                if (s->variants[i]->bandwidth < bandwidth) {
                    temp = i;
                }
            }
            if (temp != s->index_variants) {
                s->index_variants = temp;
                av_log(s, AV_LOG_ERROR, "switch down index=%d\n",s->index_variants);
                needSwitch = 1;
                av_strlcpy(s->playlist_url, s->variants[s->index_variants]->url,
                        sizeof(s->playlist_url));
            }
        } else if (bandwidth > 13*s->variants[s->index_variants]->bandwidth/10) {
            for (i = 0; i < s->n_variants; i++) {
                av_log(NULL, AV_LOG_ERROR, "bandwidth=%lld s->variants[i]->bandwidth=%d\n",
                        bandwidth, s->variants[i]->bandwidth);
                if ((int64_t)s->variants[i]->bandwidth < bandwidth) {
                    temp = i;
                }
            }
            if (temp != s->index_variants && (mIsStable || minEstimate == 0)) {
                s->index_variants = temp;
                av_log(s, AV_LOG_ERROR, "switch up index=%d\n", s->index_variants);
                needSwitch = 1;
                av_strlcpy(s->playlist_url, s->variants[s->index_variants]->url,
                        sizeof(s->playlist_url));
            }
        }
    }
#endif
retry:
#ifdef AMFFMPEG
    if (s->finished && needSwitch)
        if ((ret = parse_playlist(h, s->playlist_url, &bw)) < 0)
            return ret;
#endif
    if (!s->finished) {
        int64_t now = av_gettime_relative();
#ifdef AMFFMPEG
        if (now - s->last_load_time >= reload_interval || needSwitch) {
            if ((ret = parse_playlist(h, s->playlist_url, &bw)) < 0)
#else
        if (now - s->last_load_time >= reload_interval) {
            if ((ret = parse_playlist(h, s->playlist_url)) < 0)
#endif
                return ret;
            /* If we need to reload the playlist again below (if
             * there's still no more segments), switch to a reload
             * interval of half the target duration. */
            reload_interval = s->target_duration / 2;
        }
    }
    if (s->cur_seq_no < s->start_seq_no) {
        av_log(h, AV_LOG_WARNING,
               "skipping %d segments ahead, expired from playlist\n",
               s->start_seq_no - s->cur_seq_no);
        s->cur_seq_no = s->start_seq_no;
    }
    if (s->cur_seq_no - s->start_seq_no >= s->n_segments) {
        if (s->finished)
            return AVERROR_EOF;
        while (av_gettime_relative() - s->last_load_time < reload_interval) {
            if (ff_check_interrupt(&h->interrupt_callback))
                return AVERROR_EXIT;
            av_usleep(100*1000);
        }
        goto retry;
    }
    url = s->segments[s->cur_seq_no - s->start_seq_no]->url;
#ifdef AMFFMPEG
    av_log(h, AV_LOG_DEBUG, "opening %s \n cur_no=%lld cur_seq_no=%d\n", url, cur_no, s->cur_seq_no);
    AVDictionary *opts = NULL;
    char temp_char[2] = {0};
    memset(temp_char, 0, sizeof(temp_char));
    sprintf(temp_char, "%d", s->reconnect);
    av_dict_set(&opts, "reconnect", temp_char, 0);
    ret = ffurl_open_whitelist(&s->seg_hd, url, AVIO_FLAG_READ,
                            &h->interrupt_callback, &opts,
                            h->protocol_whitelist, h->protocol_blacklist, h);
#else
    ret = ffurl_open_whitelist(&s->seg_hd, url, AVIO_FLAG_READ,
                               &h->interrupt_callback, NULL,
                               h->protocol_whitelist, h->protocol_blacklist, h);
#endif
    if (ret < 0) {
        if (ff_check_interrupt(&h->interrupt_callback))
            return AVERROR_EXIT;
        av_log(h, AV_LOG_WARNING, "Unable to open %s\n", url);
        s->cur_seq_no++;
        goto retry;
    }
    goto start;
}
static int64_t hls_seek_ext(URLContext *h, int64_t off, int whence)
{   /*add for drmplayer, off is segment idx (0 or start segment) OR timeUs*/
    int i = 0;
    HLSContext *s = h->priv_data;
#ifdef AMFFMPEG
    av_log(h, AV_LOG_WARNING, "hls_seek_ext enter cur_seq_no %d start_seq_no %d off 0x%llx s->durations %lld \n", s->cur_seq_no,s->start_seq_no,off,s->durations);
    if ((off >= s->start_seq_no && off <= s->durations) || s->start_seq_no)
#else
    if (off >= s->start_seq_no && off <= s->durations)
#endif
    {
       if (s->seg_hd)
       {
          ffurl_close(s->seg_hd);
          s->seg_hd = NULL;
       }
    }
    if ((off == 0) || (s->n_segments == 1))
    {
#ifdef AMFFMPEG
        if (s->finished)
#endif
            s->cur_seq_no = s->start_seq_no;
    }
    else if ((off > s->start_seq_no) && (off < s->durations))
    {
#ifdef AMFFMPEG
       int64_t temp_duration = s->durations;
#else
       int temp_duration = s->durations;
#endif
       for (i = s->n_segments -1; i >= 0; i--)
       {
          temp_duration -= s->segments[i]->duration;
          if (temp_duration < off)
          {
#ifdef AMFFMPEG
             s->cur_seq_no = i + s->start_seq_no;
#else
             s->cur_seq_no = i ;
#endif
             break;
          }
       }
    }
#ifdef AMFFMPEG
    av_log(h, AV_LOG_WARNING, "hls_seek_ext cur_seq_no %d start_seq_no %d off 0x%llx \n", s->cur_seq_no,s->start_seq_no,off);
#endif


    return off;
}
const URLProtocol ff_hls_protocol = {
    .name           = "hls",
    .url_open       = hls_open,
    .url_seek       = hls_seek_ext,
    .url_read       = hls_read,
    .url_close      = hls_close,
    .flags          = URL_PROTOCOL_FLAG_NESTED_SCHEME,
    .priv_data_size = sizeof(HLSContext),
    .priv_data_class     = &hls_context_class
};
