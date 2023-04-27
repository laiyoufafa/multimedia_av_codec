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

#include "audio_ffmpeg_mp3_decoder_plugin.h"
#include "avcodec_errors.h"

namespace OHOS {
namespace Media {

AudioFFMpegMp3DecoderPlugin::AudioFFMpegMp3DecoderPlugin() : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>()) {}

AudioFFMpegMp3DecoderPlugin::~AudioFFMpegMp3DecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

int32_t AudioFFMpegMp3DecoderPlugin::init(const Format &format)
{

    int32_t ret = basePlugin->AllocateContext("mp3");
    int32_t checkresult = AudioFFMpegMp3DecoderPlugin::checkinit(format);
    if (checkresult != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return checkresult;
    }
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("mp3 init error.");
        return ret;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("mp3 init error.");
        return ret;
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegMp3DecoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegMp3DecoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegMp3DecoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegMp3DecoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegMp3DecoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegMp3DecoderPlugin::getInputBufferSize() const
{
    uint32_t size = int(bit_rate / 150);
    return size;
}

uint32_t AudioFFMpegMp3DecoderPlugin::getOutputBufferSize() const
{
    uint32_t size = (int(sample_rate / 31) + 128) * channels * sizeof(short);
    return size;
}

Format AudioFFMpegMp3DecoderPlugin::GetFormat() const noexcept
{
    return basePlugin->GetFormat();
}

int32_t AudioFFMpegMp3DecoderPlugin::checkinit(const Format &format)
{
#define SUPPORT_SAMPLE_RATE 9
    int sample_rate_pick[SUPPORT_SAMPLE_RATE] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

    format.GetIntValue("channel-count", channels);
    format.GetIntValue("sample-rate", sample_rate);
    format.GetLongValue("bits-rate", bit_rate);
    if (channels < 1 || channels > 2) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    if (bit_rate > 320000) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }

    for (int i = 0; i < SUPPORT_SAMPLE_RATE; i++) {
        if (sample_rate == sample_rate_pick[i]) {
            break;
        } else if (i == SUPPORT_SAMPLE_RATE - 1) {
            return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
        }
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

} // namespace Media
} // namespace OHOS