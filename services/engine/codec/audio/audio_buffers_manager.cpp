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

#include "audio_buffers_manager.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioBuffersManager"};
constexpr uint8_t LOGD_FREQUENCY = 5;
} // namespace

namespace OHOS {
namespace Media {
constexpr short DEFAULT_BUFFER_LENGTH = 8;
constexpr short DEFAULT_SLEEP_TIME = 500;
constexpr short MAX_WAIT_TIMES = 20;

AudioBuffersManager::~AudioBuffersManager()
{
    AVCODEC_LOGI("deconstructor called.");
}

AudioBuffersManager::AudioBuffersManager(const uint32_t &bufferSize, const std::string_view &name,
                                         const uint32_t &metaSize, size_t align)
    : isRunning_(true),
      inBufIndexExist(DEFAULT_BUFFER_LENGTH, false),
      bufferSize_(bufferSize),
      metaSize_(metaSize),
      align_(align),
      name_(name),
      bufferInfo_(DEFAULT_BUFFER_LENGTH)
{
    initBuffers();
}

std::shared_ptr<AudioBufferInfo> AudioBuffersManager::getMemory(const uint32_t &index) const noexcept
{
    if (index >= bufferInfo_.size()) {
        return nullptr;
    }
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "start get memory,name:%{public}s,index:%{public}u", name_.data(), index);
    return bufferInfo_[index];
}

bool AudioBuffersManager::SetBufferBusy(const uint32_t &index)
{
    std::unique_lock lock(stateMutex_);
    if (index < bufferInfo_.size()) {
        bufferInfo_[index]->SetBufferOwned();
        return true;
    }
    return false;
}

void AudioBuffersManager::initBuffers()
{
    std::unique_lock lock(stateMutex_);
    AVCODEC_LOGI("start allocate %{public}s buffers,each buffer size:%{public}d", name_.data(), bufferSize_);
    for (size_t i = 0; i < DEFAULT_BUFFER_LENGTH; i++) {
        bufferInfo_[i] = std::make_shared<AudioBufferInfo>(bufferSize_, name_, metaSize_, align_);
        inBufIndexQue_.emplace(i);
        inBufIndexExist[i] = true;
    }
    AVCODEC_LOGI("end allocate %{public}s buffers", name_.data());
}

bool AudioBuffersManager::RequestNewBuffer(uint32_t &index, std::shared_ptr<AudioBufferInfo> &buffer)
{
    buffer = createNewBuffer();
    if (buffer == nullptr) {
        return false;
    }
    index = bufferInfo_.size() - 1;
    inBufIndexExist.emplace_back(false);
    return true;
}

bool AudioBuffersManager::RequestAvailableIndex(uint32_t &index)
{
    short waitTimes = 0;
    bool isTimeOut = false;
    while (inBufIndexQue_.empty() && isRunning_) {
        if (waitTimes >= MAX_WAIT_TIMES) {
            AVCODEC_LOGW("Request empty %{public}s buffer time out.", name_.data());
            isTimeOut = true;
            break;
        }
        AVCODEC_LOGD("Request empty %{public}s buffer", name_.data());
        std::unique_lock aLock(availableMutex_);
        availableCondition_.wait_for(aLock, std::chrono::milliseconds(DEFAULT_SLEEP_TIME),
                                     [this] { return !inBufIndexQue_.empty() || !isRunning_; });
        waitTimes++;
    }

    if (isTimeOut) {
        return false;
    }

    if (!isRunning_) {
        return false;
    }
    std::unique_lock lock(stateMutex_);
    index = inBufIndexQue_.front();
    inBufIndexQue_.pop();
    if (index >= bufferInfo_.size()) {
        AVCODEC_LOGW("Request %{public}s buffer index is invalidate ,index:%{public}u.", name_.data(), index);
        return false;
    }
    AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "Request %{public}s buffer successful,index:%{public}u,size:%{public}u",
                       name_.data(), index, inBufIndexExist.size());
    inBufIndexExist[index] = false;
    bufferInfo_[index]->SetBufferOwned();
    return true;
}

void AudioBuffersManager::ReleaseAll()
{
    isRunning_ = false;
    availableCondition_.notify_all();
    std::unique_lock lock(stateMutex_);
    while (!inBufIndexQue_.empty()) {
        inBufIndexQue_.pop();
    }
    for (uint32_t i = 0; i < bufferInfo_.size(); ++i) {
        bufferInfo_[i]->ResetBuffer();
        inBufIndexQue_.emplace(i);
        inBufIndexExist[i] = true;
    }
    AVCODEC_LOGI("release all %{public}s buffer.", name_.data());
}

void AudioBuffersManager::SetRunning()
{
    isRunning_ = true;
}

bool AudioBuffersManager::ReleaseBuffer(const uint32_t &index)
{
    if (index < bufferInfo_.size()) {
        AVCODEC_LOGD_LIMIT(LOGD_FREQUENCY, "ReleaseBuffer %{public}s buffer,index:%{public}u", name_.data(), index);
        std::unique_lock lock(stateMutex_);
        bufferInfo_[index]->ResetBuffer();
        if (!inBufIndexExist[index]) {
            inBufIndexQue_.emplace(index);
            inBufIndexExist[index] = true;
        }
        availableCondition_.notify_all();
        return true;
    }
    return false;
}

std::shared_ptr<AudioBufferInfo> AudioBuffersManager::createNewBuffer()
{
    std::unique_lock lock(stateMutex_);
    std::shared_ptr<AudioBufferInfo> buffer = std::make_shared<AudioBufferInfo>(bufferSize_, name_, metaSize_, align_);
    bufferInfo_.emplace_back(buffer);
    return buffer;
}
} // namespace Media
} // namespace OHOS