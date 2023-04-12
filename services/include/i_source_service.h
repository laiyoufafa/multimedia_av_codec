#ifndef I_SOURCE_SERVICE_H
#define I_SOURCE_SERVICE_H

#include <string>
#include <memory>
#include "avcodec_base.h"

namespace OHOS {
namespace Media {
class ISourceService {
public:
    virtual ~ISourceService() = default;

    // 业务
    virtual int32_t Init(const std::string &uri) = 0;
    virtual int32_t GetTrackCount() = 0;
    virtual int32_t Destroy() = 0;
    virtual int32_t SetParameter(const Format &param, uint32_t trackId) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackId) = 0;
    virtual uint64_t GetSourceAttr() = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_SOURCE_SERVICE_H