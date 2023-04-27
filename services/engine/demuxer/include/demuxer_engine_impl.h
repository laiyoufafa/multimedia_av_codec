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
#include <atomic>
#include <thread>
// #include <mutex>
#include <condition_variable>
#include "i_demuxer_engine.h"
#include "demuxer.h"
#include "block_queue.h"
#include "avcodec_common.h"

namespace OHOS {
namespace Media {
class DemuxerEngineImpl : public IDemuxerEngine {
public:
    DemuxerEngineImpl(int32_t appUid, int32_t appPid, uintptr_t sourceAddr);
    ~DemuxerEngineImpl() override;
    int32_t SelectSourceTrackByID(uint32_t trackIndex) override;
    int32_t UnselectSourceTrackByID(uint32_t trackIndex) override;
    int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo) override;
    int32_t SeekToTime(int64_t mSeconds, AVSeekMode mode) override;


private:
    // int32_t StartThread(std::string name);
    // int32_t StopThread() noexcept;
    // void ThreadProcessor();
    // bool CanAddTrack(std::string &mimeType);
    // bool CheckKeys(std::string &mimeType, const Format &trackFormat);
    // std::string ConvertStateToString(int32_t state);

private:
    uintptr_t sourceAddr_;
    // enum State {
    //     UNINITIALIZED,
    //     INITIALIZED,
    //     STARTED,
    //     STOPPED
    // };
    // struct BlockBuffer {
    //     uint32_t trackIndex_;
    //     std::shared_ptr<AVSharedMemory> buffer_;
    //     AVCodecBufferInfo info_;
    // };

    int32_t appUid_ = -1;
    int32_t appPid_ = -1;
//     int64_t fd_ = -1;
//     OutputFormat format_;
    // std::atomic<State> state_ = UNINITIALIZED;
    std::shared_ptr<Plugin::Demuxer> demuxer_ = nullptr;
    // std::map<uint32_t, std::string> tracks_;
    // BlockQueue<std::shared_ptr<BlockBuffer>> que_;
    // std::string threadName_;
    // std::mutex mutex_;
    // std::condition_variable cond_;
    // std::unique_ptr<std::thread> thread_ = nullptr;
    // bool isThreadExit_ = true;
};
} // namespace Media
} // namespace OHOS
#endif // DEMUXER_ENGINE_IMPL_H