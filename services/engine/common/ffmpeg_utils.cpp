/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ffmpeg_utils.h"

#include <algorithm>
#include <functional>

// #include "foundation/log.h"
#include "plugin_audio_tags.h"
#include "plugin_time.h"

namespace OHOS {
namespace Media {
namespace Codec {
// namespace Ffmpeg {
// Internal definitions
namespace {
// Histreamer channel layout to ffmpeg channel layout
std::map<AudioChannelLayout, uint64_t> g_toFFMPEGChannelLayout = {
    {AudioChannelLayout::MONO, AV_CH_LAYOUT_MONO},
    {AudioChannelLayout::STEREO, AV_CH_LAYOUT_STEREO},
    {AudioChannelLayout::CH_2POINT1, AV_CH_LAYOUT_2POINT1},
    {AudioChannelLayout::CH_2_1, AV_CH_LAYOUT_2_1},
    {AudioChannelLayout::SURROUND, AV_CH_LAYOUT_SURROUND},
    {AudioChannelLayout::CH_3POINT1, AV_CH_LAYOUT_3POINT1},
    {AudioChannelLayout::CH_4POINT0, AV_CH_LAYOUT_4POINT0},
    {AudioChannelLayout::CH_4POINT1, AV_CH_LAYOUT_4POINT1},
    {AudioChannelLayout::CH_2_2, AV_CH_LAYOUT_2_2},
    {AudioChannelLayout::QUAD, AV_CH_LAYOUT_QUAD},
    {AudioChannelLayout::CH_5POINT0, AV_CH_LAYOUT_5POINT0},
    {AudioChannelLayout::CH_5POINT1, AV_CH_LAYOUT_5POINT1},
    {AudioChannelLayout::CH_5POINT0_BACK, AV_CH_LAYOUT_5POINT0_BACK},
    {AudioChannelLayout::CH_5POINT1_BACK, AV_CH_LAYOUT_5POINT1_BACK},
    {AudioChannelLayout::CH_6POINT0, AV_CH_LAYOUT_6POINT0},
    {AudioChannelLayout::CH_6POINT0_FRONT, AV_CH_LAYOUT_6POINT0_FRONT},
    {AudioChannelLayout::HEXAGONAL, AV_CH_LAYOUT_HEXAGONAL},
    {AudioChannelLayout::CH_6POINT1, AV_CH_LAYOUT_6POINT1},
    {AudioChannelLayout::CH_6POINT1_BACK, AV_CH_LAYOUT_6POINT1_BACK},
    {AudioChannelLayout::CH_6POINT1_FRONT, AV_CH_LAYOUT_6POINT1_FRONT},
    {AudioChannelLayout::CH_7POINT0, AV_CH_LAYOUT_7POINT0},
    {AudioChannelLayout::CH_7POINT0_FRONT, AV_CH_LAYOUT_7POINT0_FRONT},
    {AudioChannelLayout::CH_7POINT1, AV_CH_LAYOUT_7POINT1},
    {AudioChannelLayout::CH_7POINT1_WIDE, AV_CH_LAYOUT_7POINT1_WIDE},
    {AudioChannelLayout::CH_7POINT1_WIDE_BACK, AV_CH_LAYOUT_7POINT1_WIDE_BACK},
    {AudioChannelLayout::OCTAGONAL, AV_CH_LAYOUT_OCTAGONAL},
    {AudioChannelLayout::HEXADECAGONAL, AV_CH_LAYOUT_HEXADECAGONAL},
    {AudioChannelLayout::STEREO_DOWNMIX, AV_CH_LAYOUT_STEREO_DOWNMIX},
};

// ffmpeg channel layout to histreamer channel layout
std::map<uint64_t, AudioChannelMasks> g_fromFFMPEGChannelLayout = {
    {AV_CH_FRONT_LEFT, AudioChannelMasks::FRONT_LEFT},
    {AV_CH_FRONT_RIGHT, AudioChannelMasks::FRONT_RIGHT},
    {AV_CH_FRONT_CENTER, AudioChannelMasks::FRONT_CENTER},
    {AV_CH_LOW_FREQUENCY, AudioChannelMasks::LOW_FREQUENCY},
    {AV_CH_BACK_LEFT, AudioChannelMasks::BACK_LEFT},
    {AV_CH_BACK_RIGHT, AudioChannelMasks::BACK_RIGHT},
    {AV_CH_FRONT_LEFT_OF_CENTER, AudioChannelMasks::FRONT_LEFT_OF_CENTER},
    {AV_CH_FRONT_RIGHT_OF_CENTER, AudioChannelMasks::FRONT_RIGHT_OF_CENTER},
    {AV_CH_BACK_CENTER, AudioChannelMasks::BACK_CENTER},
    {AV_CH_SIDE_LEFT, AudioChannelMasks::SIDE_LEFT},
    {AV_CH_SIDE_RIGHT, AudioChannelMasks::SIDE_RIGHT},
    {AV_CH_TOP_CENTER, AudioChannelMasks::TOP_CENTER},
    {AV_CH_TOP_FRONT_LEFT, AudioChannelMasks::TOP_FRONT_LEFT},
    {AV_CH_TOP_FRONT_CENTER, AudioChannelMasks::TOP_FRONT_CENTER},
    {AV_CH_TOP_FRONT_RIGHT, AudioChannelMasks::TOP_FRONT_RIGHT},
    {AV_CH_TOP_BACK_LEFT, AudioChannelMasks::TOP_BACK_LEFT},
    {AV_CH_TOP_BACK_CENTER, AudioChannelMasks::TOP_BACK_CENTER},
    {AV_CH_TOP_BACK_RIGHT, AudioChannelMasks::TOP_BACK_RIGHT},
    {AV_CH_STEREO_LEFT, AudioChannelMasks::STEREO_LEFT},
    {AV_CH_STEREO_RIGHT, AudioChannelMasks::STEREO_RIGHT},
    {AV_CH_WIDE_LEFT, AudioChannelMasks::WIDE_LEFT},
    {AV_CH_WIDE_RIGHT, AudioChannelMasks::WIDE_RIGHT},
    {AV_CH_SURROUND_DIRECT_LEFT, AudioChannelMasks::SURROUND_DIRECT_LEFT},
    {AV_CH_SURROUND_DIRECT_RIGHT, AudioChannelMasks::SURROUND_DIRECT_RIGHT},
    {AV_CH_LOW_FREQUENCY_2, AudioChannelMasks::LOW_FREQUENCY_2},
    {AV_CH_TOP_SIDE_LEFT, AudioChannelMasks::TOP_SIDE_LEFT},
    {AV_CH_TOP_SIDE_RIGHT, AudioChannelMasks::TOP_SIDE_RIGHT},
    {AV_CH_BOTTOM_FRONT_CENTER, AudioChannelMasks::BOTTOM_FRONT_CENTER},
    {AV_CH_BOTTOM_FRONT_LEFT, AudioChannelMasks::BOTTOM_FRONT_LEFT},
    {AV_CH_BOTTOM_FRONT_RIGHT, AudioChannelMasks::BOTTOM_FRONT_RIGHT},
};

const std::map<std::string, Tag> g_tagMap = {
    {"title", Tag::MEDIA_TITLE},
    {"artist", Tag::MEDIA_ARTIST},
    {"lyricist", Tag::MEDIA_LYRICIST},
    {"album", Tag::MEDIA_ALBUM},
    {"album-artist", Tag::MEDIA_ALBUM_ARTIST},
    {"date", Tag::MEDIA_DATE},
    {"comment", Tag::MEDIA_COMMENT},
    {"genre", Tag::MEDIA_GENRE},
    {"copyright", Tag::MEDIA_COPYRIGHT},
    {"language", Tag::MEDIA_LANGUAGE},
    {"description", Tag::MEDIA_DESCRIPTION},
    {"lyrics", Tag::MEDIA_LYRICS},
};

std::map<std::string, std::function<void(TagMap&, AVDictionaryEntry*)>> g_MediaMap = {
    {"title", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_TITLE>(tag->value); }},
    {"artist", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_ARTIST>(tag->value); }},
    {"lyricist", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_LYRICIST>(tag->value); }},
    {"album", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_ALBUM>(tag->value); }},
    {"album-artist", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_ALBUM_ARTIST>(tag->value); }},
    {"date", [](TagMap& meta, AVDictionaryEntry* tag) {
        uint32_t year;
        uint32_t month;
        uint32_t day = 0;
        if (sscanf_s(tag->value, "%04u-%02u-%02u", &year, &month, &day) == 3) { // 3
            meta.Insert<Tag::MEDIA_DATE>(RemoveDelimiter(tag->value, '-'));
        }
    }},
    {"comment", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_COMMENT>(tag->value); }},
    {"genre", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_GENRE>(tag->value); }},
    {"copyright", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_COPYRIGHT>(tag->value); }},
    {"language", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_LANGUAGE>(tag->value); }},
    {"description", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_DESCRIPTION>(tag->value); }},
    {"lyrics", [](TagMap& meta, AVDictionaryEntry* tag) {meta.Insert<Tag::MEDIA_LYRICS>(tag->value); }}
};

