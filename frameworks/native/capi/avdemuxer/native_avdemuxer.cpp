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

#include <memory>
#include "avdemuxer.h"
#include "native_avcodec_demuxer.h"
#include "native_avmagic.h"
#include "media_error.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVDemuxer"}
}

using namespase OHOS::AVCodec;

struct DemuxerObject : public OH_AVDemuxer {
    explicit DemuxerObject(const std::shared_ptr<AVDemuxer> &demuxer)
        : demuxer_(demuxer) {}
    ~DemuxerObject() = default;

    const std::shared_ptr<OH_AVDemuxer> demuxer_;
};

struct OH_AVDemuxer *OH_AVDemuxer_CreateWithSource(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "input source is nullptr!");

    std::shared_ptr<AVDemuxer> demuxer = DemuxerFactory::CreateWithSource(source);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, nullptr, "failed to DemuxerFactory::CreateWithSource!");

    struct DemuxerObject *object = new(std::nothrow) DemuxerObject(demuxer);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "failed to new DemuxerObject!");

    return object;
}

OH_AVErrCode OH_AVDemuxer_Destroy(OH_AVDemuxer *demuxer)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "input demuxer is nullptr!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);

    if (demuxerObj != nullptr && demuxerObj->demuxer_ != nullptr) {
        int32_t ret = demuxerObj->demuxer_->Destroy();
        if (ret != MSERR_OK) {
            MEDIA_LOGE("demuxer Destroy failed!");
            delete demuxer;
            return AV_ERR_OPERATE_NOT_PERMIT;
        }
    } else {
        MEDIA_LOGE("demuxer_ is nullptr!");
    }

    delete demuxer;
    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_AddSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(trackId >=0, AV_ERR_INVALID_VAL, "input trackId is negative!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, nullptr, "demuxer_ is nullptr!");

    int32_t ret = demuxerObj->demuxer_->AddSourceTrackByID(trackId);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer AddSourceTrackByID failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_RemoveSourceTrackByID(OH_AVDemuxer *demuxer, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(trackId >=0, AV_ERR_INVALID_VAL, "input trackId is negative!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, nullptr, "demuxer_ is nullptr!");

    int32_t ret = demuxerObj->demuxer_->RemoveSourceTrackByID(trackId);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer RemoveSourceTrackByID failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_CopyCurrentSampleToBuf(OH_AVDemuxer *demuxer, OH_AVBufferElement *buffer, OH_AVCodecBufferAttr *bufferInfo)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, AV_ERR_INVALID_VAL, "input buffer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(attr != nullptr, AV_ERR_INVALID_VAL, "input attr is nullptr!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, nullptr, "demuxer_ is nullptr!")

    int32_t ret = demuxerObj->demuxer_->CopyCurrentSampleToBuf(buffer, attr);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer RemoveSourceTrackByID failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVDemuxer_SeekToTimeStamp(OH_AVDemuxer *demuxer, int64_t mSeconds, OH_AVSeekMode mode)
{
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, AV_ERR_INVALID_VAL, "input demuxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(mSeconds >=0, AV_ERR_INVALID_VAL, "input mSeconds is negative!");

    struct DemuxerObject *demuxerObj = reinterpret_cast<DemuxerObject *>(demuxer);
    CHECK_AND_RETURN_RET_LOG(demuxerObj->demuxer_ != nullptr, nullptr, "demuxer_ is nullptr!")

    int32_t ret = demuxerObj->demuxer_->SeekToTimeStamp(mSeconds, mode);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "demuxer SeekToTimeStamp failed!");

    return AV_ERR_OK;
}