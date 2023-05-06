#ifndef FCODEC_H
#define FCODEC_H

#include "avcodec_common.h" //AVCodecBufferInfo & callback
#include "avcodec_errors.h" //Errorcode
#include "codec_utils.h"
#include "codecbase.h"
#include "share_memory.h"
#include "surface_memory.h"
#include "task_thread.h"
#include "avcodec_info.h"
#include <any>
#include <atomic>
#include <list>
#include <map>
#include <shared_mutex>
#include <tuple>
#include <vector>

namespace OHOS {
namespace Media {
namespace Codec {
class FCodec : public CodecBase {
public:
    explicit FCodec() = default;
    explicit FCodec(const std::string &name);
    explicit FCodec(bool isEncoder, const std::string &mime);
    ~FCodec() override;
    int32_t Configure(const Format &format) override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Flush() override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t SetParameter(const Format &format) override;
    int32_t GetOutputFormat(Format &format) override;
    std::shared_ptr<AVSharedMemory> GetInputBuffer(size_t index) override;
    int32_t QueueInputBuffer(size_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;
    std::shared_ptr<AVSharedMemory> GetOutputBuffer(size_t index) override;
    int32_t ReleaseOutputBuffer(size_t index) override;
    int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) override;
    int32_t SetOutputSurface(sptr<Surface> surface) override;
    int32_t RenderOutputBuffer(size_t index) override;
    int32_t Pause() override;
    int32_t Resume() override;
    int32_t getCodecCapability(std::vector<CapabilityData> &capaArray);

    struct AVBuffer {
    public:
        AVBuffer() = default;
        ~AVBuffer() = default;

        enum status {
            OWNED_BY_CODEC,
            OWNED_BY_USER,
            OWNED_BY_SURFACE,
        };

        std::shared_ptr<AVSharedMemory> memory_;
        std::atomic<status> owner_;
        AVCodecBufferInfo bufferInfo_;
        AVCodecBufferFlag bufferFlag_;
    };

private:
    int32_t Init(const std::string &name);

    enum struct State : int32_t {
        Uninitialized,
        Initialized,
        Configured,
        Running,
        Flushing,
        Flushed,
        Stopping,
        Releasing,
        EOS,
        Error,
    };

    template <typename T>
    void GetParameter(Tag tag, T &val);
    bool IsActive() const;
    void ResetContext(bool isFlush = false);
    std::tuple<uint32_t, uint32_t> CalculateBufferSize();
    int32_t AllocateBuffers();
    int32_t ReleaseBuffers(bool isFlush = false);
    void sendFrame();
    void receiveFrame();
    void renderFrame();
    int32_t FillFrameBuffer(const std::shared_ptr<AVBuffer> &frameBuffer);
    int32_t UpdateSurfaceMemory(std::shared_ptr<SurfaceMemory> &surfaceMemory, int64_t pts);

    std::string codecName_;
    std::atomic<State> state_{State::Uninitialized};
    uint32_t width_{0};
    uint32_t height_{0};
    std::map<Tag, std::any> decParams_;
    bool isUpTodate_{false};
    // INIT
    std::shared_ptr<AVCodec> avCodec_{nullptr};
    // Config
    std::shared_ptr<AVCodecContext> avCodecContext_{nullptr};
    // Start
    std::shared_ptr<AVPacket> avPacket_{nullptr};
    std::shared_ptr<AVFrame> cachedFrame_{nullptr};
    // Receive frame
    uint8_t *scaleData_[AV_NUM_DATA_POINTERS];
    int32_t scaleLineSize_[AV_NUM_DATA_POINTERS];
    std::shared_ptr<Scale> scale_{nullptr};
    bool isConverted_{false};
    // Running
    std::vector<std::shared_ptr<AVBuffer>> buffers_[2];
    std::list<size_t> codecAvailBuffers_; // 保留
    std::list<size_t> renderBuffers_;
    std::list<size_t> inBufQue_;
    uint32_t inBufferCnt_;  // 输入buffer个数，默认8个
    uint32_t outBufferCnt_; // 输入buffer个数，默认8个
    uint32_t outBufferSize_;
    sptr<Surface> surface_{nullptr};
    VideoPixelFormat outputPixelFmt_{VideoPixelFormat::RGBA};
    ScalingMode scalingMode_{ScalingMode::SCALING_MODE_SCALE_TO_WINDOW};
    GraphicTransformType surfaceRotate_{GraphicTransformType::GRAPHIC_ROTATE_NONE};

    std::shared_ptr<TaskThread> sendTask_{nullptr};
    std::shared_ptr<TaskThread> receiveTask_{nullptr};
    std::shared_ptr<TaskThread> renderTask_{nullptr};
    std::shared_mutex inputMutex_;
    std::mutex outputMutex_;
    std::mutex sendMutex_;
    std::mutex syncMutex_;
    std::condition_variable outputCv_;
    std::condition_variable sendCv_;
    std::shared_ptr<AVCodecCallback> callback_;
    bool isSendWait_ = false;
};

} // namespace Codec
} // namespace Media
} // namespace OHOS

#endif // FCODEC_H