const std::vector<std::pair<AudioAacProfile, int32_t>> g_AacProfileMap = {
    {AudioAacProfile::MAIN, FF_PROFILE_AAC_MAIN},
    {AudioAacProfile::LC, FF_PROFILE_AAC_LOW},
    {AudioAacProfile::SSR, FF_PROFILE_AAC_SSR},
    {AudioAacProfile::LTP, FF_PROFILE_AAC_LTP},
    {AudioAacProfile::HE, FF_PROFILE_AAC_HE},
    {AudioAacProfile::HE_PS, FF_PROFILE_AAC_HE_V2},
    {AudioAacProfile::LD, FF_PROFILE_AAC_LD},
    {AudioAacProfile::ELD, FF_PROFILE_AAC_ELD},
};

// Histreamer pixel format to ffmpeg pixel format
std::map<VideoPixelFormat, AVPixelFormat> g_pixelFormatMap = {
    {VideoPixelFormat::YUV410P, AV_PIX_FMT_YUV410P},
    {VideoPixelFormat::YUV411P, AV_PIX_FMT_YUV411P},
    {VideoPixelFormat::YUV420P, AV_PIX_FMT_YUV420P},
    {VideoPixelFormat::NV12, AV_PIX_FMT_NV12},
    {VideoPixelFormat::NV21, AV_PIX_FMT_NV21},
    {VideoPixelFormat::YUYV422, AV_PIX_FMT_YUYV422},
    {VideoPixelFormat::YUV422P, AV_PIX_FMT_YUV422P},
    {VideoPixelFormat::YUV444P, AV_PIX_FMT_YUV444P},
    {VideoPixelFormat::RGBA, AV_PIX_FMT_RGBA},
    {VideoPixelFormat::ARGB, AV_PIX_FMT_ARGB},
    {VideoPixelFormat::ABGR, AV_PIX_FMT_ABGR},
    {VideoPixelFormat::BGRA, AV_PIX_FMT_BGRA},
    {VideoPixelFormat::RGB24, AV_PIX_FMT_RGB24},
    {VideoPixelFormat::BGR24, AV_PIX_FMT_BGR24},
    {VideoPixelFormat::PAL8, AV_PIX_FMT_PAL8},
    {VideoPixelFormat::GRAY8, AV_PIX_FMT_GRAY8},
    {VideoPixelFormat::MONOWHITE, AV_PIX_FMT_MONOWHITE},
    {VideoPixelFormat::MONOBLACK, AV_PIX_FMT_MONOBLACK},
    {VideoPixelFormat::YUVJ420P, AV_PIX_FMT_YUVJ420P},
    {VideoPixelFormat::YUVJ422P, AV_PIX_FMT_YUVJ422P},
    {VideoPixelFormat::YUVJ444P, AV_PIX_FMT_YUVJ444P},
};

