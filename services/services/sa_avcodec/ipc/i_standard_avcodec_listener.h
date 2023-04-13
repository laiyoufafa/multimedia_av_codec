#ifndef I_STANDARD_AVCODEC_LISTENER_H
#define I_STANDARD_AVCODEC_LISTENER_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MediaAVCodec {
class IStandardAVCodecListener : public IRemoteBroker {
public:
    virtual ~IStandardAVCodecListener() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardAVCodecListener");
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // I_STANDARD_AVCODEC_LISTENER_H
