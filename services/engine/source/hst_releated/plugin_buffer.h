/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PLUGIN_COMMON_BUFFER_H
#define HISTREAMER_PLUGIN_COMMON_BUFFER_H

#include <memory>
#include <map>
#include <vector>

#include "plugin_memory.h"
// #include "tag_map.h"
#include "securec.h"

namespace OHOS {
namespace Media {
namespace Plugin {
/// End of Stream Buffer Flag
#define BUFFER_FLAG_EOS 0x00000001
/// Video Key Frame Flag
#define BUFFER_FLAG_KEY_FRAME 0x00000002

// Align value template
template <typename T>
using MakeUnsigned = typename std::make_unsigned<T>::type;

template <typename T, typename U>
constexpr T AlignUp(T num, U alignment)
{
    return (alignment > 0) ? (static_cast<uint64_t>((num + static_cast<MakeUnsigned<T>>(alignment) - 1)) &
        static_cast<uint64_t>((~(static_cast<MakeUnsigned<T>>(alignment) - 1)))) :
        num;
}

/**
 * @enum Buffer Meta Type
 *
 * @since 1.0
 * @version 1.0
 */
enum struct BufferMetaType : uint32_t {
    AUDIO,      ///< Meta used to describe audio data
    VIDEO,      ///< Meta used to describe video data
};


/**
* @brief Buffer base class.
* Contains the data storage and metadata information of the buffer (buffer description information).
*
* @since 1.0
* @version 1.0
*/
class Buffer {
public:
    /// Construct an empty buffer.
    explicit Buffer(BufferMetaType type = BufferMetaType::AUDIO);

    /// Destructor
    ~Buffer() = default;

    static std::shared_ptr<Buffer> CreateDefaultBuffer(BufferMetaType type, size_t capacity,
                                                       std::shared_ptr<Allocator> allocator = nullptr,
                                                       size_t align = 1);

    std::shared_ptr<Memory> WrapMemory(uint8_t* data, size_t capacity, size_t size);

    std::shared_ptr<Memory> WrapMemoryPtr(std::shared_ptr<uint8_t> data, size_t capacity, size_t size);

    std::shared_ptr<Memory> AllocMemory(std::shared_ptr<Allocator> allocator, size_t capacity, size_t align = 1);

    uint32_t GetMemoryCount();

    std::shared_ptr<Memory> GetMemory(uint32_t index = 0);

    void Reset();

    /// no memory in the buffer.
    bool IsEmpty();

    /// track index.
    uint32_t trackID;

    /// presentation timestamp of the buffer based on {@link HST_TIME_BASE}.
    int64_t pts;

    /// decoding timestamp of the buffer based on {@link HST_TIME_BASE}.
    int64_t dts;

    /// duration in time of the buffer data based on {@link HST_TIME_BASE}.
    int64_t duration;

    /// flag of the buffer, which is used to record extra information.
    /// @see BUFFER_FLAG_EOS
    uint64_t flag;

private:
    /// Data described by this buffer.
    std::vector<std::shared_ptr<Memory>> data {};

};
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_COMMON_BUFFER_H