std::map<VideoH264Profile, int32_t> g_H264ProfileMap = {
    {VideoH264Profile::BASELINE, FF_PROFILE_H264_BASELINE},
    {VideoH264Profile::MAIN, FF_PROFILE_H264_MAIN},
    {VideoH264Profile::EXTENDED, FF_PROFILE_H264_EXTENDED},
    {VideoH264Profile::HIGH, FF_PROFILE_H264_HIGH},
    {VideoH264Profile::HIGH10, FF_PROFILE_H264_HIGH_10},
    {VideoH264Profile::HIGH422, FF_PROFILE_H264_HIGH_422},
    {VideoH264Profile::HIGH444, FF_PROFILE_H264_HIGH_444}
};
std::vector<std::pair<AudioSampleFormat, AVSampleFormat>> g_pFfSampleFmtMap = {
    {AudioSampleFormat::U8,  AVSampleFormat::AV_SAMPLE_FMT_U8},
    {AudioSampleFormat::U8P, AVSampleFormat::AV_SAMPLE_FMT_U8P},
    {AudioSampleFormat::S16, AVSampleFormat::AV_SAMPLE_FMT_S16},
    {AudioSampleFormat::S16P, AVSampleFormat::AV_SAMPLE_FMT_S16P},
    {AudioSampleFormat::S32, AVSampleFormat::AV_SAMPLE_FMT_S32},
    {AudioSampleFormat::S32P, AVSampleFormat::AV_SAMPLE_FMT_S32P},
    {AudioSampleFormat::F32, AVSampleFormat::AV_SAMPLE_FMT_FLT},
    {AudioSampleFormat::F32P, AVSampleFormat::AV_SAMPLE_FMT_FLTP},
    {AudioSampleFormat::F64, AVSampleFormat::AV_SAMPLE_FMT_DBL},
    {AudioSampleFormat::F64P, AVSampleFormat::AV_SAMPLE_FMT_DBLP},
    {AudioSampleFormat::S64, AVSampleFormat::AV_SAMPLE_FMT_S64},
    {AudioSampleFormat::S64P, AVSampleFormat::AV_SAMPLE_FMT_S64P},
};
} // namespace

