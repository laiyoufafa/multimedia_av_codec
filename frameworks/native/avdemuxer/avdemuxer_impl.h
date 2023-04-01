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

#ifndef AVDEMUXER_IMPL_H
#define AVDEMUXER_IMPL_H

#include <memory>
#include "avdemuxer.h"
#include "i_demuxer_service.h"
#include "nocopyable.h"

namespace OHOS {
namespace AVCodec{
class AVDemuxerImpl : public AVDemuxer, public NoCopyable {
public:
    AVDemuxerImpl();
    ~AVDemuxerImpl();

    int32_t AddSourceTrackByID(uint32_t index) override;
    int32_t RemoveSourceTrackByID(uint32_t index) override;
    int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) override;
    int32_t SeekToTimeStamp(int64_t mSeconds, const SeekMode mode) override;
    int32_t Init(Source *source);

private:
    std::shared_ptr<IAVDemuxerService> demuxerService_ = nullptr;
};
} // namespace AVCodec
} // namespace OHOS
#endif // AVDEMUXER_IMPL_H