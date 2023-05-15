/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "audio_buffer_info.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"
#include "securec.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-AudioBufferInfo"};
}

namespace OHOS {
namespace Media {
AudioBufferInfo::AudioBufferInfo(const uint32_t &bufferSize, const std::string_view &name, const uint32_t &metaSize,
                                 size_t align)
    : isHasMeta_(false),
      isEos_(false),
      status_(BufferStatus::IDEL),
      bufferSize_(bufferSize),
      metaSize_(metaSize),
      name_(name),
      buffer_(nullptr),
      metadata_(nullptr),
      flag_(AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE)
{
    if (metaSize_ > 0) {
        metadata_ =
            std::make_shared<AVSharedMemoryBase>(metaSize_, AVSharedMemory::Flags::FLAGS_READ_ONLY, std::string(name_));
        int32_t ret = metadata_->Init();
        if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
            AVCODEC_LOGE("Create metadata avsharedmemory failed, ret = %{public}d", ret);
        }
        isHasMeta_ = true;
    }

    buffer_ =
        std::make_shared<AVSharedMemoryBase>(bufferSize_, AVSharedMemory::Flags::FLAGS_READ_WRITE, std::string(name_));
    int32_t ret = buffer_->Init();
    if (ret != AVCodecServiceErrCode::AVCS_ERR_OK) {
        AVCODEC_LOGE("Create buffer avsharedmemory failed, ret = %{public}d", ret);
    }
    AVCODEC_LOGI("AudioBufferInfo constructor %{public}s buffer.", name_.data());
}

AudioBufferInfo::~AudioBufferInfo()
{
    AVCODEC_LOGI("AudioBufferInfo destructor %{public}s buffer.", name_.data());
    isEos_ = false;
    status_ = BufferStatus::IDEL;

    if (buffer_) {
        buffer_.reset();
        buffer_ = nullptr;
    }

    if (metadata_) {
        metadata_.reset();
        metadata_ = nullptr;
    }
}

std::shared_ptr<AVSharedMemoryBase> AudioBufferInfo::GetBuffer() const noexcept
{
    return buffer_;
}

BufferStatus AudioBufferInfo::GetStatus() const noexcept
{
    return status_;
}

bool AudioBufferInfo::IsAvilable() const noexcept
{
    return status_ == BufferStatus::IDEL;
}

bool AudioBufferInfo::CheckIsEos() const noexcept
{
    return isEos_;
}

bool AudioBufferInfo::SetBufferOwned()
{
    if (buffer_) {
        status_ = BufferStatus::OWNE_BY_CLIENT;
        return true;
    }
    return false;
}

void AudioBufferInfo::SetEos(bool eos)
{
    isEos_ = eos;
    if (isEos_) {
        flag_ = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_EOS;
    } else {
        flag_ = AVCodecBufferFlag::AVCODEC_BUFFER_FLAG_NONE;
    }
}

void AudioBufferInfo::SetBufferAttr(const AVCodecBufferInfo &attr)
{
    info_ = attr;
}

AVCodecBufferInfo AudioBufferInfo::GetBufferAttr() const noexcept
{
    return info_;
}

AVCodecBufferFlag AudioBufferInfo::GetFlag() const noexcept
{
    return flag_;
}

std::shared_ptr<AVSharedMemoryBase> AudioBufferInfo::GetMetadata() const noexcept
{
    return metadata_;
}

bool AudioBufferInfo::IsHasMetaData() const noexcept
{
    return isHasMeta_;
}

bool AudioBufferInfo::ResetBuffer()
{
    isEos_ = false;
    status_ = BufferStatus::IDEL;
    if (buffer_) {
        buffer_->ClearUsedSize();
    }
    return true;
}
} // namespace Media
} // namespace OHOS