std::string AVStrError(int errnum)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
    return std::string(errbuf);
}

int64_t ConvertTimeFromFFmpeg(int64_t pts, AVRational base)
{
    int64_t out;
    if (pts == AV_NOPTS_VALUE) {
        out = -1;
    } else {
        AVRational bq = {1, HST_SECOND};
        out = av_rescale_q(pts, base, bq);
    }
    return out;
}

int64_t ConvertTimeToFFmpeg(int64_t timestampUs, AVRational base)
{
    int64_t result;
    if (base.num == 0) {
        result = AV_NOPTS_VALUE;
    } else {
        AVRational bq = {1, HST_SECOND};
        result = av_rescale_q(timestampUs, bq, base);
    }
    return result;
}

int FillAVPicture(AVFrame* picture, uint8_t* ptr, enum AVPixelFormat pixFmt, int width, int height)
{
    (void)picture;
    (void)ptr;
    (void)pixFmt;
    (void)width;
    (void)height;
    return 0;
}

int GetAVPictureSize(int pixFmt, int width, int height)
{
    AVFrame dummy;
    return FillAVPicture(&dummy, nullptr, static_cast<AVPixelFormat>(pixFmt), width, height);
}

std::string RemoveDelimiter(const char* str, char delimiter)
{
    std::string tmp(str);
    RemoveDelimiter(delimiter, tmp);
    return tmp;
}

void RemoveDelimiter(char delimiter, std::string& str)
{
    for (auto it = std::find(str.begin(), str.end(), delimiter); it != str.end();) {
        it = str.erase(it);
        if (*it != delimiter) {
            it = std::find(it, str.end(), delimiter);
        }
    }
}

void ReplaceDelimiter(const std::string& delmiters, char newDelimiter, std::string& str)
{
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (delmiters.find(newDelimiter) != std::string::npos) {
            *it = newDelimiter;
        }
    }
}

