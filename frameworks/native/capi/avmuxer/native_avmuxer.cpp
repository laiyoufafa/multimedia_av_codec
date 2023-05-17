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

#include "native_avmuxer.h"
#include "native_avmagic.h"
#include "avmuxer.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVMuxer"};
}

using namespace OHOS::Media;

struct AVMuxerObject : public OH_AVMuxer {
    explicit AVMuxerObject(const std::shared_ptr<AVMuxer> &muxer)
        : OH_AVMuxer(AVMagic::AVCODEC_MAGIC_AVMUXER), muxer_(muxer) {}
    ~AVMuxerObject() = default;

    const std::shared_ptr<AVMuxer> muxer_;
};

struct OH_AVMuxer *OH_AVMuxer_Create(int32_t fd, OH_AVOutputFormat format)
{
    CHECK_AND_RETURN_RET_LOG(fd >= 0, nullptr, "fd %{public}d is error!", fd);
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd, static_cast<OutputFormat>(format));
    CHECK_AND_RETURN_RET_LOG(avmuxer != nullptr, nullptr, "create muxer failed!");
    struct AVMuxerObject *object = new(std::nothrow) AVMuxerObject(avmuxer);
    return object;
}

OH_AVErrCode OH_AVMuxer_SetRotation(OH_AVMuxer *muxer, int32_t rotation)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");

    struct AVMuxerObject *object = reinterpret_cast<AVMuxerObject *>(muxer);
    CHECK_AND_RETURN_RET_LOG(object->muxer_ != nullptr, AV_ERR_INVALID_VAL, "muxer_ is nullptr!");

    int32_t ret = object->muxer_->SetRotation(rotation);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "muxer_ SetRotation failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVMuxer_AddTrack(OH_AVMuxer *muxer, int32_t *trackIndex, OH_AVFormat *trackFormat)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");
    CHECK_AND_RETURN_RET_LOG(trackIndex != nullptr, AV_ERR_INVALID_VAL, "input track index is nullptr!");
    CHECK_AND_RETURN_RET_LOG(trackFormat != nullptr, AV_ERR_INVALID_VAL, "input track format is nullptr!");
    CHECK_AND_RETURN_RET_LOG(trackFormat->magic_ == AVMagic::AVCODEC_MAGIC_FORMAT, AV_ERR_INVALID_VAL, "magic error!");

    struct AVMuxerObject *object = reinterpret_cast<AVMuxerObject *>(muxer);
    CHECK_AND_RETURN_RET_LOG(object->muxer_ != nullptr, AV_ERR_INVALID_VAL, "muxer_ is nullptr!");

    int32_t ret = object->muxer_->AddTrack(*trackIndex, trackFormat->format_);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "muxer_ AddTrack failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVMuxer_Start(OH_AVMuxer *muxer)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");

    struct AVMuxerObject *object = reinterpret_cast<AVMuxerObject *>(muxer);
    CHECK_AND_RETURN_RET_LOG(object->muxer_ != nullptr, AV_ERR_INVALID_VAL, "muxer_ is nullptr!");

    int32_t ret = object->muxer_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "muxer_ Start failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVMuxer_WriteSample(OH_AVMuxer *muxer,
                                    uint32_t trackIndex,
                                    OH_AVMemory *sample,
                                    OH_AVCodecBufferAttr info)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");
    CHECK_AND_RETURN_RET_LOG(sample != nullptr, AV_ERR_INVALID_VAL, "Invalid memory");
    CHECK_AND_RETURN_RET_LOG(info.offset >= 0 && info.pts >= 0 && info.size >= 0,
        AV_ERR_INVALID_VAL, "Invalid memory");

    struct AVMuxerObject *object = reinterpret_cast<AVMuxerObject *>(muxer);
    CHECK_AND_RETURN_RET_LOG(object->muxer_ != nullptr, AV_ERR_INVALID_VAL, "muxer_ is nullptr!");

    TrackSampleInfo sampleInfo;
    sampleInfo.trackIndex = trackIndex;
    sampleInfo.timeUs = info.pts;
    sampleInfo.size = info.size;
    sampleInfo.offset = info.offset;
    sampleInfo.flags = info.flags;

    int32_t ret = object->muxer_->WriteSample(sample->memory_, sampleInfo);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "muxer_ WriteSample failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVMuxer_Stop(OH_AVMuxer *muxer)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");

    struct AVMuxerObject *object = reinterpret_cast<AVMuxerObject *>(muxer);
    CHECK_AND_RETURN_RET_LOG(object->muxer_ != nullptr, AV_ERR_INVALID_VAL, "muxer_ is nullptr!");

    int32_t ret = object->muxer_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "muxer_ Stop failed!");

    return AV_ERR_OK;
}

OH_AVErrCode OH_AVMuxer_Destroy(OH_AVMuxer *muxer)
{
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AV_ERR_INVALID_VAL, "input muxer is nullptr!");
    CHECK_AND_RETURN_RET_LOG(muxer->magic_ == AVMagic::AVCODEC_MAGIC_AVMUXER, AV_ERR_INVALID_VAL, "magic error!");

    delete muxer;

    return AV_ERR_OK;
}