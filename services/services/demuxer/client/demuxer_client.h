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
#ifndef DEMUXER_CLIENT_H
#define DEMUXER_CLIENT_H

#include "i_demuxer_service.h"
#include "i_standard_demuxer_service.h"

namespace OHOS {
namespace Media {
class DemuxerClient : public IDemuxerService, public NoCopyable {
public:
    static std::shared_ptr<DemuxerClient> Create(const sptr<IStandardDemuxerService> &ipcProxy);
    explicit DemuxerClient(const sptr<IStandardDemuxerService> &ipcProxy);
    ~DemuxerClient();

    // 业务
    int32_t Init(uint64_t sourceAddr) override;
    int32_t SelectSourceTrackByID(uint32_t index) override;
    int32_t UnselectSourceTrackByID(uint32_t index) override;
    int32_t CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo) override;
    int32_t SeekToTime(int64_t mSeconds, const AVSeekMode mode) override;
    
    void AVCodecServerDied();
private:
    std::mutex mutex_;
    sptr<IStandardDemuxerService> demuxerProxy_ = nullptr;
};

}  // namespace Media
}  // namespace OHOS
#endif  // DEMUXER_CLIENT_H