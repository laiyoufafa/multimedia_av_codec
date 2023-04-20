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
namespace Media {
// using AVSeekMode = OH_AVSeekMode;
typedef enum AVSeekMode {
    SEEK_MODE_NEXT_SYNC = 0,
    SEEK_MODE_PREVIOUS_SYNC,
    SEEK_MODE_CLOSEST_SYNC,
    SEEK_MODE_CLOSEST
} AVSeekMode;

class IAVDemuxer {
public:
    virtual ~IAVDemuxer() = default;

    // 业务
    virtual int32_t Init(uint64_t attr) = 0;
    virtual int32_t AddSourceTrackByID(uint32_t index) = 0;
    virtual int32_t RemoveSourceTrackByID(uint32_t index) = 0;
    virtual int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) = 0;
    virtual int32_t SeekToTimeStamp(int64_t mSeconds, const AVSeekMode mode) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_DEMUXER_SERVICE_H