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
#ifndef DEMUXER_SERVICE_PROXY_H
#define DEMUXER_SERVICE_PROXY_H

#include "i_standard_demuxer_service.h"

namespace OHOS {
namespace MediaAVCodec {
class DemuxerServiceProxy : public IRemoteProxy<IStandardDemuxerService>, public NoCopyable {
public:
    explicit DemuxerServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~DemuxerServiceProxy();
    int32_t Init(uintptr_t sourceAddr) override;
    int32_t SelectTrackByID(uint32_t trackIndex) override;
    int32_t UnselectTrackByID(uint32_t trackIndex) override;
    int32_t ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;
    int32_t SeekToTime(int64_t millisecond, const AVSeekMode mode) override;
    int32_t DestroyStub() override;
private:
    static inline BrokerDelegator<DemuxerServiceProxy> delegator_;
};
}  // namespace MediaAVCodec
}  // namespace OHOS
#endif  // DEMUXER_SERVICE_PROXY_H