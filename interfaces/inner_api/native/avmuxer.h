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

#ifndef AVMUXER_H
#define AVMUXER_H

#include "format.h"
#include "avcodec_errors.h"
#include "avcodec_base.h"
#include "avcodec_common.h"

namespace OHOS {
namespace AVCodec {
class AVMuxer {
public:
    virtual ~AVMuxer() = default;
    virtual int32_t SetLocation(float latitude, float longitude) = 0;
    virtual int32_t SetRotation(int32_t rotation) = 0;
    virtual int32_t SetParameter(const Format &generalFormat) = 0;
    virtual int32_t AddTrack(const Format &trackFormat) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) = 0;
    virtual int32_t Stop() = 0;
};

class __attribute__((visibility("default"))) AVMuxerFactory {
public:
    static std::shared_ptr<AVMuxer> CreateAVMuxer(int32_t fd, OutputFormat format);
private:
    AVMuxerFactory() = default;
    ~AVMuxerFactory() = default;
};
} // namespace AVCodec
} // namespace OHOS

#endif // AVMUXER_H
