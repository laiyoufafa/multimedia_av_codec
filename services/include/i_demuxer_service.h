#ifndef I_DEMUXER_SERVICE_H
#define I_DEMUXER_SERVICE_H

#include <string>
// #include <vector>
#include <memory>
// #include "avsharedmemory.h"
// #include "avcontainer_common.h"
// #include "media_description.h"
#include "avcodec_base.h"
namespace OHOS {
namespace AVCodec {
using OH_AVSeekMode = AVSeekMode;
class IDemuxerService {
public:
    virtual ~IDemuxerService() = default;

    // 业务
    virtual int32_t Init(uint64_t attr) = 0;
    virtual int32_t AddSourceTrackByID(uint32_t index) = 0;
    virtual int32_t RemoveSourceTrackByID(uint32_t index) = 0;
    virtual int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) = 0;
    virtual int32_t SeekToTimeStamp(int64_t mSeconds, const AVSeekMode mode) = 0;
};
} // namespace AVCodec
} // namespace OHOS
#endif // I_DEMUXER_SERVICE_H