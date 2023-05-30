/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DEMUXER_ENGINE_IMPL_H
#define DEMUXER_ENGINE_IMPL_H

#include <map>
#include <mutex>
#include <condition_variable>
#include "i_demuxer_engine.h"
#include "demuxer.h"
#include "avcodec_common.h"

namespace OHOS {
namespace Media {
class DemuxerEngineImpl : public IDemuxerEngine {
public:
    DemuxerEngineImpl(int32_t appUid, int32_t appPid, uintptr_t sourceAddr);
    ~DemuxerEngineImpl() override;
    int32_t SelectTrackByID(uint32_t trackIndex) override;
    int32_t UnselectTrackByID(uint32_t trackIndex) override;
    int32_t ReadSample(uint32_t trackIndex, std::shared_ptr<AVSharedMemory> sample,
        AVCodecBufferInfo &info, AVCodecBufferFlag &flag) override;
    int32_t SeekToTime(int64_t millisecond, AVSeekMode mode) override;
private:
    int32_t appUid_ = -1;
    int32_t appPid_ = -1;
    uintptr_t sourceAddr_;
    std::mutex mutex_;
    std::shared_ptr<Plugin::Demuxer> demuxer_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // DEMUXER_ENGINE_IMPL_H