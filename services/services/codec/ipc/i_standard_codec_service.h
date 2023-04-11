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

#ifndef I_STANDARD_CODEC_SERVICE_H
#define I_STANDARD_CODEC_SERVICE_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "avcodec_common.h"
#include "avcodec_info.h"
#include "avsharedmemory.h"
#include "surface.h"

namespace OHOS {
namespace Media {
class IStandardCodecService : public IRemoteBroker {
public:
    virtual ~IStandardCodecService() = default;

    virtual int32_t SetListenerObject(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t Init(AVCodecType type, bool isMimeType, const std::string &name) = 0;
    virtual int32_t Configure(const Format &format) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Flush() = 0;
    virtual int32_t Reset() = 0;
    virtual int32_t Release() = 0;
    virtual int32_t NotifyEos() = 0;
    virtual sptr<Surface> CreateInputSurface() = 0;
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;
    virtual std::shared_ptr<AVBufferElement> GetInputBuffer(uint32_t index) = 0;
    virtual int32_t QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) = 0;
    virtual std::shared_ptr<AVBufferElement> GetOutputBuffer(uint32_t index) = 0;
    virtual int32_t GetOutputFormat(Format &format) = 0;
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;
    virtual int32_t SetParameter(const Format &format) = 0;
    // virtual int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) = 0;

    virtual int32_t SetInputSurface(sptr<PersistentSurface> surface) = 0;
    virtual int32_t DequeueInputBuffer(uint32_t index, int64_t timetUs) = 0;
    virtual int32_t DequeueOutputBuffer(uint32_t index, int64_t timetUs) = 0;
    // virtual int32_t SetRenderedListener(const std::shared_ptr<AVCodecFrameRenderedListener> &listener) = 0;

    virtual int32_t DestroyStub() = 0;

    /**
     * IPC code ID
     */
    enum CodecServiceMsg {
        SET_LISTENER_OBJ = 0,
        INIT,
        CONFIGURE,
        START,
        STOP,
        FLUSH,
        RESET,
        RELEASE,
        NOTIFY_EOS,
        CREATE_INPUT_SURFACE,
        SET_OUTPUT_SURFACE,
        GET_INPUT_BUFFER,
        QUEUE_INPUT_BUFFER,
        GET_OUTPUT_BUFFER,
        GET_OUTPUT_FORMAT,
        RELEASE_OUTPUT_BUFFER,
        SET_PARAMETER,
        SET_INPUT_SURFACE,
        DEQUEUE_INPUT_BUFFER,
        DEQUEUE_OUTPUT_BUFFER,

        DESTROY_STUB
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardCodecService");
};
} // namespace Media
} // namespace OHOS
#endif // I_STANDARD_CODEC_SERVICE_H
