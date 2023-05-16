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
#include "ffmpeg_converter.h"
#include <vector>
namespace OHOS {
namespace Media {

// ffmpeg channel layout to histreamer channel layout
const std::vector<std::pair<AudioChannelLayout, uint64_t>> g_toFFMPEGChannelLayout = {
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

const std::vector<std::pair<AVSampleFormat, AudioSampleFormat>> g_pFfSampleFmtMap = {
    {AVSampleFormat::AV_SAMPLE_FMT_U8, AudioSampleFormat::SAMPLE_U8},
    {AVSampleFormat::AV_SAMPLE_FMT_S16, AudioSampleFormat::SAMPLE_S16LE},
    {AVSampleFormat::AV_SAMPLE_FMT_S32, AudioSampleFormat::SAMPLE_S32LE},
    {AVSampleFormat::AV_SAMPLE_FMT_FLT, AudioSampleFormat::SAMPLE_F32LE},
};

AudioSampleFormat FFMpegConverter::ConvertFFMpegToOHAudioFormat(AVSampleFormat ffSampleformate)
{
    auto ite = std::find_if(g_pFfSampleFmtMap.begin(), g_pFfSampleFmtMap.end(),
                            [&ffSampleformate](const auto &item) -> bool { return item.first == ffSampleformate; });
    if (ite == g_pFfSampleFmtMap.end()) {
        return AudioSampleFormat::INVALID_WIDTH;
    }
    return ite->second;
}
AudioChannelLayout FFMpegConverter::ConvertFFToOHAudioChannelLayout(uint64_t ffChannelLayout)
{
    auto ite = std::find_if(g_toFFMPEGChannelLayout.begin(), g_toFFMPEGChannelLayout.end(),
                            [&ffChannelLayout](const auto &item) -> bool { return item.second == ffChannelLayout; });
    if (ite == g_toFFMPEGChannelLayout.end()) {
        return AudioChannelLayout::MONO;
    }
    return ite->first;
}
} // namespace Media
} // namespace OHOS