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

#ifndef SURFACE_INNER_MOCK_H
#define SURFACE_INNER_MOCK_H

#include "common_mock.h"
#include "window.h"

namespace OHOS {
namespace Media {
class SurfaceInnerMock : public SurfaceMock {
public:
    explicit SurfaceInnerMock(sptr<Surface> &surface) : surface_(surface) {}
    SurfaceInnerMock() = default;
    ~SurfaceInnerMock();
    sptr<Surface> GetSurface();

private:
    sptr<Surface> surface_ = nullptr;
    sptr<Rosen::Window> window_ = nullptr;
};
} // namespace Media
} // namespace OHOS
#endif // SURFACE_INNER_MOCK_H