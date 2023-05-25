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

#include "audio_ffmpeg_aac_decoder_plugin.h"
#include "avcodec_dfx.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "media_description.h"
#include "avcodec_mime_type.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegAacDecoderPlugin"};
}

namespace OHOS {
namespace Media {
static constexpr uint32_t INPUT_BUFFER_SIZE_DEFAULT = 8192;
static constexpr uint32_t OUTPUT_BUFFER_SIZE_DEFAULT = 4 * 1024 * 8;

AudioFFMpegAacDecoderPlugin::AudioFFMpegAacDecoderPlugin() : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>()) {}

AudioFFMpegAacDecoderPlugin::~AudioFFMpegAacDecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

int32_t AudioFFMpegAacDecoderPlugin::init(const Format &format)
{
    int type;
    if (!format.GetIntValue(MediaDescriptionKey::MD_KEY_AAC_IS_ADTS, type)) {
        AVCODEC_LOGE("aac_is_adts parameter is missing");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    if (type != 1 && type != 0) {
        AVCODEC_LOGE("aac_is_adts value invalid");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    std::string aacName = (type == 1 ? "aac" : "aac_latm");
    int32_t ret = basePlugin->AllocateContext(aacName);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("AllocateContext failed, ret=%{public}d", ret);
        return ret;
    }
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("InitContext failed, ret=%{public}d", ret);
        return ret;
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegAacDecoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegAacDecoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegAacDecoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegAacDecoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegAacDecoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegAacDecoderPlugin::getInputBufferSize() const
{
    int32_t maxSize = basePlugin->GetMaxInputSize();
    if (maxSize < 0 || maxSize > INPUT_BUFFER_SIZE_DEFAULT) {
        maxSize = INPUT_BUFFER_SIZE_DEFAULT;
    }
    return maxSize;
}

uint32_t AudioFFMpegAacDecoderPlugin::getOutputBufferSize() const
{
    return OUTPUT_BUFFER_SIZE_DEFAULT;
}

Format AudioFFMpegAacDecoderPlugin::GetFormat() const noexcept
{
    auto format = basePlugin->GetFormat();
    format.PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, AVCodecMimeType::MEDIA_MIMETYPE_AUDIO_AAC);
    return format;
}
} // namespace Media
} // namespace OHOS