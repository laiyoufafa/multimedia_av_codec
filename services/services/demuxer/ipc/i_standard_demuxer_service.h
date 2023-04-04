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

#ifndef I_STANDARD_AVMUXER_SERVICE_H
#define I_STANDARD_AVMUXER_SERVICE_H


#include "iremote_proxy.h"
#include "i_demuxer_service.h"

namespace OHOS {
namespace AVCodec {
class IStandardDemuxerService : public IRemoteBroker {
public:
    virtual ~IStandardDemuxerService() = default;

    // 业务
    
    virtual int32_t AddSourceTrackByID(uint32_t index) = 0;
    virtual int32_t RemoveSourceTrackByID(uint32_t index) = 0;
    virtual int32_t CopyCurrentSampleToBuf(AVBufferElement *buffer, AVCodecBufferInfo *bufferInfo) = 0;
    virtual int32_t SeekToTimeStamp(int64_t mSeconds, const SeekMode mode) = 0;

    virtual int32_t DestroyStub() = 0;

    enum DemuxerServiceMsg {
        ADD_SOURCE_TRACK_BY_ID,
        REMOVE_SOURCE_TRACK_BY_ID,
        COPY_CURRENT_SAMPLE_TO_BUF,
        SEEK_TO_TIME_STAMP,

        DESTROY,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardDemuxerService");
};

}  // namespace AVCodec
}  // namespace OHOS
#endif  // I_STANDARD_AVMUXER_SERVICE_H