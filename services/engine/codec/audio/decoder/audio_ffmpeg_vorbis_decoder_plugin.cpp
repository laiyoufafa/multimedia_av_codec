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

#include "audio_ffmpeg_vorbis_decoder_plugin.h"
#include "avcodec_errors.h"
#include "securec.h"

namespace OHOS {
namespace Media {

AudioFFMpegVorbisDecoderPlugin::AudioFFMpegVorbisDecoderPlugin()
    : basePlugin(std::make_unique<AudioFfmpegDecoderPlugin>())
{
}

AudioFFMpegVorbisDecoderPlugin::~AudioFFMpegVorbisDecoderPlugin()
{
    basePlugin->Release();
    basePlugin.reset();
    basePlugin = nullptr;
}

std::shared_ptr<AVCodecContext> AudioFFMpegVorbisDecoderPlugin::GenEncodeContext(const Format &format)
{
    auto encodec = avcodec_find_encoder_by_name("libvorbis");
    if (encodec == nullptr) {
        return nullptr;
    }
    auto context = avcodec_alloc_context3(encodec);
    if (context == nullptr) {
        return nullptr;
    }
    auto encodeContext =
        std::shared_ptr<AVCodecContext>(context, [](AVCodecContext *ptr) { avcodec_free_context(&ptr); });
    if (encodeContext == nullptr) {
        // std::cout << "null pointer." << std::endl;
        return nullptr;
    }
    encodeContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
    format.GetIntValue("channel-count", encodeContext->channels); // todo: 统一KEY定义
    format.GetIntValue("sample-rate", encodeContext->sample_rate);

    int ret = avcodec_open2(encodeContext.get(), encodec, nullptr);
    if (ret != 0) {
        // std::cout << "open encoder failed" << std::endl;
        return nullptr;
    }
    return encodeContext;
}

int32_t AudioFFMpegVorbisDecoderPlugin::AssignExtradata(std::shared_ptr<AVCodecContext> &context, const Format &format)
{
    if (context == nullptr) {
        // std::cout << " null pointer" << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    auto encodeContext = GenEncodeContext(format);
    if (encodeContext == nullptr) {
        // std::cout << "Gen encode context failed." << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    context->extradata_size = encodeContext->extradata_size;
    context->extradata = static_cast<uint8_t *>(av_mallocz(encodeContext->extradata_size));
    if (context->extradata == nullptr) {
        // std::cout << "Alloc memory failed." << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }
    int ret =
        memcpy_s(context->extradata, context->extradata_size, encodeContext->extradata, encodeContext->extradata_size);
    if (ret != 0) {
        // std::cout << "Copy memory failed." << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFFMpegVorbisDecoderPlugin::init(const Format &format)
{
    int32_t ret = basePlugin->AllocateContext("vorbis");
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        // std::cout << "init 1 OH error:" << ret << "\n";
        return ret;
    }
    auto codecCtx = basePlugin->GetCodecContext();
    ret = AssignExtradata(codecCtx, format);
    ret = basePlugin->InitContext(format);
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        // std::cout << "init 2 OH error:" << ret << "\n";
        return ret;
    }
    return basePlugin->OpenContext();
}

int32_t AudioFFMpegVorbisDecoderPlugin::processSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    return basePlugin->ProcessSendData(inputBuffer);
}

int32_t AudioFFMpegVorbisDecoderPlugin::processRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    return basePlugin->ProcessRecieveData(outBuffer);
}

int32_t AudioFFMpegVorbisDecoderPlugin::reset()
{
    return basePlugin->Reset();
}

int32_t AudioFFMpegVorbisDecoderPlugin::release()
{
    return basePlugin->Release();
}

int32_t AudioFFMpegVorbisDecoderPlugin::flush()
{
    return basePlugin->Flush();
}

uint32_t AudioFFMpegVorbisDecoderPlugin::getInputBufferSize() const
{
    return 8192;
}

uint32_t AudioFFMpegVorbisDecoderPlugin::getOutputBufferSize() const
{
    return 2 * 4096 * 2;
}

Format AudioFFMpegVorbisDecoderPlugin::GetFormat() const noexcept
{
    return basePlugin->GetFormat();
}

} // namespace Media
} // namespace OHOS