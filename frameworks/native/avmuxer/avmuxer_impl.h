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
#ifndef AVMUXER_IMPL_H
#define AVMUXER_IMPL_H

#include "avmuxer.h"
#include "i_muxer_service.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class AVMuxerImpl : public AVMuxer, public NoCopyable {
public:
    AVMuxerImpl(int32_t fd, OutputFormat format);
    ~AVMuxerImpl() override;
    int32_t Init();
    int32_t SetLocation(float latitude, float longitude) override;
    int32_t SetRotation(int32_t rotation) override;
    int32_t SetParameter(const Format &generalFormat) override;
    int32_t AddTrack(const Format &trackFormat) override;
    int32_t Start() override;
    int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) override;
    int32_t Stop() override;

private:
    // std::shared_ptr<IMuxerEngine> muxerEngine_ = nullptr;
    std::shared_ptr<IAVMuxer> muxerClient_ = nullptr;
    int32_t fd_ = -1;
    OutputFormat format_ = AV_OUTPUT_FORMAT_UNKNOWN;
};
} // namespace Media
} // namespace OHOS
#endif // AVMUXER_IMPL_H