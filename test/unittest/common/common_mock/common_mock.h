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

#ifndef COMMON_MOCK_H
#define COMMON_MOCK_H

#include <string>
#include "avformat_mock.h"
#include "native_avcodec_base.h"
#include "nocopyable.h"
#include "surface.h"

namespace OHOS {
namespace MediaAVCodec {
class SurfaceMock : public NoCopyable {
public:
    virtual ~SurfaceMock() = default;
};

class AVMemoryMock : public NoCopyable {
public:
    virtual ~AVMemoryMock() = default;
    virtual uint8_t *GetAddr() const = 0;
    virtual int32_t GetSize() const = 0;
    virtual uint32_t GetFlags() const = 0;
};

class AVCodecCallbackMock : public NoCopyable {
public:
    virtual ~AVCodecCallbackMock() = default;
    virtual void OnError(int32_t errorCode) = 0;
    virtual void OnStreamChanged(std::shared_ptr<FormatMock> format) = 0;
    virtual void OnNeedInputData(uint32_t index, std::shared_ptr<AVMemoryMock> data) = 0;
    virtual void OnNewOutputData(uint32_t index, std::shared_ptr<AVMemoryMock> data, OH_AVCodecBufferAttr attr) = 0;
};

class __attribute__((visibility("default"))) SurfaceMockFactory {
public:
    static std::shared_ptr<SurfaceMock> CreateSurface();
    static std::shared_ptr<SurfaceMock> CreateSurface(sptr<Surface> &surface);

private:
    SurfaceMockFactory() = delete;
    ~SurfaceMockFactory() = delete;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // COMMON_MOCK_H