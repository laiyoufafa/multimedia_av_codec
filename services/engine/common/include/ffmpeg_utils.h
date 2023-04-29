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

#ifndef HISTREAMER_FFMPEG_UTILS_H
#define HISTREAMER_FFMPEG_UTILS_H

#include <string>
#include <type_traits>
#include <vector>
#include "plugin_tags.h"
#include "plugin_audio_tags.h"
#include "plugin_video_tags.h"
#include "tag_map.h"
#include "avcodec_errors.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
namespace Codec {
// namespace Ffmpeg {
std::string AVStrError(int errnum);

/**
 * Convert time from ffmpeg to time in HST_TIME_BASE.
 * @param pts ffmpeg time
 * @param base ffmpeg time_base
 * @return time in HST_TIME_BASE
 */
int64_t ConvertTimeFromFFmpeg(int64_t pts, AVRational base);

/**
 * Convert time in HST_TIME_BASE to ffmpeg time.
 * @param time time in HST_TIME_BASE
 * @param base ffmpeg time_base
 * @return time in ffmpeg.
 */
int64_t ConvertTimeToFFmpeg(int64_t timestampUs, AVRational base);

/*
 * Fill in pointers in an AVFrame, aligned by 4 (required by X).
 */
int FillAVPicture(AVFrame* picture, uint8_t* ptr, enum AVPixelFormat pixFmt, int width, int height);

/*
 * Get the size of an picture
 */
int GetAVPictureSize(int pixFmt, int width, int height);

void RemoveDelimiter(char delimiter, std::string& str);

std::string RemoveDelimiter(const char* str, char delimiter);

void ReplaceDelimiter(const std::string& delmiters, char newDelimiter, std::string& str);

std::vector<std::string> SplitString(const char* str, char delimiter);

std::vector<std::string> SplitString(const std::string& str, char delimiter);

AudioSampleFormat ConvFf2PSampleFmt(AVSampleFormat sampleFormat);

AVSampleFormat ConvP2FfSampleFmt(AudioSampleFormat sampleFormat);

AudioChannelLayout ConvertChannelLayoutFromFFmpeg(int channels, uint64_t ffChannelLayout);

uint64_t ConvertChannelLayoutToFFmpeg(AudioChannelLayout channelLayout);

bool FindAvMetaNameByTag(Tag tag, std::string& metaName);

void InsertMediaTag(TagMap& meta, AVDictionaryEntry* tag);

AudioAacProfile ConvAacProfileFromFfmpeg (int32_t ffmpegProfile);

int32_t ConvAacProfileToFfmpeg (AudioAacProfile profile);

VideoPixelFormat ConvertPixelFormatFromFFmpeg(int32_t ffmpegPixelFormat);

AVPixelFormat ConvertPixelFormatToFFmpeg(VideoPixelFormat pixelFormat);

bool IsYuvFormat(AVPixelFormat format);

bool IsRgbFormat(AVPixelFormat format);

VideoH264Profile ConvH264ProfileFromFfmpeg (int32_t ffmpegProfile);

int32_t ConvH264ProfileToFfmpeg(VideoH264Profile profile);

struct ResamplePara {
    uint32_t channels {2}; // 2: STEREO
    uint32_t sampleRate {0};
    uint32_t bitsPerSample {0};
    int64_t channelLayout {0};
    AVSampleFormat srcFfFmt {AV_SAMPLE_FMT_NONE};
    uint32_t destSamplesPerFrame {0};
    AVSampleFormat destFmt {AV_SAMPLE_FMT_S16};
};

class Resample {
public:
    int32_t Init(const ResamplePara& resamplePara);
    int32_t Convert(const uint8_t* srcBuffer, const size_t srcLength, uint8_t*& destBuffer, size_t& destLength);
private:
    ResamplePara resamplePara_ {};
#if defined(_WIN32) || !defined(OHOS_LITE)
    std::vector<uint8_t> resampleCache_ {};
    std::vector<uint8_t*> resampleChannelAddr_ {};
    std::shared_ptr<SwrContext> swrCtx_ {nullptr};
#endif
};

// #if defined(VIDEO_SUPPORT)
struct ScalePara {
    int32_t srcWidth {0};
    int32_t srcHeight {0};
    AVPixelFormat srcFfFmt {AVPixelFormat::AV_PIX_FMT_NONE};
    int32_t dstWidth {0};
    int32_t dstHeight {0};
    AVPixelFormat dstFfFmt {AVPixelFormat::AV_PIX_FMT_RGBA};
    int32_t align {16};
};

struct Scale {
public:
    int32_t Init(const ScalePara& scalePara, uint8_t** dstData, int32_t* dstLineSize);
    int32_t Convert(uint8_t** srcData, const int32_t* srcLineSize, uint8_t** dstData, int32_t* dstLineSize);
private:
    ScalePara scalePara_ {};
    std::shared_ptr<SwsContext> swsCtx_ {nullptr};
};
// #endif
// } // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_FFMPEG_UTILS_H
