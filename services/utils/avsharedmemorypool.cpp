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

#include "avsharedmemorypool.h"
#include "avsharedmemorybase.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVShMemPool"};
    constexpr int32_t MAX_MEM_SIZE = 100 * 1024 * 1024;
}

namespace OHOS {
namespace Media {
AVSharedMemoryPool::AVSharedMemoryPool(const std::string &name) : name_(name)
{
    AVCODEC_LOGD("enter ctor, 0x%{public}06" PRIXPTR ", name: %{public}s", FAKE_POINTER(this), name_.c_str());
}

AVSharedMemoryPool::~AVSharedMemoryPool()
{
    AVCODEC_LOGD("enter dtor, 0x%{public}06" PRIXPTR ", name: %{public}s", FAKE_POINTER(this), name_.c_str());
    Reset();
}

int32_t AVSharedMemoryPool::Init(const InitializeOption &option)
{
    std::unique_lock<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(!inited_, AVCS_ERR_INVALID_OPERATION, "already init, please do not init again.");
    CHECK_AND_RETURN_RET_LOG(option.memSize < MAX_MEM_SIZE, AVCS_ERR_INVALID_VAL, "memSize = %{public}d is more than "
                            "maxSize = %{public}d.", option.memSize, MAX_MEM_SIZE);
    CHECK_AND_RETURN_RET_LOG(option.maxMemCnt != 0, AVCS_ERR_INVALID_VAL, "maxMemCnt is equal 0.");

    option_ = option;
    if (option.preAllocMemCnt > option.maxMemCnt) {
        option_.preAllocMemCnt = option.maxMemCnt;
    }

    AVCODEC_LOGI("name: %{public}s, init option: preAllocMemCnt = %{public}u, memSize = %{public}d, "
                 "maxMemCnt = %{public}u, enableFixedSize = %{public}d",
                 name_.c_str(), option_.preAllocMemCnt, option_.memSize, option_.maxMemCnt,
                 option_.enableFixedSize);
    bool ret = true;
    for (uint32_t i = 0; i < option_.preAllocMemCnt; ++i) {
        auto memory = AllocMemory(option_.memSize);
        if (memory == nullptr) {
            AVCODEC_LOGE("alloc memory failed");
            ret = false;
            break;
        }
        idleList_.push_back(memory);
    }
    
    if (!ret) {
        for (auto iter = idleList_.begin(); iter != idleList_.end(); ++iter) {
            delete *iter;
            *iter = nullptr;
        }
        return AVCS_ERR_NO_MEMORY;
    }

    inited_ = true;
    notifier_ = option.notifier;
    return AVCS_ERR_OK;
}

AVSharedMemory *AVSharedMemoryPool::AllocMemory(int32_t size)
{
    AVSharedMemoryBase *memory = new (std::nothrow) AVSharedMemoryBase(size, option_.flags, name_);
    CHECK_AND_RETURN_RET_LOG(memory != nullptr, nullptr, "create object failed");

    if (memory->Init() != AVCS_ERR_OK) {
        delete memory;
        memory = nullptr;
        AVCODEC_LOGE("init avsharedmemorybase failed");
    }

    return memory;
}

void AVSharedMemoryPool::ReleaseMemory(AVSharedMemory *memory)
{
    CHECK_AND_RETURN_LOG(memory != nullptr, "memory is nullptr");
    std::unique_lock<std::mutex> lock(mutex_);

    for (auto iter = busyList_.begin(); iter != busyList_.end(); ++iter) {
        if (*iter != memory) {
            continue;
        }

        busyList_.erase(iter);
        idleList_.push_back(memory);
        cond_.notify_all();
        AVCODEC_LOGD("0x%{public}06" PRIXPTR " released back to pool %{public}s",
                    FAKE_POINTER(memory), name_.c_str());

        lock.unlock();
        if (notifier_ != nullptr) {
            notifier_();
        }
        return;
    }

    AVCODEC_LOGE("0x%{public}06" PRIXPTR " is no longer managed by this pool", FAKE_POINTER(memory));
    delete memory;
}

bool AVSharedMemoryPool::DoAcquireMemory(int32_t size, AVSharedMemory **outMemory)
{
    AVCODEC_LOGD("busylist size %{public}zu, idlelist size %{public}zu", busyList_.size(), idleList_.size());

    AVSharedMemory *result = nullptr;
    std::list<AVSharedMemory *>::iterator minSizeIdleMem = idleList_.end();
    int32_t minIdleSize = std::numeric_limits<int32_t>::max();

    for (auto iter = idleList_.begin(); iter != idleList_.end(); ++iter) {
        if ((*iter)->GetSize() >= size) {
            result = *iter;
            idleList_.erase(iter);
            break;
        }

        if ((*iter)->GetSize() < minIdleSize) {
            minIdleSize = (*iter)->GetSize();
            minSizeIdleMem = iter;
        }
    }

    if (result == nullptr) {
        auto totalCnt = busyList_.size() + idleList_.size();
        if (totalCnt < option_.maxMemCnt) {
            result = AllocMemory(size);
            CHECK_AND_RETURN_RET_LOG(result != nullptr, false, "result is nullptr, AllocMemory failed.");
        }

        if (!option_.enableFixedSize && minSizeIdleMem != idleList_.end()) {
            delete *minSizeIdleMem;
            *minSizeIdleMem = nullptr;
            idleList_.erase(minSizeIdleMem);
            result = AllocMemory(size);
            CHECK_AND_RETURN_RET_LOG(result != nullptr, false, "result is nullptr, AllocMemory failed.");
        }
    }

    *outMemory = result;
    return true;
}

bool AVSharedMemoryPool::CheckSize(int32_t size)
{
    if (size <= 0 && size != -1) {
        return false;
    }

    if (!option_.enableFixedSize && size == -1) {
        return false;
    }

    if (option_.enableFixedSize) {
        if (size > option_.memSize) {
            return false;
        }

        if (size <= 0 && size != -1) {
            return false;
        }
    }

    return true;
}

std::shared_ptr<AVSharedMemory> AVSharedMemoryPool::AcquireMemory(int32_t size, bool blocking)
{
    AVCODEC_LOGD("acquire memory for size: %{public}d from pool %{public}s, blocking: %{public}d",
                 size, name_.c_str(), blocking);

    std::unique_lock<std::mutex> lock(mutex_);
    if (!CheckSize(size)) {
        AVCODEC_LOGE("invalid size: %{public}d", size);
        return nullptr;
    }

    if (option_.enableFixedSize) {
        size = option_.memSize;
    }

    AVSharedMemory *memory = nullptr;
    do {
        if (!DoAcquireMemory(size, &memory) || memory != nullptr) {
            break;
        }

        if (!blocking || forceNonBlocking_) {
            break;
        }

        cond_.wait(lock);
    } while (inited_ && !forceNonBlocking_);

    if (memory == nullptr) {
        AVCODEC_LOGD("acquire memory failed for size: %{public}d", size);
        return nullptr;
    }

    busyList_.push_back(memory);

    auto result = std::shared_ptr<AVSharedMemory>(memory, [weakPool = weak_from_this()](AVSharedMemory *memory) {
        std::shared_ptr<AVSharedMemoryPool> pool = weakPool.lock();
        if (pool != nullptr) {
            pool->ReleaseMemory(memory);
        } else {
            AVCODEC_LOGI("release memory 0x%{public}06" PRIXPTR ", but the pool is destroyed", FAKE_POINTER(memory));
            delete memory;
        }
    });

    AVCODEC_LOGD("0x%{public}06" PRIXPTR " acquired from pool", FAKE_POINTER(memory));
    return result;
}

void AVSharedMemoryPool::SetNonBlocking(bool enable)
{
    std::unique_lock<std::mutex> lock(mutex_);
    AVCODEC_LOGD("SetNonBlocking: %{public}d", enable);
    forceNonBlocking_ = enable;
    if (forceNonBlocking_) {
        cond_.notify_all();
    }
}

void AVSharedMemoryPool::Reset()
{
    AVCODEC_LOGD("Reset");

    std::unique_lock<std::mutex> lock(mutex_);
    for (auto &memory : idleList_) {
        delete memory;
        memory = nullptr;
    }
    idleList_.clear();
    inited_ = false;
    forceNonBlocking_ = false;
    notifier_ = nullptr;
    cond_.notify_all();
    // for busylist, the memory will be released when the refcount of shared_ptr is zero.
}
} // namespace Media
} // namespace OHOS