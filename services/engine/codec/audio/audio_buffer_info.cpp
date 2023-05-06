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
      bufferUseSize_(0),
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

size_t AudioBufferInfo::WriteBuffer(const uint8_t *in, size_t writeSize)
{
    auto length = Write(in, writeSize, bufferSize_);
    bufferUseSize_ = length;
    return length;
}

size_t AudioBufferInfo::WriteMetadata(const uint8_t *in, size_t writeSize)
{
    return Write(in, writeSize, metaSize_);
}

size_t AudioBufferInfo::ReadBuffer(uint8_t *out, size_t readSize)
{
    return Read(out, readSize, bufferUseSize_);
}

size_t AudioBufferInfo::ReadMetadata(uint8_t *out, size_t readSize)
{
    return Read(out, readSize, metaSize_);
}

size_t AudioBufferInfo::Write(const uint8_t *in, size_t writeSize, const size_t &bufferSize)
{
    size_t start = 0;
    size_t length = std::min(writeSize, bufferSize);
    AVCODEC_LOGD("write data,length:%{public}d, start:%{public}d, name:%{public}s", length, start, name_.data());
    auto error = memcpy_s(buffer_->GetBase(), length, in, length);
    if (error != EOK) {
        AVCODEC_LOGE("sharedMem_ WriteToAshmem failed,error:%{public}d, name:%{public}s", error, name_.data());
        return 0;
    }
    return length;
}

size_t AudioBufferInfo::Read(uint8_t *out, size_t readSize, const size_t &useSize)
{
    size_t length = std::min(readSize, useSize);
    if (memcpy_s(out, length, buffer_->GetBase(), length) != EOK) {
        return 0;
    }
    return length;
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
    bufferUseSize_ = 0;
    return true;
}

} // namespace Media
} // namespace OHOS