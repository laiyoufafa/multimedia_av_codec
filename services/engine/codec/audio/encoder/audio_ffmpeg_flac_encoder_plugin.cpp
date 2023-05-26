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

#include "audio_ffmpeg_flac_encoder_plugin.h"
#include "media_description.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "avcodec_mime_type.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegFlacEncoderPlugin"};
constexpr int32_t MIN_CHANNELS = 1;
constexpr int32_t MAX_CHANNELS = 8;
constexpr int32_t MIN_COMPLIANCE_LEVEL = -2;
constexpr int32_t MAX_COMPLIANCE_LEVEL = 2;
constexpr int32_t GET_INPUT_BUFFER_SIZE = 65536;
constexpr int32_t GET_OUTPUT_BUFFER_SIZE = 65536;
static const uint32_t FLAC_ENCODER_SAMPLE_RATE_TABLE[] = {
    0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
};
static const uint32_t FLAC_ENCODER_BITS_SAMPLE_TABLE[] = {16, 24, 32};
}

namespace OHOS {
namespace Media {
AudioFFMpegFlacEncoderPlugin::AudioFFMpegFlacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>())
{
}

AudioFFMpegFlacEncoderPlugin::~AudioFFMpegFlacEncoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static bool CheckSampleRate(uint32_t sample_rate)
{
    for (auto i : FLAC_ENCODER_SAMPLE_RATE_TABLE) {
        if (i == sample_rate) {
            return true;
        }
    }
    return false;
}

static bool CheckBitsPerSample(uint32_t bits_per_coded_sample)
{
    for (auto i : FLAC_ENCODER_BITS_SAMPLE_TABLE) {
        if (i == bits_per_coded_sample) {
            return true;
        }
    }
    return false;
}

int32_t AudioFFMpegFlacEncoderPlugin::CheckFormat(const Format &format) const
{
    int32_t compliance_level;
    int32_t channels;
    int32_t sample_rate;
    int32_t bits_per_coded_sample;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channels);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sample_rate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE, bits_per_coded_sample);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_COMPLIANCE_LEVEL, compliance_level);
    if (!CheckSampleRate(sample_rate)) {
        AVCODEC_LOGE("init failed, because sample rate=%{public}d not in table.", sample_rate);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    } else if (channels < MIN_CHANNELS || channels > MAX_CHANNELS) {
        AVCODEC_LOGE("init failed, because channels=%{public}d not support.", channels);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    } else if (!CheckBitsPerSample(bits_per_coded_sample)) {
        AVCODEC_LOGE("init failed, because bits_per_coded_sample=%{public}d not support.", bits_per_coded_sample);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_BIT_RATE;
    } else if (compliance_level < MIN_COMPLIANCE_LEVEL || compliance_level > MAX_COMPLIANCE_LEVEL) {
        AVCODEC_LOGE("init failed, because compliance_level=%{public}d not support.", compliance_level);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacEncoderPlugin::Init(const Format &format)
{
    int32_t ret = basePlugin->AllocateContext("flac");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because AllocateContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = CheckFormat(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because CheckFormat failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because InitContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->OpenContext();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because OpenContext failed. ret=%{public}d", ret);
        return ret;
    }

    ret = basePlugin->InitFrame();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because InitFrame failed. ret=%{public}d", ret);
        return ret;
    }

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacEncoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegFlacEncoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegFlacEncoderPlugin::Reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegFlacEncoderPlugin::Release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegFlacEncoderPlugin::Flush()
{
    return basePlugin->Flush();
}

int32_t AudioFFMpegFlacEncoderPlugin::GetInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > GET_INPUT_BUFFER_SIZE) {
        maxSize = GET_INPUT_BUFFER_SIZE;
    }
    return maxSize;
}

int32_t AudioFFMpegFlacEncoderPlugin::GetOutputBufferSize() const
{
    return GET_OUTPUT_BUFFER_SIZE;
}

Format AudioFFMpegFlacEncoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC);
    return format;
}
} // namespace Media
} // namespace OHOS