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
#ifndef DEMUXER_SERVER_H
#define DEMUXER_SERVER_H

#include <mutex>
#include "i_demuxer_service.h"
// #include "i_demuxer_engine.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class DemuxerServer : public IDemuxerService, public NoCopyable {
public:
    static std::shared_ptr<IDemuxerService> Create();
    DemuxerServer();
    ~DemuxerServer();

    // 业务
    int32_t Init(uint64_t attr) override;
    int32_t AddSourceTrackByID(uint32_t index) override;
    int32_t RemoveSourceTrackByID(uint32_t index) override;
    int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) override;
    int32_t SeekToTimeStamp(int64_t mSeconds, const SeekMode mode) override;

private:
    int32_t InitServer();
    std::mutex mutex_;
    // std::shared_ptr<IDemuxerEngine> demuxerEngine_ = nullptr;
    // IDemuxerEngine curState_ = DEMUXER_IDEL;
    // uint32_t trackNum_ = 0;

};

}  // namespace Media
}  // namespace OHOS
#endif  // DEMUXER_SERVER_H