std::vector<std::string> SplitString(const char* str, char delimiter)
{
    std::vector<std::string> rtv;
    if (str) {
        SplitString(std::string(str), delimiter).swap(rtv);
    }
    return rtv;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
    if (str.empty()) {
        return {};
    }
    std::vector<std::string> rtv;
    std::string::size_type startPos = 0;
    std::string::size_type endPos = str.find_first_of(delimiter, startPos);
    while (startPos != endPos) {
        rtv.emplace_back(str.substr(startPos, endPos - startPos));
        if (endPos == std::string::npos) {
            break;
        }
        startPos = endPos + 1;
        endPos = str.find_first_of(delimiter, startPos);
    }
    return rtv;
}

AudioSampleFormat ConvFf2PSampleFmt(AVSampleFormat sampleFormat)
{
    auto ite = std::find_if(g_pFfSampleFmtMap.begin(), g_pFfSampleFmtMap.end(),
        [&sampleFormat] (const std::pair<AudioSampleFormat, AVSampleFormat>& item) ->bool {
        return item.second == sampleFormat;
    });
    if (ite == g_pFfSampleFmtMap.end()) {
        return AudioSampleFormat::NONE;
    }
    return ite->first;
}

AVSampleFormat ConvP2FfSampleFmt(AudioSampleFormat sampleFormat)
{
    auto ite = std::find_if(g_pFfSampleFmtMap.begin(), g_pFfSampleFmtMap.end(),
        [&sampleFormat] (const std::pair<AudioSampleFormat, AVSampleFormat>& item) ->bool {
        return item.first == sampleFormat;
    });
    if (ite == g_pFfSampleFmtMap.end()) {
        return AV_SAMPLE_FMT_NONE;
    }
    return ite->second;
}

AudioChannelLayout GetDefaultChannelLayout(int channels)
{
    AudioChannelLayout ret;
    switch (channels) {
        case 1: { // 1: MONO
            ret = AudioChannelLayout::MONO;
            break;
        }
        case 2: { // 2: STEREO
            ret = AudioChannelLayout::STEREO;
            break;
        }
        case 4: { // 4: CH_4POINT0
            ret = AudioChannelLayout::CH_4POINT0;
            break;
        }
        case 6: { // 6: CH_5POINT1
            ret = AudioChannelLayout::CH_5POINT1;
            break;
        }
        case 8: { // 8: CH_5POINT1POINT2 or CH_7POINT1
            ret = AudioChannelLayout::CH_5POINT1POINT2;
            break;
        }
        case 10: { // 10: CH_7POINT1POINT2 or CH_5POINT1POINT4 ?
            ret = AudioChannelLayout::CH_7POINT1POINT2;
            break;
        }
        case 12: { // 12: CH_7POINT1POINT4 or CH_10POINT2 ?
            ret = AudioChannelLayout::CH_7POINT1POINT4;
            break;
        }
        case 14: { // 14: CH_9POINT1POINT4
            ret = AudioChannelLayout::CH_9POINT1POINT4;
            break;
        }
        case 16: { // 16: CH_9POINT1POINT6
            ret = AudioChannelLayout::CH_9POINT1POINT6;
            break;
        }
        case 24: { // 24: CH_22POINT2
            ret = AudioChannelLayout::CH_22POINT2;
            break;
        }
        default: {
            ret = AudioChannelLayout::UNKNOWN;
            break;
        }
    }
    return ret;
}

AudioChannelLayout ConvertChannelLayoutFromFFmpeg(int channels, uint64_t ffChannelLayout)
{
    uint64_t channelLayout = 0;
    uint64_t mask;
    for (uint8_t bitPos = 0, channelNum = 0; (bitPos < 64) && (channelNum < channels); ++bitPos) { // 64
        mask = 1ULL << bitPos;
        if (!(mask & ffChannelLayout)) {
            continue;
        }
        channelNum++;
        auto it = g_fromFFMPEGChannelLayout.find(mask);
        if (it != g_fromFFMPEGChannelLayout.end()) {
            channelLayout |= static_cast<uint64_t>(it->second);
        } else {
            // MEDIA_LOG_W("unsupported audio channel layout: " PUBLIC_LOG_U64, mask);
        }
    }
    auto ret = static_cast<AudioChannelLayout>(channelLayout);
    if (ffChannelLayout == 0) {
        ret = GetDefaultChannelLayout(channels);
    }
    return ret;
}

