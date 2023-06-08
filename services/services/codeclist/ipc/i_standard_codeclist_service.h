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

#ifndef I_STANDARD_CODECLIST_SERVICE_H
#define I_STANDARD_CODECLIST_SERVICE_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "avcodec_info.h"

namespace OHOS {
namespace Media {
class IStandardCodecListService : public IRemoteBroker {
public:
    virtual ~IStandardCodecListService() = default;
    virtual std::string FindDecoder(const Format &format) = 0;
    virtual std::string FindEncoder(const Format &format) = 0;
    virtual int32_t GetCapability(CapabilityData &capabilityData, const std::string &mime, const bool isEncoder,
                                  const AVCodecCategory &category) = 0;
    virtual int32_t DestroyStub() = 0;

    /**
     * IPC code ID
     */
    enum class AVCodecListServiceMsg : uint32_t { FIND_DECODER = 0, FIND_ENCODER, GET_CAPABILITY, DESTROY };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardCodecListService");
};
} // namespace Media
} // namespace OHOS
#endif // I_STANDARD_CODECLIST_SERVICE_H
