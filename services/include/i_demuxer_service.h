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
#ifndef I_DEMUXER_SERVICE_H
#define I_DEMUXER_SERVICE_H

#include <string>
#include <memory>
#include "avcodec_common.h"

namespace OHOS {
namespace Media {
class IDemuxerService {
public:
    virtual ~IDemuxerService() = default;

    virtual int32_t Init(uintptr_t sourceAddr) = 0;
    virtual int32_t SelectSourceTrackByID(uint32_t index) = 0;
    virtual int32_t UnselectSourceTrackByID(uint32_t index) = 0;
    virtual int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer,
                                    AVCodecBufferInfo &bufferInfo, AVCodecBufferFlag &flag) = 0;
    virtual int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) = 0;
};
} // namespace Media
} // namespace OHOS
#endif // I_DEMUXER_SERVICE_H