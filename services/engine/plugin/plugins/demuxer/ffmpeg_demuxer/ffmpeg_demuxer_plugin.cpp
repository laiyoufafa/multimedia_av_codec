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

#include "ffmpeg_demuxer_plugin.h"
#include "avcodec_errors.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "FFmpegDemuxerPlugin"};
}

namespace OHOS {
namespace AVCodec{
namespace Plugin {
namespace Ffmpeg {
FFmpegDemuxerPlugin::FFmpegDemuxerPlugin(size_t sourceAttr)
{
    formatContext_ = (AVFormatContext*)sourceAttr;
}

Status FFmpegDemuxerPlugin::AddSourceTrackByID(uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, 
                            ErrCode::AVCSERR_DEMUXER_FAILED, "formatContext_ is empty!");
    CHECK_AND_RETURN_RET_LOG(trackId > 0 && trackId < static_cast<int32_t>(formatContext_->nb_streames), 
                            ErrCode::AVCSERR_INVALID_VAL, "trackId is invalid!");

    OSAL::ScopedLock lock(mutex_);
    auto index = std::find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                              [trackId](int32_t selectedId) {return trackId == selectedId; });
    if (index == selectedTrackIds_.end()) {
        selectedTrackIds_.push_back(trackId);
    }
    return ErrCode::AVCSERR_OK;
}

Status FFmpegDemuxerPlugin::RemoveSourceTrackByID(uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(formatContext_ != nullptr, Status::ERROR_NULL_POINTER, "formatContext_ is empty");

    OSAL::ScopedLock lock(mutex_);
    auto index = find_if(selectedTrackIds_.begin(), selectedTrackIds_.end(),
                         [trackId](int32_t selectedId) {return trackId == selectedId; });
    if (index != selectedTrackIds_.end()) {
        selectedTrackIds_.erase(index);
    }

    return ErrCode::AVCSERR_OK;
}

} // Ffmpeg
} // Plugin
} // Media
} // OHOS
