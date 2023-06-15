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

#include "audio_ffmpeg_encoder_plugin.h"
#include "avcodec_errors.h"
#include "media_description.h"
#include "avcodec_dfx.h"
#include "avcodec_log.h"
#include "securec.h"
#include "ffmpeg_converter.h"
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioFFMpegEncoderPlugin"};
}

namespace OHOS {
namespace Media {
AudioFfmpegEncoderPlugin::AudioFfmpegEncoderPlugin()
    : maxInputSize_(-1), avCodec_(nullptr), avCodecContext_(nullptr), cachedFrame_(nullptr), avPacket_(nullptr),
      prevPts_(0)
{
}

AudioFfmpegEncoderPlugin::~AudioFfmpegEncoderPlugin()
{
    CloseCtxLocked();
    avCodecContext_.reset();
}

int32_t AudioFfmpegEncoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    if (avCodecContext_ == nullptr) {
        AVCODEC_LOGE("avCodecContext_ is nullptr");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION;
    }
    std::unique_lock lock(avMutext_);
    return SendBuffer(inputBuffer);
}

static std::string AVStrError(int errnum)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
    return std::string(errbuf);
}

int32_t AudioFfmpegEncoderPlugin::PcmFillFrame(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    auto memory = inputBuffer->GetBuffer();
    const uint8_t *ptr = memory->GetBase();
    auto bytesPerSample = av_get_bytes_per_sample(avCodecContext_->sample_fmt);
    if (!av_sample_fmt_is_planar(avCodecContext_->sample_fmt)) {
        auto ret = av_samples_fill_arrays(cachedFrame_->data, cachedFrame_->linesize, ptr, cachedFrame_->channels,
                                          cachedFrame_->nb_samples, (AVSampleFormat)cachedFrame_->format, 0);
        if (ret < 0) {
            AVCODEC_LOGE("Samples fill arrays failed: %{public}s", AVStrError(ret).c_str());
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    }
    for (int i = 0; i < cachedFrame_->nb_samples; i++) {
        for (int j = 0; j < cachedFrame_->channels; j++) {
            auto ret = memcpy_s((void *)(&cachedFrame_->data[j][i * bytesPerSample]), bytesPerSample,
                                (void *)(&ptr[i * cachedFrame_->channels * bytesPerSample + bytesPerSample * j]),
                                bytesPerSample);
            if (ret != EOK) {
                AVCODEC_LOGE("memory copy failed, errno: %{public}d", ret);
                return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
            }
        }
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::SendBuffer(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    if (!inputBuffer) {
        AVCODEC_LOGE("inputBuffer is nullptr");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    int ret = av_frame_make_writable(cachedFrame_.get());
    if (ret != 0) {
        AVCODEC_LOGE("Frame make writable failed: %{public}s", AVStrError(ret).c_str());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (!inputBuffer->CheckIsEos()) {
        auto errCode = PcmFillFrame(inputBuffer);
        if (errCode != AVCodecServiceErrCode::AVCS_ERR_OK) {
            return errCode;
        }
        ret = avcodec_send_frame(avCodecContext_.get(), cachedFrame_.get());
    } else {
        ret = avcodec_send_frame(avCodecContext_.get(), nullptr);
    }
    if (ret == 0) {
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    } else if (ret == AVERROR(EAGAIN)) {
        AVCODEC_LOGW("skip this frame because data not enough, msg:%{public}s", AVStrError(ret).data());
        return AVCodecServiceErrCode::AVCS_ERR_NOT_ENOUGH_DATA;
    } else if (ret == AVERROR_EOF) {
        AVCODEC_LOGW("eos send frame, msg:%{public}s", AVStrError(ret).data());
        return AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM;
    } else {
        AVCODEC_LOGE("Send frame unknown error: %{public}s", AVStrError(ret).c_str());
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
}

int32_t AudioFfmpegEncoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    if (!outBuffer) {
        AVCODEC_LOGE("outBuffer is nullptr");
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    int32_t status;
    {
        std::unique_lock l(avMutext_);
        if (avCodecContext_ == nullptr) {
            AVCODEC_LOGE("avCodecContext_ is nullptr");
            return AVCodecServiceErrCode::AVCS_ERR_INVALID_OPERATION;
        }
        status = ReceiveBuffer(outBuffer);
    }
    return status;
}

int32_t AudioFfmpegEncoderPlugin::ReceiveBuffer(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    auto ret = avcodec_receive_packet(avCodecContext_.get(), avPacket_.get());
    int32_t status;
    if (ret >= 0) {
        AVCODEC_LOGD("receive one packet");
        status = ReceivePacketSucc(outBuffer);
    } else if (ret == AVERROR_EOF) {
        outBuffer->SetEos(true);
        avcodec_flush_buffers(avCodecContext_.get());
        status = AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM;
    } else if (ret == AVERROR(EAGAIN)) {
        status = AVCodecServiceErrCode::AVCS_ERR_NOT_ENOUGH_DATA;
    } else {
        AVCODEC_LOGE("audio encoder receive unknow error: %{public}s", AVStrError(ret).c_str());
        status = AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return status;
}

int32_t AudioFfmpegEncoderPlugin::ReceivePacketSucc(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    uint32_t headerSize = 0;
    auto memory = outBuffer->GetBuffer();
    if (headerFuncValid_) {
        std::string header;
        GetHeaderFunc_(header, headerSize, avCodecContext_, avPacket_->size);
        if (headerSize == 0) {
            AVCODEC_LOGE("Get header failed.");
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
        uint32_t len = memory->Write(reinterpret_cast<uint8_t *>(const_cast<char *>(header.c_str())), headerSize);
        if (len < headerSize) {
            AVCODEC_LOGE("Write header failed, len = %{public}d", len);
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
    }

    int32_t outputSize = avPacket_->size + headerSize;
    if (memory->GetSize() < outputSize) {
        AVCODEC_LOGW("Output buffer capacity is not enough");
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    auto len = memory->Write(avPacket_->data, avPacket_->size);
    if (len < avPacket_->size) {
        AVCODEC_LOGE("write packet data failed, len = %{public}d", len);
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    auto attr = outBuffer->GetBufferAttr();
    attr.size = avPacket_->size + headerSize;
    prevPts_ += avPacket_->duration;
    attr.presentationTimeUs = FFMpegConverter::ConvertAudioPtsToUs(prevPts_, avCodecContext_->time_base);
    outBuffer->SetBufferAttr(attr);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::Reset()
{
    auto ret = CloseCtxLocked();
    avCodecContext_.reset();
    prevPts_ = 0;
    return ret;
}

int32_t AudioFfmpegEncoderPlugin::Release()
{
    std::unique_lock lock(avMutext_);
    auto ret = CloseCtxLocked();
    avCodecContext_.reset();
    return ret;
}

int32_t AudioFfmpegEncoderPlugin::Flush()
{
    std::unique_lock lock(avMutext_);
    if (avCodecContext_ != nullptr) {
        avcodec_flush_buffers(avCodecContext_.get());
    }
    prevPts_ = 0;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::AllocateContext(const std::string &name)
{
    {
        std::unique_lock lock(avMutext_);
        avCodec_ = std::shared_ptr<AVCodec>(const_cast<AVCodec *>(avcodec_find_encoder_by_name(name.c_str())),
                                            [](AVCodec *ptr) {});
        cachedFrame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame *fp) { av_frame_free(&fp); });
        avPacket_ = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket *ptr) { av_packet_free(&ptr); });
    }
    if (avCodec_ == nullptr) {
        return AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_PROTOCOL_TYPE;
    }

    AVCodecContext *context = nullptr;
    {
        std::unique_lock lock(avMutext_);
        context = avcodec_alloc_context3(avCodec_.get());
        avCodecContext_ = std::shared_ptr<AVCodecContext>(context, [](AVCodecContext *ptr) {
            avcodec_free_context(&ptr);
            avcodec_close(ptr);
        });
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::InitContext(const Format &format)
{
    format_ = format;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, avCodecContext_->channels);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, avCodecContext_->sample_rate);
    format.GetLongValue(MediaDescriptionKey::MD_KEY_BITRATE, avCodecContext_->bit_rate);
    format.GetIntValue(MediaDescriptionKey::MD_KEY_MAX_INPUT_SIZE, maxInputSize_);

    int64_t channelLayout;
    format.GetLongValue(MediaDescriptionKey::MD_KEY_CHANNEL_LAYOUT, channelLayout);
    auto ffChannelLayout =
        FFMpegConverter::ConvertOHAudioChannelLayoutToFFMpeg(static_cast<AudioChannelLayout>(channelLayout));
    if (ffChannelLayout == AV_CH_LAYOUT_NATIVE) {
        AVCODEC_LOGE("InitContext failed, because ffChannelLayout is AV_CH_LAYOUT_NATIVE");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    avCodecContext_->channel_layout = ffChannelLayout;

    int32_t sampleFormat;
    format.GetIntValue(MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT, sampleFormat);
    auto ffSampleFormat = FFMpegConverter::ConvertOHAudioFormatToFFMpeg(static_cast<AudioSampleFormat>(sampleFormat));
    if (ffSampleFormat == AV_SAMPLE_FMT_NONE) {
        AVCODEC_LOGE("InitContext failed, because avSampleFormat is AV_SAMPLE_FMT_NONE");
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    avCodecContext_->sample_fmt = ffSampleFormat;

    AVCODEC_LOGI("avcodec name: %{public}s", avCodec_->name);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::OpenContext()
{
    {
        std::unique_lock lock(avMutext_);
        auto res = avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr);
        if (res != 0) {
            AVCODEC_LOGE("avcodec open error %{public}s", AVStrError(res).c_str());
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
    }
    if (avCodecContext_->frame_size <= 0) {
        AVCODEC_LOGE("frame size invalid");
    }
    format_.PutIntValue(MediaDescriptionKey::MD_KEY_AUDIO_SAMPLES_PER_FRAME, avCodecContext_->frame_size);
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::InitFrame()
{
    cachedFrame_->nb_samples = avCodecContext_->frame_size;
    cachedFrame_->format = avCodecContext_->sample_fmt;
    cachedFrame_->channel_layout = avCodecContext_->channel_layout;
    cachedFrame_->channels = avCodecContext_->channels;
    int ret = av_frame_get_buffer(cachedFrame_.get(), 0);
    if (ret < 0) {
        AVCODEC_LOGE("Get frame buffer failed: %{public}s", AVStrError(ret).c_str());
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

Format AudioFfmpegEncoderPlugin::GetFormat() const noexcept
{
    return format_;
}

std::shared_ptr<AVCodecContext> AudioFfmpegEncoderPlugin::GetCodecContext() const
{
    return avCodecContext_;
}

std::shared_ptr<AVPacket> AudioFfmpegEncoderPlugin::GetCodecAVPacket() const
{
    return avPacket_;
}

std::shared_ptr<AVFrame> AudioFfmpegEncoderPlugin::GetCodecCacheFrame() const
{
    return cachedFrame_;
}

std::shared_ptr<AVCodec> AudioFfmpegEncoderPlugin::GetAVCodec() const
{
    return avCodec_;
}

int32_t AudioFfmpegEncoderPlugin::GetMaxInputSize() const noexcept
{
    return maxInputSize_;
}

int32_t AudioFfmpegEncoderPlugin::CloseCtxLocked()
{
    if (avCodecContext_ != nullptr) {
        auto res = avcodec_close(avCodecContext_.get());
        if (res != 0) {
            AVCODEC_LOGE("avcodec close failed: %{public}s", AVStrError(res).c_str());
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

void AudioFfmpegEncoderPlugin::RegisterHeaderFunc(HeaderFunc headerFunc)
{
    GetHeaderFunc_ = headerFunc;
    headerFuncValid_ = true;
}
} // namespace Media
} // namespace OHOS