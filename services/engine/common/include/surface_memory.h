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

#ifndef FCODEC_PLUGIN_COMMON_SURFACE_MEMORY_H
#define FCODEC_PLUGIN_COMMON_SURFACE_MEMORY_H

// #if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#include <shared_mutex>
#include "surface_allocator.h"
#include "type_cast_ext.h"
// #include "task.h"

namespace OHOS {
namespace Media {
namespace Codec {
class SurfaceMemory : public Memory {
public:
    ~SurfaceMemory();

    sptr<SurfaceBuffer> GetSurfaceBuffer();

    int32_t GetFlushFence();

    BufferHandle *GetBufferHandle();

    uint32_t GetSurfaceBufferStride();

    void SetNeedRender(bool needRender);
    void ReleaseSurfaceBuffer();
private:
    explicit SurfaceMemory(size_t capacity, std::shared_ptr<Allocator> allocator = nullptr, size_t align = 1);

    uint8_t *GetRealAddr() const override;

private:
    void AllocSurfaceBuffer();

    // mutable Mutex memMutex_ {};
    mutable std::mutex memMutex_ {};

    /// Surface buffer
    sptr<SurfaceBuffer> surfaceBuffer_ {nullptr};

    std::shared_ptr<SurfaceAllocator> surfaceAllocator_ {nullptr};

    /// the fence fd for Surface
    int32_t fence_ {-1};

    size_t bufferSize_ {0};

    uint32_t stride_ {0};

    bool needRender_ {false};

    friend class Buffer;
    friend class FBuffer;
};
} // namespace Codec
} // namespace Media
} // namespace OHOS

// #endif
#endif // FCODEC_PLUGIN_COMMON_SURFACE_MEMORY_H
