/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef I_STANDARD_SOURCE_SERVICE_H
#define I_STANDARD_SOURCE_SERVICE_H

#include "i_source_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Media {
class IStandardSourceService : public IRemoteBroker {
public:
    virtual ~IStandardSourceService() = default;

    virtual int32_t Init(const std::string &uri) = 0;
    virtual int32_t GetTrackCount() = 0;
    virtual int32_t Destroy() = 0;
    virtual int32_t SetParameter(const Format &param, uint32_t trackId) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackId) = 0;
    virtual uint64_t GetSourceAttr() = 0;

    virtual int32_t DestroyStub() = 0;
    enum MuxerServiceMsg {
        INIT,
        GET_TRACK_COUNT,
        DESTROY,
        SET_PARAMETER,
        GET_TRACK_FORMAT,
        GET_SOURCE_ATTR,

        DESTROY_STUB,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardSourceService");
};
}  // namespace Media
}  // namespace OHOS
#endif  // I_STANDARD_SOURCE_SERVICE_H