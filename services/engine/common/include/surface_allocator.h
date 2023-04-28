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

#ifndef FCODEC_PLUGIN_COMMON_SURFACE_ALLOCATOR_H
#define FCODEC_PLUGIN_COMMON_SURFACE_ALLOCATOR_H

// #if !defined(OHOS_LITE) && defined(VIDEO_SUPPORT)

#include "plugin_memory.h"
#include "plugin_types.h"
#include "refbase.h"
#include "surface/surface.h"

namespace OHOS {
namespace Media {
namespace Codec {
class SurfaceAllocator : public Allocator {
public:
    explicit SurfaceAllocator(sptr<Surface> surface = nullptr);
    ~SurfaceAllocator() override = default;

    sptr<SurfaceBuffer> AllocSurfaceBuffer();
    void ReleaseSurfaceBuffer(sptr<SurfaceBuffer>& surfaceBuffer, bool needRender);

    void* Alloc(size_t size) override;
    void Free(void* ptr) override; // NOLINT: void*

    void Config(int32_t width, int32_t height, uint64_t usage, int32_t format, int32_t strideAlign, int32_t timeout);
    void SetScaleType(VideoScaleType videoScaleType);
    void UpdateSurfaceBufferScaleMode(sptr<SurfaceBuffer>& surfaceBuffer);
private:
    sptr<Surface> surface_ {nullptr};
    BufferRequestConfig requestConfig_;
    ScalingMode scalingMode_ {ScalingMode::SCALING_MODE_SCALE_TO_WINDOW};
};
} // namespace Codec
} // namespace Media
} // namespace OHOS

// #endif
#endif // FCODEC_PLUGIN_COMMON_SURFACE_BUFFER_H
