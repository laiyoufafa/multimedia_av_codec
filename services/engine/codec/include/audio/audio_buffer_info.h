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
#include "share_memory.h"

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

    ~AudioBufferInfo() = default;

    std::shared_ptr<ShareMemory> GetBuffer() const noexcept;

    std::shared_ptr<ShareMemory> GetMetadata() const noexcept;

    bool IsHasMetaData() const noexcept;

    bool Reset();

    bool SetBufferOwned();

    bool IsAvilable() const noexcept;

    BufferStatus GetStatus() const noexcept;

    bool CheckIsEos() const noexcept;

    void SetEos(bool eos);

    void SetBufferAttr(const AVCodecBufferInfo &attr);

    AVCodecBufferInfo GetBufferAttr() const noexcept;

    AVCodecBufferFlag GetFlag() const noexcept;

private:
    bool isHasMeta_;
    bool isEos_;
    std::atomic<BufferStatus> status_;
    uint32_t bufferSize_;
    uint32_t metaSize_;
    std::string_view name_;
    std::shared_ptr<ShareMemory> buffer_;
    std::shared_ptr<ShareMemory> metadata_;
    AVCodecBufferInfo info_;
    AVCodecBufferFlag flag_;
};

} // namespace Media
} // namespace OHOS
#endif