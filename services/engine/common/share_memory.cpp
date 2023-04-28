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

#include "share_memory.h"
#include "ashmem.h"
#include "avcodec_log.h"
#include "securec.h"
#include "utils.h"
#include <sys/mman.h>
#include <unistd.h>

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AvCodec-ShareMemory"};
}

namespace OHOS {
namespace Media {

ShareMemory::ShareMemory(size_t capacity, const std::string_view &name, uint32_t flags, size_t align)
    : capacity_(capacity), size_(0), flags_(flags), offset(0), name_(name)
{
    capacity_ = align ? (capacity_ + align - 1) : capacity;
    AVCODEC_LOGI("allocate share memory,memory size:%{public}d, align:%{public}d, name:%{public}s", capacity_, align,
                 name_.data());
    fd_ = AshmemCreate(name_.data(), static_cast<size_t>(capacity_));
    if (fd_ <= 0) {
        AVCODEC_LOGE("allocate share memory failed,fd:%{public}d, name:%{public}s", fd_, name_.data());
    }
    int32_t ret = mapMemory(false);
    if (ret != 0) {
        AVCODEC_LOGE("mapMemory memory failed,ret:%{public}d, name:%{public}s", ret, name_.data());
    }
    offset = static_cast<size_t>(AlignUp(reinterpret_cast<uintptr_t>(base_), static_cast<uintptr_t>(align)) -
                                 reinterpret_cast<uintptr_t>(base_));
}

ShareMemory::~ShareMemory()
{
    if (base_ != nullptr) {
        (void)::munmap(base_, static_cast<size_t>(size_));
        base_ = nullptr;
        size_ = 0;
        flags_ = 0;
        capacity_ = 0;
    }

    if (fd_ > 0) {
        (void)::close(fd_);
        fd_ = -1;
    }
}

size_t ShareMemory::Write(const uint8_t *in, size_t writeSize, size_t position)
{
    size_t start = 0;
    if (position == INVALID_POSITION) {
        start = size_;
    } else {
        start = std::min(position, capacity_);
    }
    size_t length = std::min(writeSize, capacity_ - start);
    AVCODEC_LOGD("write data,length:%{public}d, start:%{public}d, name:%{public}s", length, start, name_.data());
    auto error = memcpy_s(GetWritableAddr(length, start), length, in, length);
    if (error != EOK) {
        AVCODEC_LOGE("sharedMem_ WriteToAshmem failed,error:%{public}d, name:%{public}s", error, name_.data());
        return 0;
    }
    size_ = start + length;
    return length;
}

size_t ShareMemory::Read(uint8_t *out, size_t readSize, size_t position)
{
    size_t start = 0;
    size_t maxLength = size_;
    if (position != INVALID_POSITION) {
        start = std::min(position, size_);
        maxLength = size_ - start;
    }
    size_t length = std::min(readSize, maxLength);
    if (memcpy_s(out, length, GetReadOnlyData(start), length) != EOK) {
        return 0;
    }
    return length;
}

int32_t ShareMemory::GetShareMemoryFd()
{
    return fd_;
}

uint8_t *ShareMemory::GetBase() const
{
    return base_ + offset;
}

uint32_t ShareMemory::GetFlags() const
{
    return flags_;
}

void ShareMemory::Reset()
{
    this->size_ = 0;
}

size_t ShareMemory::GetUsedSize() const noexcept
{
    return size_;
}

int32_t ShareMemory::GetSize() const
{
    return capacity_;
}

const uint8_t *ShareMemory::GetReadOnlyData(size_t position)
{
    if (position > capacity_) {
        return nullptr;
    }
    return GetBase() + position;
}

uint8_t *ShareMemory::GetWritableAddr(size_t estimatedWriteSize, size_t position)
{
    if (position + estimatedWriteSize > capacity_) {
        return nullptr;
    }
    uint8_t *ptr = GetBase() + position;
    size_ = (estimatedWriteSize + position);
    return ptr;
}

int32_t ShareMemory::mapMemory(bool isRemote)
{
    unsigned int port = PROT_READ | PROT_WRITE;
    if (isRemote && (flags_ & FLAGS_READ_ONLY)) {
        port &= ~PROT_WRITE;
    }

    int result = AshmemSetProt(fd_, static_cast<int>(port));
    if (result < 0) {
        AVCODEC_LOGE("failed to exec AshmemSetProt,result:%{public}d, name:%{public}s", result, name_.data());
        return -1;
    }

    void *addr = ::mmap(nullptr, static_cast<size_t>(capacity_), static_cast<int>(port), MAP_SHARED, fd_, 0);
    if (addr == MAP_FAILED) {
        AVCODEC_LOGE("failed to exec mmap");
        return -1;
    }

    base_ = reinterpret_cast<uint8_t *>(addr);
    return 0;
}

} // namespace Media
} // namespace OHOS