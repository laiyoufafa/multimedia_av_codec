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

#ifndef SOURCE_SERVICE_STUB_H
#define SOURCE_SERVICE_STUB_H

#include <string>
#include "i_standard_source_service.h"
#include "source_server.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Media {
class SourceServiceStub : public IRemoteStub<IStandardSourceService>, public NoCopyable {
public:
    static sptr<SourceServiceStub> Create();
    virtual ~SourceServiceStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using SourceStubFunc = int32_t(SourceServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t Init(const std::string &uri) override;
    int32_t GetTrackCount(uint32_t &trackCount) override;
    int32_t SetTrackFormat(const Format &format, uint32_t trackIndex) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackIndex) override;
    int32_t GetSourceFormat(Format &format) override;
    uint64_t GetSourceAddr() override;

    int32_t DumpInfo(int32_t fd);
    int32_t DestroyStub() override;
private:
    SourceServiceStub();
    int32_t InitStub();

    int32_t Init(MessageParcel &data, MessageParcel &reply);
    int32_t GetTrackCount(MessageParcel &data, MessageParcel &reply);
    int32_t SetTrackFormat(MessageParcel &data, MessageParcel &reply);
    int32_t GetTrackFormat(MessageParcel &data, MessageParcel &reply);
    int32_t GetSourceFormat(MessageParcel &data, MessageParcel &reply);
    int32_t GetSourceAddr(MessageParcel &data, MessageParcel &reply);

    int32_t GetDumpInfo(std::string& dumpInfo);
    int32_t DestroyStub(MessageParcel &data, MessageParcel &reply);

    std::mutex mutex_;
    std::shared_ptr<ISourceService> sourceServer_ = nullptr;
    std::map<uint32_t, SourceStubFunc> sourceFuncs_;
};
}  // namespace Media
}  // namespace OHOS
#endif // SOURCE_SERVICE_STUB_H