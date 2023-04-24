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

#ifndef MUXER_CLIENT_H
#define MUXER_CLIENT_H

#include "i_avmuxer.h"
#include "i_avmuxer_service.h"

namespace OHOS {
namespace Media {
class AVMuxerClient : public IAVMuxer, public NoCopyable {
public:
    static std::shared_ptr<AVMuxerClient> Create(const sptr<IAVMuxerService> &ipcProxy);
    explicit AVMuxerClient(const sptr<IAVMuxerService> &ipcProxy);
    ~AVMuxerClient();

    int32_t Init() override;
    int32_t SetLocation(float latitude, float longitude) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t SetParameter(const Format &generalFormat) override;
    int32_t AddTrack(uint32_t &trackIndex, const Format &trackFormat) override;
    int32_t Start() override;
    int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) override;
    int32_t Stop() override;

    void AVCodecServerDied();
private:
    std::mutex mutex_;
    sptr<IAVMuxerService> muxerProxy_ = nullptr;
};
}  // namespace Media
}  // namespace OHOS
#endif  // MUXER_CLIENT_H