uint64_t ConvertChannelLayoutToFFmpeg(AudioChannelLayout channelLayout)
{
    auto it = g_toFFMPEGChannelLayout.find(channelLayout);
    if (it == g_toFFMPEGChannelLayout.end()) {
        // MEDIA_LOG_E("ConvertChannelLayoutToFFmpeg: unknown audio channel layout: " PUBLIC_LOG_U64, channelLayout);
        return 0;
    }
    return it->second;
}

bool FindAvMetaNameByTag(Tag tag, std::string& metaName)
{
    for (const auto& pair : g_tagMap) {
        if (pair.second == tag) {
            metaName = pair.first;
            return true;
        }
    }
    return false;
}

void InsertMediaTag(TagMap& meta, AVDictionaryEntry* tag)
{
    for (auto e : g_MediaMap) {
        if (e.first == tag->key) {
            e.second(meta, tag);
            return;
        }
    }
}

AudioAacProfile ConvAacProfileFromFfmpeg(int32_t ffmpegProfile)
{
    auto ite = std::find_if(g_AacProfileMap.begin(), g_AacProfileMap.end(),
        [&] (const std::pair<AudioAacProfile, int32_t>& tmp) -> bool {
        return tmp.second == ffmpegProfile;
    });
    return ite == g_AacProfileMap.end() ? AudioAacProfile::NONE : ite->first;
}

int32_t ConvAacProfileToFfmpeg(AudioAacProfile profile)
{
    auto ite = std::find_if(g_AacProfileMap.begin(), g_AacProfileMap.end(),
        [&] (const std::pair<AudioAacProfile, int32_t>& tmp) -> bool {
        return tmp.first == profile;
    });
    return ite == g_AacProfileMap.end() ? FF_PROFILE_UNKNOWN : ite->second;
}

VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.second == ffmpegPixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? VideoPixelFormat::UNKNOWN : iter->first;
}

AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat)
{
    auto iter = std::find_if(g_pixelFormatMap.begin(), g_pixelFormatMap.end(),
        [&] (const std::pair<VideoPixelFormat, AVPixelFormat>& tmp) -> bool {
        return tmp.first == pixelFormat;
    });
    return iter == g_pixelFormatMap.end() ? AV_PIX_FMT_NONE : iter->second;
}

bool IsYuvFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_YUV420P || format == AV_PIX_FMT_NV12 || format == AV_PIX_FMT_NV21 ||
            format == AV_PIX_FMT_YUYV422 || format == AV_PIX_FMT_YUV422P || format == AV_PIX_FMT_YUV444P ||
            format == AV_PIX_FMT_YUV410P || format == AV_PIX_FMT_YUV411P || format == AV_PIX_FMT_YUVJ420P ||
            format == AV_PIX_FMT_YUVJ422P || format == AV_PIX_FMT_YUVJ444P);
}

bool IsRgbFormat(AVPixelFormat format)
{
    return (format == AV_PIX_FMT_ABGR || format == AV_PIX_FMT_ARGB || format == AV_PIX_FMT_RGBA ||
            format == AV_PIX_FMT_BGRA || format == AV_PIX_FMT_RGB24 || format == AV_PIX_FMT_BGR24);
}

VideoH264Profile ConvH264ProfileFromFfmpeg(int32_t ffmpegProfile)
{
    auto iter = std::find_if(g_H264ProfileMap.begin(), g_H264ProfileMap.end(),
                             [&] (const std::pair<VideoH264Profile, int32_t>& tmp) -> bool {
        return tmp.second == ffmpegProfile;
    });
    return (iter == g_H264ProfileMap.end()) ? VideoH264Profile::UNKNOWN : iter->first;
}

