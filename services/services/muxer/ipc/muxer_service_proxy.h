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

#ifndef MUXER_SERVICE_PROXY_H
#define MUXER_SERVICE_PROXY_H

#include "i_standard_muxer_service.h"

namespace OHOS {
namespace AVCodec {
class MuxerServiceProxy : public IRemoteProxy<IStandardMuxerService>, public NoCopyable {
public:
    explicit MuxerServiceProxy(const sptr<IRemoteObject> &impl);
    virtual ~MuxerServiceProxy();

    int32_t Init() override;
    int32_t SetLocation(float latitude, float longitude) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t SetParameter(const Format &generalFormat) override;
    int32_t AddTrack(const Format &trackFormat) override;
    int32_t Start() override;
    int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) override;
    int32_t Stop() override;

    int32_t DestroyStub() override;
private:
    static inline BrokerDelegator<MuxerServiceProxy> delegator_;
};
}  // namespace AVCodec
}  // namespace OHOS
#endif  // MUXER_SERVICE_PROXY_H