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

#ifndef AVCODECLIST_SERVICE_STUB_H
#define AVCODECLIST_SERVICE_STUB_H

#include <map>
#include "i_standard_avcodeclist_service.h"
#include "avcodec_death_recipient.h"
#include "avcodeclist_server.h"
#include "nocopyable.h"
#include "avcodec_parcel.h"
#include "avcodeclist_parcel.h"

namespace OHOS {
namespace Media {
class AVCodecListServiceStub : public IRemoteStub<IStandardAVCodecListService>, public NoCopyable {
public:
    static sptr<AVCodecListServiceStub> Create();
    virtual ~AVCodecListServiceStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    std::string FindDecoder(const Format &format) override;
    std::string FindEncoder(const Format &format) override;
    CapabilityData CreateCapability(std::string codecName) override;
    int32_t DestroyStub() override;

private:
    AVCodecListServiceStub();
    int32_t Init();
    int32_t DoFindDecoder(MessageParcel &data, MessageParcel &reply);
    int32_t DoFindEncoder(MessageParcel &data, MessageParcel &reply);
    int32_t DoCreateCapability(MessageParcel &data, MessageParcel &reply);
    int32_t DoDestroyStub(MessageParcel &data, MessageParcel &reply);
    std::shared_ptr<IAVCodecListService> codecListServer_ = nullptr;
    using AVCodecListStubFunc = int32_t(AVCodecListServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, AVCodecListStubFunc> codecListFuncs_;
    std::mutex mutex_;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODECLIST_SERVICE_STUB_H
