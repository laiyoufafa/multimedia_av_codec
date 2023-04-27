/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");;
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

#include "demuxer.h"
#include "avcodec_errors.h"
#include "demuxer_plugin.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "Demuxer"};
}

namespace OHOS {
namespace Media {
namespace Plugin {
Demuxer::Demuxer (uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<DemuxerPlugin> plugin)
    : pkgVersion_(pkgVer), apiVersion_(apiVer), demuxer_(std::move(plugin)) {}

int32_t Demuxer::SelectSourceTrackByID(uint32_t trackIndex){
    return demuxer_->SelectSourceTrackByID(trackIndex);
}

int32_t Demuxer::UnselectSourceTrackByID(uint32_t trackIndex){
    return demuxer_->UnselectSourceTrackByID(trackIndex);
}

int32_t Demuxer::CopyNextSample(uint32_t &trackIndex, uint8_t *buffer, AVCodecBufferInfo &bufferInfo){
    return demuxer_->CopyNextSample(trackIndex, buffer, bufferInfo);
}

int32_t Demuxer::SeekToTime(int64_t mSeconds, AVSeekMode mode){
    int32_t ret = demuxer_->SeekToTime(mSeconds, mode);
    return ret;
}

} // namespace Plugin
} // namespace Media
} // namespace OHOS