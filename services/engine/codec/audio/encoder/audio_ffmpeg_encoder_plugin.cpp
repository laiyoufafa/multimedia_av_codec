#include "audio_ffmpeg_encoder_plugin.h"
#include "avcodec_errors.h"
#include <iostream>

namespace OHOS {
namespace Media {

const std::string CHANNEL_COUNT_KEY{"channel-count"};
const std::string SAMPLE_RATE_KEY{"sample-rate"};
const std::string BITS_RATE_KEY{"bit-rate"};
const std::string BITS_PER_CODED_SAMPLE_KEY{"bits_per_coded_sample"};
const std::string SAMPLE_FORMAT_KEY{"sample-format"};
const std::string CHANNEL_LAYOUT_KEY{"channel-layout"};

AudioFfmpegEncoderPlugin::AudioFfmpegEncoderPlugin() {}

AudioFfmpegEncoderPlugin::~AudioFfmpegEncoderPlugin() {
    CloseCtxLocked();
    avCodecContext_.reset();
    avCodecContext_ = nullptr;
}

int32_t AudioFfmpegEncoderPlugin::ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer) {
    int32_t ret = AVCodecServiceErrCode::AVCS_ERR_OK;
    {
        std::unique_lock lock(avMutext_);
        if (avCodecContext_ == nullptr) {
            return AVCodecServiceErrCode::AVCS_ERR_WRONG_STATE;
        }
        ret = sendBuffer(inputBuffer);
    }
    return ret;
}

int32_t AudioFfmpegEncoderPlugin::PcmFillFrame(const std::shared_ptr<AudioBufferInfo> &inputBuffer)
{
    // auto attr = inputBuffer->GetBufferAttr();
    // 输入的pcm数据应该包含数据属性吧，s16，f32, f32p etc,
    // 不符合编码器输入要求的话需要转换
    auto memory = inputBuffer->GetBuffer();
    const uint8_t *ptr = memory->GetReadOnlyData();
    auto bytesPerSample = av_get_bytes_per_sample(avCodecContext_->sample_fmt);
    if (av_sample_fmt_is_planar(avCodecContext_->sample_fmt)) {
        for (int i = 0; i < cachedFrame_->nb_samples; i++) {
            for (int j = 0; j < cachedFrame_->channels; j++) {
                memcpy((void *)(&cachedFrame_->data[j][i * bytesPerSample]),
                       (void *)(&ptr[i * cachedFrame_->channels * bytesPerSample + bytesPerSample * j]),
                       bytesPerSample);
            }
        }
    } else { // aac需要fltp(f32p)格式的数据，不走这个分支，packed数据还没验证过
        auto ret = av_samples_fill_arrays(cachedFrame_->data, cachedFrame_->linesize, ptr, cachedFrame_->channels,
                                     cachedFrame_->nb_samples, (AVSampleFormat)cachedFrame_->format, 0);
        if (ret < 0) {
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::sendBuffer(const std::shared_ptr<AudioBufferInfo> &inputBuffer) {
    std::cout << "begin: " << __func__ << std::endl;
    if (!inputBuffer) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    int ret = av_frame_make_writable(cachedFrame_.get());
    if (ret != 0) {
        std::cout << "make writable failed" << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }

    if (!inputBuffer->CheckIsEos()) {
        // 填充AVFrame还有什么简单的方式不？？
        // 输入的数据默认是FLTP(f32p)
        // 输入解交织，有没有现成的接口嘞？？？
        auto errCode = PcmFillFrame(inputBuffer);
        if (errCode != AVCodecServiceErrCode::AVCS_ERR_OK) {
            return errCode;
        }
        ret = avcodec_send_frame(avCodecContext_.get(), cachedFrame_.get());
    } else {
        ret = avcodec_send_frame(avCodecContext_.get(), nullptr);
        std::cout << "send eof frame +++++++++++++++++++++++++++++ ret = " << ret << std::endl;
    }
    if (ret == 0) {
        return AVCodecServiceErrCode::AVCS_ERR_OK;
    } else if (ret == AVERROR(EAGAIN)) {
        return AVCodecServiceErrCode::AVCS_ERR_AGAIN;
    } else if (ret == AVERROR_EOF) { // AVStrError(ret).c_str() == "End of file"
        return AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM;
    } else {
        std::cout << "send buffer error:" << ret << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
}

int32_t AudioFfmpegEncoderPlugin::ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer) {
    if (!outBuffer) {
        return AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL;
    }
    int32_t status;
    {
        std::unique_lock l(avMutext_);
        if (avCodecContext_ == nullptr) {
            return AVCodecServiceErrCode::AVCS_ERR_WRONG_STATE;
        }
        status = receiveBuffer(outBuffer);
    }
    return status;
}

int32_t AudioFfmpegEncoderPlugin::receiveBuffer(std::shared_ptr<AudioBufferInfo> &outBuffer) {
    auto ret = avcodec_receive_packet(avCodecContext_.get(), avPacket_.get());
    std::cout << "receive packet ret = " << ret << " packet size = " << avPacket_->size << std::endl;
    int32_t status;
    if (ret >= 0) {
        status = ReceivePacketSucc(outBuffer);
    } else if (ret == AVERROR_EOF) { // 最后一次send之后有两次有效receive
        std::cout << "receive eof packet eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
        outBuffer->SetEos(true);
        avcodec_flush_buffers(avCodecContext_.get());
        status = AVCodecServiceErrCode::AVCS_ERR_END_OF_STREAM;
    } else if (ret == AVERROR(EAGAIN)) {
        status = AVCodecServiceErrCode::AVCS_ERR_NOT_ENOUGH_DATA;
    } else {
        status = AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return status;
}

int32_t AudioFfmpegEncoderPlugin::ReceivePacketSucc(std::shared_ptr<AudioBufferInfo> &outBuffer)
{
    auto ioInfoMem = outBuffer->GetBuffer();
    uint32_t headerSize = 0;
    if (headerFuncValid_) {
        std::string header;
        GetHeaderFunc_(header, headerSize, avCodecContext_, avPacket_->size);
        ioInfoMem->Write((uint8_t*)header.c_str(), headerSize);
    }

    int32_t outputSize = avPacket_->size + headerSize;
    if (ioInfoMem->GetSize() < outputSize) {
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }

    ioInfoMem->Write(avPacket_->data, avPacket_->size);
    auto attr = outBuffer->GetBufferAttr();
    attr.size = avPacket_->size + headerSize;
    outBuffer->SetBufferAttr(attr);

    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::Reset() {
    auto ret = CloseCtxLocked();
    avCodecContext_.reset();
    avCodecContext_ = nullptr;
    return ret;
}

int32_t AudioFfmpegEncoderPlugin::Release() {
    std::unique_lock lock(avMutext_);
    auto ret = CloseCtxLocked();
    avCodecContext_.reset();
    avCodecContext_ = nullptr;
    return ret;
}

int32_t AudioFfmpegEncoderPlugin::Flush() {
    std::unique_lock lock(avMutext_);
    if (avCodecContext_ != nullptr) {
        avcodec_flush_buffers(avCodecContext_.get());
    }
    // MEDIA_LOG_I("Flush exit.");
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::AllocateContext(const std::string &name) {
    {
        std::unique_lock lock(avMutext_);
        avCodec_ = std::shared_ptr<AVCodec>(const_cast<AVCodec *>(avcodec_find_encoder_by_name(name.c_str())),
                                            [](AVCodec *ptr) {});
        cachedFrame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame *fp) { av_frame_free(&fp); });
        avPacket_ = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket *ptr) { av_packet_free(&ptr); });
        // todo: 初始化不加锁行不行?
    }
    if (avCodec_ == nullptr) {
        return AVCodecServiceErrCode::AVCS_ERR_UNSUPPORT_PROTOCOL_TYPE;
    }

    AVCodecContext *context = nullptr;
    {
        std::unique_lock lock(avMutext_);
        // FALSE_RETURN_V(avCodec_ != nullptr, Status::ERROR_WRONG_STATE);
        context = avcodec_alloc_context3(avCodec_.get());

        avCodecContext_ = std::shared_ptr<AVCodecContext>(context, [](AVCodecContext *ptr) {
            avcodec_free_context(&ptr);
            avcodec_close(ptr);
        });
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::InitContext(const Format &format) {
    /**
     * todo: 
     *      1. input pcm sample_format should be checked, if not supported, return error
     *      2. target sample format shoule be checked, if not supported, should return error
    */
    format.GetIntValue(CHANNEL_COUNT_KEY, avCodecContext_->channels);
    format.GetIntValue(SAMPLE_RATE_KEY, avCodecContext_->sample_rate);
    format.GetLongValue(BITS_RATE_KEY, avCodecContext_->bit_rate);
    format.GetIntValue(BITS_PER_CODED_SAMPLE_KEY, avCodecContext_->bits_per_coded_sample);
    int64_t layout;
    format.GetLongValue(CHANNEL_LAYOUT_KEY, layout);
    avCodecContext_->channel_layout = layout; // layout
    int32_t sampleFormat;
    format.GetIntValue(SAMPLE_FORMAT_KEY, sampleFormat); // for aac is AV_SMAPLE_FORMAT_FLTP
    avCodecContext_->sample_fmt = (AVSampleFormat)sampleFormat;
    avCodecContext_->request_sample_fmt = avCodecContext_->sample_fmt;
    avCodecContext_->workaround_bugs =
        static_cast<uint32_t>(avCodecContext_->workaround_bugs) | static_cast<uint32_t>(FF_BUG_AUTODETECT);
    avCodecContext_->err_recognition = 1;
    avCodecContext_->profile = FF_PROFILE_AAC_LOW;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

int32_t AudioFfmpegEncoderPlugin::OpenContext() {
    std::cout << "AudioFfmpegEncoderPlugin::OpenContext" << std::endl;
    {
        std::unique_lock lock(avMutext_);
        auto res = avcodec_open2(avCodecContext_.get(), avCodec_.get(), nullptr);
        if (res != 0) {
            // MEDIA_LOG_E("avcodec open error " PUBLIC_LOG_S, AVStrError(res).c_str());
            return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
        }
    }
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
        std::cout << "get buffer failed. ret = " << ret << std::endl;
        return AVCodecServiceErrCode::AVCS_ERR_NO_MEMORY;
    }
    std::cout << "av frame get buffer succeeded" << std::endl;
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

Format AudioFfmpegEncoderPlugin::GetFormat() const noexcept {
    return format_;
}

std::shared_ptr<AVCodecContext> AudioFfmpegEncoderPlugin::GetCodecContext() const {
    return avCodecContext_;
}

std::shared_ptr<AVPacket> AudioFfmpegEncoderPlugin::GetCodecAVPacket() const {
    return avPacket_;
}

std::shared_ptr<AVFrame> AudioFfmpegEncoderPlugin::GetCodecCacheFrame() const {
    return cachedFrame_;
}

std::shared_ptr<AVCodec> AudioFfmpegEncoderPlugin::GetAVCodec() const
{
    return avCodec_;
}

int32_t AudioFfmpegEncoderPlugin::CloseCtxLocked() {
    if (avCodecContext_ != nullptr) {
        auto res = avcodec_close(avCodecContext_.get());
        if (res != 0) {
            // MEDIA_LOG_E("avcodec close error " PUBLIC_LOG_S, AVStrError(res).c_str());
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