int32_t ConvH264ProfileToFfmpeg(VideoH264Profile profile)
{
    auto iter = std::find_if(g_H264ProfileMap.begin(), g_H264ProfileMap.end(),
                             [&] (const std::pair<VideoH264Profile, int32_t>& tmp) -> bool {
        return tmp.first == profile;
    });
    return (iter == g_H264ProfileMap.end()) ? FF_PROFILE_UNKNOWN : iter->second;
}

int32_t Resample::Init(const ResamplePara& resamplePara)
{
    resamplePara_ = resamplePara;
#if defined(_WIN32) || !defined(OHOS_LITE)
    if (resamplePara_.bitsPerSample != 8 && resamplePara_.bitsPerSample != 24) { // 8 24
        auto destFrameSize = av_samples_get_buffer_size(nullptr, resamplePara_.channels,
                                                        resamplePara_.destSamplesPerFrame, resamplePara_.destFmt, 0);
        resampleCache_.reserve(destFrameSize);
        resampleChannelAddr_.reserve(resamplePara_.channels);
        auto tmp = resampleChannelAddr_.data();
        av_samples_fill_arrays(tmp, nullptr, resampleCache_.data(), resamplePara_.channels,
                               resamplePara_.destSamplesPerFrame, resamplePara_.destFmt, 0);
        auto swrContext = swr_alloc();
        if (swrContext == nullptr) {
            // MEDIA_LOG_E("cannot allocate swr context");
            return AVCS_ERR_NO_MEMORY;
        }
        swrContext = swr_alloc_set_opts(swrContext, resamplePara_.channelLayout, resamplePara_.destFmt,
                                        resamplePara_.sampleRate, resamplePara_.channelLayout,
                                        resamplePara_.srcFfFmt, resamplePara_.sampleRate, 0, nullptr);
        if (swr_init(swrContext) != 0) {
            // MEDIA_LOG_E("swr init error");
            return AVCS_ERR_UNKNOWN;
        }
        swrCtx_ = std::shared_ptr<SwrContext>(swrContext, [](SwrContext *ptr) {
            if (ptr) {
                swr_free(&ptr);
            }
        });
    }
#endif
    return AVCS_ERR_OK;
}

int32_t Resample::Convert(const uint8_t* srcBuffer, const size_t srcLength, uint8_t*& destBuffer, size_t& destLength)
{
#if defined(_WIN32) || !defined(OHOS_LITE)
    if (resamplePara_.bitsPerSample == 8) { // 8
        // FALSE_RETURN_V_MSG(resamplePara_.destFmt == AV_SAMPLE_FMT_S16, AVCS_ERR_UNSUPPORT,
        //                    "resample 8bit to other format can not support");
        if(resamplePara_.destFmt != AV_SAMPLE_FMT_S16){
            return AVCS_ERR_UNSUPPORT;
        }
        destLength = srcLength * 2;  // 2
        resampleCache_.reserve(destLength);
        resampleCache_.assign(destLength, 0);
        for (size_t i {0}; i < destLength / 2; i++) { // 2
            memcpy_s(&resampleCache_[0] + i * 2 + 1, 1, srcBuffer + i, 1); // 0 2 1
            *(&resampleCache_[0] + i * 2 + 1) += 0x80; // 0x80
        }
        destBuffer = resampleCache_.data();
    } else if (resamplePara_.bitsPerSample == 24) {  // 24
        // FALSE_RETURN_V_MSG(resamplePara_.destFmt == AV_SAMPLE_FMT_S16, AVCS_ERR_UNSUPPORT,
        //                    "resample 24bit to other format can not support");
        if(resamplePara_.destFmt != AV_SAMPLE_FMT_S16){
            return AVCS_ERR_UNSUPPORT;
        }
        destLength = srcLength / 3 * 2; // 3 2
        resampleCache_.reserve(destLength);
        resampleCache_.assign(destLength, 0);
        for (size_t i = 0; i < destLength / 2; i++) { // 2
            memcpy_s(&resampleCache_[0] + i * 2, 2, srcBuffer + i * 3 + 1, 2); // 2 3 1
        }
        destBuffer = resampleCache_.data();
    } else {
        size_t lineSize = srcLength / resamplePara_.channels;
        std::vector<const uint8_t*> tmpInput(resamplePara_.channels);
        tmpInput[0] = srcBuffer;
        if (av_sample_fmt_is_planar(resamplePara_.srcFfFmt)) {
            for (size_t i = 1; i < tmpInput.size(); ++i) {
                tmpInput[i] = tmpInput[i-1] + lineSize;
            }
        }
        auto samples = lineSize / av_get_bytes_per_sample(resamplePara_.srcFfFmt);
        auto res = swr_convert(swrCtx_.get(), resampleChannelAddr_.data(), resamplePara_.destSamplesPerFrame,
                               tmpInput.data(), samples);
        if (res < 0) {
            // MEDIA_LOG_E("resample input failed");
            destLength = 0;
        } else {
            destBuffer = resampleCache_.data();
            destLength = res * av_get_bytes_per_sample(resamplePara_.destFmt) * resamplePara_.channels;
        }
    }
#endif
    return AVCS_ERR_OK;
}

