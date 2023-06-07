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

#ifndef UTILS_BLOCK_QUEUE_POOL_H
#define UTILS_BLOCK_QUEUE_POOL_H
#include <vector>
#include <map>
#include <cstdint>
#include "block_queue.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace Media {
namespace {
constexpr size_t SINGLE_QUEUE_SIZE = 100;
}

struct SamplePacket {
    uint64_t offset;
    AVPacket* pkt;
};

class BlockQueuePool {
public:
    explicit BlockQueuePool(std::string name, size_t singleQueSize = SINGLE_QUEUE_SIZE)
        : name_(std::move(name)), singleQueSize_(singleQueSize)
    {
    }
    ~BlockQueuePool();

    int32_t AddTrackQueue(uint32_t trackIndex);
    int32_t RemoveTrackQueue(uint32_t trackIndex);
    bool HasCache(uint32_t trackIndex);
    void ResetQueue(uint32_t queueIndex);
    void FreeQueue(uint32_t queueIndex);
    bool Push(uint32_t trackIndex, std::shared_ptr<SamplePacket> block);
    std::shared_ptr<SamplePacket> Pop(uint32_t trackIndex);
    std::shared_ptr<SamplePacket> Front(uint32_t trackIndex);
    
private:
    struct InnerQueue {
        bool isValid;
        std::shared_ptr<BlockQueue<std::shared_ptr<SamplePacket>>> blockQue;
    };
    std::string name_;
    uint32_t queCount_;
    std::map<uint32_t, InnerQueue> quePool_;
    std::map<uint32_t, std::vector<uint32_t>> queMap_;
    size_t singleQueSize_;
    uint32_t GetValidQueue();
    bool InnerQueueIsFull(uint32_t queueIndex);
    bool IsInQueue(uint32_t trackIndex);
};
} // namespace Media
} // namespace OHOS
#endif // !UTILS_BLOCK_QUEUE_POOL_H