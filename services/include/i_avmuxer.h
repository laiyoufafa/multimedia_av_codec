#ifndef I_MUXER_SERVICE_H
#define I_MUXER_SERVICE_H

#include <string>
#include <memory>
#include "format.h"
#include "avcodec_common.h"

namespace OHOS {
namespace Media {
class IAVMuxer {
public:
    virtual ~IAVMuxer() = default;

    // 业务
    virtual int32_t Init() = 0;
    virtual int32_t SetLocation(float latitude, float longitude) = 0;
    virtual int32_t SetRotation(int32_t rotation) = 0;
    virtual int32_t SetParameter(const Format &generalFormat) = 0;
    virtual int32_t AddTrack(uint32_t &trackIndex, const Format &trackFormat) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WriteSampleBuffer(uint32_t trackIndex, const std::shared_ptr<AVSharedMemory> &sampleBuffer,
                                     AVCodecBufferInfo info) = 0;
    virtual int32_t Stop() = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_MUXER_SERVICE_H