// #if defined(VIDEO_SUPPORT)
int32_t Scale::Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize)
{
    scalePara_ = scalePara;
    if (swsCtx_ != nullptr) {
        return AVCS_ERR_OK;
    }
    auto swsContext = sws_getContext(scalePara_.srcWidth, scalePara_.srcHeight, scalePara_.srcFfFmt,
        scalePara_.dstWidth, scalePara_.dstHeight, scalePara_.dstFfFmt,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    // FALSE_RETURN_V_MSG_E(swsContext != nullptr, AVCS_ERR_UNKNOWN, "sws_getContext fail");
    if(swsContext == nullptr){
        return AVCS_ERR_UNKNOWN;
    }
    swsCtx_ = std::shared_ptr<SwsContext>(swsContext, [](struct SwsContext *ptr) {
        if (ptr != nullptr) {
            sws_freeContext(ptr);
        }
    });
    auto ret = av_image_alloc(dstData, dstLineSize, scalePara_.dstWidth, scalePara_.dstHeight,
        scalePara_.dstFfFmt, scalePara_.align);
    // FALSE_RETURN_V_MSG_E(ret >= 0, AVCS_ERR_UNKNOWN, "could not allocate destination image" PUBLIC_LOG_D32, ret);
    if(ret < 0){
        return AVCS_ERR_UNKNOWN;
    }
    // MEDIA_LOG_D("av_image_alloc call, ret: " PUBLIC_LOG_U32 "dstPixelFormat_: " PUBLIC_LOG_U32,
        // ret, scalePara_.dstFfFmt);
    // av_image_alloc can make sure that dstLineSize last element is 0
    for (int32_t i = 0; dstLineSize[i] > 0; i++) {
        // MEDIA_LOG_D("dstLineSize[" PUBLIC_LOG_D32 "]: " PUBLIC_LOG_D32, i, dstLineSize[i]);
        if (dstData[i] && !dstLineSize[i]) {
            // MEDIA_LOG_E("scale frame is broken, i: " PUBLIC_LOG_D32, i);
            return AVCS_ERR_UNKNOWN;
        }
    }
    return AVCS_ERR_OK;
}

int32_t Scale::Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize)
{
    auto res = sws_scale(swsCtx_.get(), srcData, srcLineSize, 0, scalePara_.srcHeight,
        dstData, dstLineSize);
    // FALSE_RETURN_V_MSG_E(res >= 0, AVCS_ERR_UNKNOWN, "sws_scale fail: " PUBLIC_LOG_D32, res);
    if(res < 0){
        return AVCS_ERR_UNKNOWN;
    }
    return AVCS_ERR_OK;
}
// #endif
// }// namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
