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

#ifndef MUXER_SERVICE_STUB_H
#define MUXER_SERVICE_STUB_H

#include <string>
#include "i_avmuxer_service.h"
#include "avmuxer_server.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Media {
class AVMuxerStub : public IRemoteStub<IAVMuxerService>, public NoCopyable {
public:
    static sptr<AVMuxerStub> Create();
    virtual ~AVMuxerStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using MuxerStubFunc = int32_t(AVMuxerStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t Init() override;
    int32_t SetLocation(float latitude, float longitude) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t SetParameter(const Format &generalFormat) override;
    int32_t AddTrack(uint32_t &trackIndex, const Format &trackFormat) override;
    int32_t Start() override;
    int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) override;
    int32_t Stop() override;

    int32_t DumpInfo(int32_t fd);
    int32_t DestroyStub() override;
private:
    AVMuxerStub();
    int32_t InitStub();
    int32_t Init(MessageParcel &data, MessageParcel &reply);
    int32_t SetLocation(MessageParcel &data, MessageParcel &reply);
    int32_t SetRotation(MessageParcel &data, MessageParcel &reply);
    int32_t SetParameter(MessageParcel &data, MessageParcel &reply);
    int32_t AddTrack(MessageParcel &data, MessageParcel &reply);
    int32_t Start(MessageParcel &data, MessageParcel &reply);
    int32_t WriteSampleBuffer(MessageParcel &data, MessageParcel &reply);
    int32_t Stop(MessageParcel &data, MessageParcel &reply);

    int32_t GetDumpInfo(std::string& dumpInfo);
    int32_t DestroyStub(MessageParcel &data, MessageParcel &reply);

    std::mutex mutex_;
    std::shared_ptr<IAVMuxer> muxerServer_ = nullptr;
    std::map<uint32_t, MuxerStubFunc> muxerFuncs_;
};
}  // namespace Media
}  // namespace OHOS
#endif