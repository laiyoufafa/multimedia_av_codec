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

#ifndef AV_CODEC_BUFFER_INFO_H
#define AV_CODEC_BUFFER_INFO_H

#include "avcodec_common.h"
#include "avsharedmemorybase.h"
#include <atomic>
#include <memory>
#include <vector>

namespace OHOS {
namespace Media {

enum class BufferStatus {
    IDEL,
    OWNE_BY_CLIENT,
};

class AudioBufferInfo {
public:
    AudioBufferInfo(const uint32_t &bufferSize, const std::string_view &name, const uint32_t &metaSize = 0,
                    size_t align = 1);

    ~AudioBufferInfo();

    std::shared_ptr<AVSharedMemoryBase> GetBuffer() const noexcept;

    std::shared_ptr<AVSharedMemoryBase> GetMetadata() const noexcept;

    bool IsHasMetaData() const noexcept;

    bool ResetBuffer();

    bool SetBufferOwned();

    bool IsAvilable() const noexcept;

    BufferStatus GetStatus() const noexcept;

    bool CheckIsEos() const noexcept;

    void SetEos(bool eos);

    void SetBufferAttr(const AVCodecBufferInfo &attr);

    AVCodecBufferInfo GetBufferAttr() const noexcept;

    AVCodecBufferFlag GetFlag() const noexcept;

    size_t WriteBuffer(const uint8_t *in, size_t writeSize);

    size_t WriteMetadata(const uint8_t *in, size_t writeSize);

    size_t ReadBuffer(uint8_t *out, size_t readSize);

    size_t ReadMetadata(uint8_t *out, size_t readSize);

private:
    size_t Write(const uint8_t *in, size_t writeSize, const size_t &bufferSize);

    size_t Read(uint8_t *out, size_t readSize, const size_t &useSize);

private:
    bool isHasMeta_;
    bool isEos_;
    std::atomic<BufferStatus> status_;
    uint32_t bufferSize_;
    uint32_t bufferUseSize_;
    uint32_t metaSize_;
    std::string_view name_;
    std::shared_ptr<AVSharedMemoryBase> buffer_;
    std::shared_ptr<AVSharedMemoryBase> metadata_;
    AVCodecBufferInfo info_;
    AVCodecBufferFlag flag_;
};

} // namespace Media
} // namespace OHOS
#endif