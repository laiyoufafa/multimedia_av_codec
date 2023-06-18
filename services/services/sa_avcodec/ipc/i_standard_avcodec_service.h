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
#ifndef I_STANDARD_AVCODEC_SERVICE_H
#define I_STANDARD_AVCODEC_SERVICE_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MediaAVCodec {
class IStandardAVCodecService : public IRemoteBroker {
public:
    /**
     * sub system ability ID
     */
    enum AVCodecSystemAbility : int32_t {
        AVCODEC_DEMUXER = 0,
        AVCODEC_MUXER,
        AVCODEC_CODECLIST,
        AVCODEC_CODEC,
        AVCODEC_SOURCE
    };

    virtual sptr<IRemoteObject> GetSubSystemAbility(IStandardAVCodecService::AVCodecSystemAbility subSystemId,
        const sptr<IRemoteObject> &listener) = 0;

    /**
     * IPC code ID
     */
    enum AVCodecServiceMsg : int32_t {
        GET_SUBSYSTEM = 0,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardAVCodecServiceInterface");
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // I_STANDARD_AVCODEC_SERVICE_H
