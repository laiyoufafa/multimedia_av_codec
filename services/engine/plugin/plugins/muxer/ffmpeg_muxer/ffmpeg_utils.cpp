/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <unordered_map>
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
namespace Plugin {
namespace Ffmpeg {
// Internal definitions
namespace {
/* time scale microsecond */
constexpr int32_t TIME_SCALE_US = 1000000;

/* MIME to AVCodecID */
std::unordered_map<std::string_view, AVCodecID> g_mimeToCodecId = {
    {CodecMimeType::AUDIO_MPEG, AV_CODEC_ID_MP3},
    {CodecMimeType::AUDIO_FLAC, AV_CODEC_ID_FLAC},
    {CodecMimeType::AUDIO_RAW, AV_CODEC_ID_PCM_S16LE},
    {CodecMimeType::AUDIO_AAC, AV_CODEC_ID_AAC},
    {CodecMimeType::AUDIO_VORBIS, AV_CODEC_ID_VORBIS},
    {CodecMimeType::AUDIO_OPUS, AV_CODEC_ID_OPUS},
    {CodecMimeType::AUDIO_AMR_NB, AV_CODEC_ID_AMR_NB},
    {CodecMimeType::AUDIO_AMR_WB, AV_CODEC_ID_AMR_WB},
    {CodecMimeType::VIDEO_AVC, AV_CODEC_ID_H264},
    {CodecMimeType::VIDEO_MPEG4, AV_CODEC_ID_MPEG4},
    {CodecMimeType::IMAGE_JPG, AV_CODEC_ID_MJPEG},
    {CodecMimeType::IMAGE_PNG, AV_CODEC_ID_PNG},
    {CodecMimeType::IMAGE_BMP, AV_CODEC_ID_BMP},
};
} // namespace

bool Mime2CodecId(const std::string_view &mime, AVCodecID &codecId)
{
    auto it = g_mimeToCodecId.find(mime);
    if (it != g_mimeToCodecId.end()) {
        codecId = it->second;
        return true;
    }
    return false;
}

void ReplaceDelimiter(const std::string &delmiters, char newDelimiter, std::string &str)
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

std::vector<std::string> SplitString(const std::string &str, char delimiter)
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
        AVRational bq = {1, TIME_SCALE_US};
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
        AVRational bq = {1, TIME_SCALE_US};
        result = av_rescale_q(timestampUs, bq, base);
    }
    return result;
}
} // namespace Ffmpeg
} // namespace Plugin
} // namespace Media
} // namespace OHOS
