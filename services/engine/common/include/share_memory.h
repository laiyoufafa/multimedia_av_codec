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

#ifndef AV_CODEC_SHARE_MEMORY_H
#define AV_CODEC_SHARE_MEMORY_H

#include "avsharedmemory.h"
#include <stdint.h>
#include <string_view>

constexpr size_t INVALID_POSITION = -1;

namespace OHOS {
namespace Media {

class ShareMemory : public AVSharedMemory {
public:
    ~ShareMemory() override;

    ShareMemory(size_t capacity, const std::string_view &name, uint32_t flags, size_t align = 1);

    size_t Write(const uint8_t *in, size_t writeSize, size_t position = INVALID_POSITION);

    size_t Read(uint8_t *out, size_t readSize, size_t position = INVALID_POSITION);

    int32_t GetShareMemoryFd();
    void Reset();
    size_t GetCapacity() const noexcept;

    const uint8_t *GetReadOnlyData(size_t position = 0);

    uint8_t *GetWritableAddr(size_t estimatedWriteSize, size_t position = 0);

    int32_t GetSize() const override;

    uint8_t *GetBase() const override;

    uint32_t GetFlags() const override;

private:
    int32_t mapMemory(bool isRemote);

    /// Allocated memory size.
    size_t capacity_;
    size_t size_;
    uint32_t flags_;
    size_t offset;
    std::string_view name_;
    int32_t fd_;
    uint8_t *base_;
};

} // namespace Media
} // namespace OHOS
#endif