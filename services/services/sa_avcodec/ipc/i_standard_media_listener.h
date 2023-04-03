#ifndef I_STANDARD_MEDIA_LISTENER_H
#define I_STANDARD_MEDIA_LISTENER_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AVCodec {
class IStandardAvcodecListener : public IRemoteBroker {
public:
    virtual ~IStandardAvcodecListener() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardAvcodecListener");
};
} // namespace AVCodec
} // namespace OHOS
#endif // I_STANDARD_MEDIA_LISTENER_H
