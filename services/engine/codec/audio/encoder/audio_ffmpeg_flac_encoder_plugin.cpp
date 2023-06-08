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
#include "avcodec_audio_common.h"
#include "ffmpeg_converter.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegFlacEncoderPlugin"};
constexpr int32_t MIN_CHANNELS = 1;
constexpr int32_t MAX_CHANNELS = 8;
constexpr int32_t MIN_COMPLIANCE_LEVEL = -2;
constexpr int32_t MAX_COMPLIANCE_LEVEL = 2;
constexpr int32_t SAMPLES = 4608;
static const int32_t FLAC_ENCODER_SAMPLE_RATE_TABLE[] = {
    88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000,
};
static const uint64_t FLAC_CHANNEL_LAYOUT_TABLE[] = {AV_CH_LAYOUT_MONO,    AV_CH_LAYOUT_STEREO,  AV_CH_LAYOUT_SURROUND,
                                                     AV_CH_LAYOUT_QUAD,    AV_CH_LAYOUT_5POINT0, AV_CH_LAYOUT_5POINT1,
                                                     AV_CH_LAYOUT_6POINT1, AV_CH_LAYOUT_7POINT1};
const std::map<int32_t, int32_t> BITS_PER_RAW_SAMPLE_MAP = {
    {OHOS::Media::AudioSampleFormat::SAMPLE_S16LE, 16},
    {OHOS::Media::AudioSampleFormat::SAMPLE_S24LE, 24},
    {OHOS::Media::AudioSampleFormat::SAMPLE_S32LE, 32},
};
} // namespace

namespace OHOS {
namespace Media {
constexpr std::string_view AUDIO_CODEC_NAME = "flac";
AudioFFMpegFlacEncoderPlugin::AudioFFMpegFlacEncoderPlugin() : basePlugin(std::make_unique<AudioFfmpegEncoderPlugin>())
{
    channels = 0;
}

AudioFFMpegFlacEncoderPlugin::~AudioFFMpegFlacEncoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

static bool CheckSampleRate(int32_t sampleRate)
{
    for (auto i : FLAC_ENCODER_SAMPLE_RATE_TABLE) {
        if (i == sampleRate) {
            return true;
        }
    }
    return false;
}

static bool CheckChannelLayout(uint64_t channelLayout)
{
    for (auto i : FLAC_CHANNEL_LAYOUT_TABLE) {
        if (i == channelLayout) {
            return true;
        }
    }
    return false;
}

static bool CheckBitsPerSample(int32_t bitsPerCodedSample)
{
    for (auto &i : BITS_PER_RAW_SAMPLE_MAP) {
        if (i.first == bitsPerCodedSample) {
            return true;
        }
    }
    return false;
}

int32_t AudioFFMpegFlacEncoderPlugin::SetContext(const Format &format)
{
    int32_t complianceLevel;
    int32_t bitsPerCodedSample;
    auto avCodecContext = basePlugin->GetCodecContext();
    format.GetIntValue(MediaDescriptionKey::MD_KEY_COMPLIANCE_LEVEL, complianceLevel);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE, bitsPerCodedSample);
    avCodecContext->strict_std_compliance = complianceLevel;
    if (BITS_PER_RAW_SAMPLE_MAP.find(bitsPerCodedSample) == BITS_PER_RAW_SAMPLE_MAP.end()) {
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    }
    avCodecContext->bits_per_raw_sample = BITS_PER_RAW_SAMPLE_MAP.at(bitsPerCodedSample);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegFlacEncoderPlugin::CheckFormat(const Format &format)
{
    int32_t channelCount;
    int32_t sampleRate;
    int32_t bitsPerCodedSample;
    int32_t complianceLevel;
    int64_t channelLayout;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, channelCount);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, sampleRate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_BITS_PER_CODED_SAMPLE, bitsPerCodedSample);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_COMPLIANCE_LEVEL, complianceLevel);
    format.GetLongValue(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT, channelLayout);
    auto ffChannelLayout =
        FFMpegConverter::ConvertOHAudioChannelLayoutToFFMpeg(static_cast<AudioChannelLayout>(channelLayout));

    if (!CheckSampleRate(sampleRate)) {
        AVCODEC_LOGE("init failed, because sampleRate=%{public}d not in table.", sampleRate);
        return AVCodecServiceErrCode::AVCS_ERR_MISMATCH_SAMPLE_RATE;
    } else if (channelCount < MIN_CHANNELS || channelCount > MAX_CHANNELS) {
        AVCODEC_LOGE("init failed, because channelCount=%{public}d not support.", channelCount);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT;
    } else if (!CheckBitsPerSample(bitsPerCodedSample)) {
        AVCODEC_LOGE("init failed, because bitsPerCodedSample=%{public}d not support.", bitsPerCodedSample);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    } else if (complianceLevel < MIN_COMPLIANCE_LEVEL || complianceLevel > MAX_COMPLIANCE_LEVEL) {
        AVCODEC_LOGE("init failed, because complianceLevel=%{public}d not support.", complianceLevel);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    } else if (!CheckChannelLayout(ffChannelLayout)) {
        AVCODEC_LOGE("init failed, because ffChannelLayout=%{public}" PRId64 "not support.", ffChannelLayout);
        return AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_ERROR;
    }
    channels = channelCount;
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

    ret = SetContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("init failed, because SetContext failed. ret=%{public}d", ret);
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
    int32_t inputBufferSize = SAMPLES * channels * sizeof(short);
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > inputBufferSize) {
        maxSize = inputBufferSize;
    }
    return maxSize;
}

int32_t AudioFFMpegFlacEncoderPlugin::GetOutputBufferSize() const
{
    int32_t outputBufferSize = SAMPLES * channels * sizeof(short);
    return outputBufferSize;
}

Format AudioFFMpegFlacEncoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_FLAC);
    return format;
}

std::string_view AudioFFMpegFlacEncoderPlugin::GetCodecType() const noexcept
{
    return AUDIO_CODEC_NAME;
}
} // namespace Media
} // namespace OHOS