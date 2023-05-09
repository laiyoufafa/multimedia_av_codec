#ifndef AUDIO_FFMPEG_ENCODER_PLUGIN
#define AUDIO_FFMPEG_ENCODER_PLUGIN

#include "audio_ffmpeg_base_codec.h"
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include <libavutil/opt.h>
#ifdef __cplusplus
};
#endif

namespace OHOS {
namespace Media {
class AudioFfmpegEncoderPlugin {
private:
    std::shared_ptr<AVCodec> avCodec_{};
    std::shared_ptr<AVCodecContext> avCodecContext_{};
    std::shared_ptr<AVFrame> cachedFrame_{};
    std::shared_ptr<AVPacket> avPacket_{};
    mutable std::mutex avMutext_{};
    std::mutex parameterMutex_{};
    Format format_;
    using HeaderFunc = std::function<int32_t(std::string &header, uint32_t &headerSize, std::shared_ptr<AVCodecContext>,
                                             uint32_t dataLength)>;

public:
    AudioFfmpegEncoderPlugin();
    ~AudioFfmpegEncoderPlugin();
    int32_t ProcessSendData(const std::shared_ptr<AudioBufferInfo> &inputBuffer);
    int32_t ProcessRecieveData(std::shared_ptr<AudioBufferInfo> &outBuffer);
    int32_t Reset();
    int32_t Release();
    int32_t Flush();

    int32_t AllocateContext(const std::string &name);
    int32_t InitContext(const Format &format);
    int32_t OpenContext();
    Format GetFormat() const noexcept;
    int32_t InitFrame();

    std::shared_ptr<AVCodecContext> GetCodecContext() const;
    std::shared_ptr<AVPacket> GetCodecAVPacket() const;
    std::shared_ptr<AVFrame> GetCodecCacheFrame() const;
    std::shared_ptr<AVCodec> GetAVCodec() const;

    void RegisterHeaderFunc(HeaderFunc headerFunc);

    int32_t CloseCtxLocked();

private:
    int32_t sendBuffer(const std::shared_ptr<AudioBufferInfo> &inputBuffer);
    int32_t receiveBuffer(std::shared_ptr<AudioBufferInfo> &outBuffer);
    int32_t ReceivePacketSucc(std::shared_ptr<AudioBufferInfo> &outBuffer);
    int32_t PcmFillFrame(const std::shared_ptr<AudioBufferInfo> &inputBuffer);
    HeaderFunc GetHeaderFunc_;
    bool headerFuncValid_ = false;
};

} // namespace Media
} // namespace OHOS

#endif