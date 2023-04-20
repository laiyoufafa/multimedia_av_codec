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
#ifndef DEMUXER_SERVICE_STUB_H
#define DEMUXER_SERVICE_STUB_H

#include <map>
#include "i_standard_demuxer_service.h"
#include "demuxer_server.h"
#include "iremote_stub.h"
#include "avcodec_common.h"
#include "i_demuxer_service.h"

namespace OHOS {
namespace Media {
class AVDemuxerStub : public IRemoteStub<IAVDemuxerService>, public NoCopyable {
public:
    static sptr<AVDemuxerStub> Create();
    virtual ~AVDemuxerStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    using DemuxerStubFunc = int32_t(AVDemuxerStub::*)(MessageParcel &data, MessageParcel &reply);

    // 业务
    int32_t Init(uint64_t attr) override;
    int32_t AddSourceTrackByID(uint32_t index) override;
    int32_t RemoveSourceTrackByID(uint32_t index) override;
    int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) override;
    int32_t SeekToTimeStamp(int64_t mSeconds, const AVSeekMode mode) override;

    int32_t DestroyStub() override;
private:
    AVDemuxerStub();
    int32_t InitStub();

    // 业务
    int32_t Init(MessageParcel &data, MessageParcel &reply);
    int32_t AddSourceTrackByID(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveSourceTrackByID(MessageParcel &data, MessageParcel &reply);
    int32_t CopyCurrentSampleToBuf(MessageParcel &data, MessageParcel &reply);
    int32_t SeekToTimeStamp(MessageParcel &data, MessageParcel &reply);

    int32_t DestroyStub(MessageParcel &data, MessageParcel &reply);

    std::mutex mutex_;
    std::shared_ptr<IAVDemuxer> demuxerServer_ = nullptr;
    std::map<uint32_t, DemuxerStubFunc> demuxerFuncs_;
};
}  // namespace Media
}  // namespace OHOS
#endif