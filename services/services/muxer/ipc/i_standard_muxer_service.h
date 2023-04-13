/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef I_STANDARD_MUXER_SERVICE_H
#define I_STANDARD_MUXER_SERVICE_H

#include "i_muxer_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace MediaAVCodec {
class IStandardMuxerService : public IRemoteBroker {
public:
    virtual ~IStandardMuxerService() = default;

    virtual int32_t Init() = 0;
    virtual int32_t SetLocation(float latitude, float longitude) = 0;
    virtual int32_t SetRotation(int32_t rotation) = 0;
    virtual int32_t SetParameter(const Format &generalFormat) = 0;
    virtual int32_t AddTrack(const Format &trackFormat) = 0;
    virtual int32_t Start() = 0;
    virtual int32_t WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info) = 0;
    virtual int32_t Stop() = 0;

    virtual int32_t DestroyStub() = 0;
    enum MuxerServiceMsg {
        INIT,
        SET_LOCATION,
        SET_ROTATION,
        SET_PARAMETER,
        ADD_TRACK,
        START,
        WRITE_SAMPLE_BUFFER,
        STOP,

        DESTROY_STUB,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardMuxerService");
};
}  // namespace MediaAVCodec
}  // namespace OHOS
#endif  // I_STANDARD_MUXER_SERVICE_H