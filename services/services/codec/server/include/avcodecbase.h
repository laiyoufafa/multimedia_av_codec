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
#ifndef AVCODECBASE_H
#define AVCODECBASE_H

#include <string>
#include "avcodec_common.h"
#include "surface.h"

namespace OHOS {
namespace AVCodec {
class AVCodecBase {
    static std::shared_ptr<AVCodecBase> Create(const std::string& name);
    static std::shared_ptr<AVCodecBase> Create(bool isEncoder,const std::string& mime);
    virtual int32_t SetCallback(const std::shared_ptr<AVCodecCallBack>& callback) = 0;
    virtual int32_t Configure(const Format& format) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t Pause() = 0;
    virtual int32_t Resume() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Flush() = 0;
    virtual int32_t Reset() = 0;
    virtual int32_t Release() = 0;
    virtual int32_t NotifyEos() = 0;
    virtual int32_t SetParameter(const Format& format) = 0;
    virtual int32_t GetOutputFormat(Format& format) = 0;
    virtual sptr<Surface> CreateInputSurface() = 0;
    virtual int32_t SetInputSurface(sptr<Surface> surface) = 0;
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;
    virtual std::shared_ptr<AVBufferElement> GetInputBuffer(size_t index) = 0;
    virtual int32_t QueueInputBuffer(size_t index, const AVCodecBufferInfo& info) = 0;
    virtual std::shared_ptr<AVBufferElement> GetOutputBuffer(size_t index) = 0;
    virtual int32_t RenderOutputBuffer(size_t index) = 0;
    virtual int32_t ReleaseOutputBuffer(size_t index) = 0;
    virtual int32_t SignalRequestIDRFrame() = 0;
};
} // namespace AVCodec
} // namespace OHOS
#endif // AVCODECBASE_H