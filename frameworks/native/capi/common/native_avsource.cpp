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

#include <string>
#include "avsource.h"
#include "native_avmagic.h"
#include "native_avsource.h"
#include "media_error.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "NativeAVSource"}
}

using namespase OHOS::AVCodec;

struct SourceObject : public OH_AVSource {
    explicit SourceObject(const std::shared_ptr<AVSource> &source)
        : source_(source) {}
    ~SourceObject() = default;

    const std::shared_ptr<OH_AVSource> source_;
};

struct OH_AVSource *OH_AVSource_CreateWithURI(char *uri)
{
    CHECK_AND_RETURN_RET_LOG(uri != nullptr, nullptr, "input uri is nullptr!");

    std::shared_ptr<AVSource> source = std::make_shared_ptr<AVSource>(uri);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "failed to new Source!");

    struct SourceObject *object = new(std::nothrow) SourceObject(source);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "failed to new SourceObject!");
    
    return object;
}

struct OH_AVSource *OH_AVSource_CreateWithFd(int32_t fd, int64_t offset, int64_t size)
{
    // 0-err, 1-in, 2-out
    CHECK_AND_RETURN_RET_LOG(fd > 2, AV_ERR_INVALID_VAL, "input fd is illegal, fd must be greater than 2!");
    CHECK_AND_RETURN_RET_LOG(size >= 0, AV_ERR_INVALID_VAL, "input size is negative");

    std::String uri = std::format("fd://()?offset=()&size=()", fd, offset, size)
    CHECK_AND_RETURN_RET_LOG(uri != nullptr, nullptr, "Format uri is nullptr!");

    std::shared_ptr<AVSource> source = std::make_shared_ptr<AVSource>(uri.c_str());
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "failed to new Source!");

    struct SourceObject *object = new(std::nothrow) SourceObject(source);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "failed to new SourceObject!");

    return object;
}

OH_AVErrCode OH_AVSource_Destroy(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, AV_ERR_INVALID_VAL, "input source is nullptr!");

    struct SourceObject *sourceObj = reinterpret_cast<SourceObject *>(source);

    if (sourceObj != nullptr && sourceObj->source_ != nullptr) {
        int32_t ret = sourceObj->source_->Destroy();
        if (ret != MSERR_OK) {
            AVCODEC_LOGE("source Destroy failed!");
            delete source;
            return AV_ERR_OPERATE_NOT_PERMIT;
        }
    } else {
        AVCODEC_LOGE("source_ is nullptr!");
    }

    delete source;
    return AV_ERR_OK;
}

uint32_t OH_AVSource_GetTrackCount(OH_AVSource *source)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, AV_ERR_INVALID_VAL, "input source is nullptr!");
    
    struct SourceObject *sourceObj = reinterpret_cast<SourceObject *>(source);
    CHECK_AND_RETURN_RET_LOG(sourceObj->source_ != nullptr, nullptr, "source_ is nullptr!");

    int32_t ret = sourceObj->source_->GetTrackCount();
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "source GetTrackCount failed!");

    return AV_ERR_OK;
}

struct OH_AVSourceTrack *OH_AVSource_LoadSourceTrackByID(OH_AVSource *source, uint32_t trackId)
{
    CHECK_AND_RETURN_RET_LOG(source != nullptr, AV_ERR_INVALID_VAL, "input source is nullptr!");
    CHECK_AND_RETURN_RET_LOG(trackId >=0, AV_ERR_INVALID_VAL, "input trackId is negative!");

    struct SourceObject *sourceObj = reinterpret_cast<SourceObject *>(source);
    CHECK_AND_RETURN_RET_LOG(sourceObj->source_ != nullptr, nullptr, "source_ is nullptr!");

    int32_t ret = sourceObj->source_->LoadSourceTrackByID(trackId);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "source LoadSourceTrackByID failed!");

    return AV_ERR_OK;
}

struct SourceTrackObject : public OH_AVSourceTrack {
    explicit SourceTrackObject(const std::shared_ptr<AVSourceTrack> &sourceTrack)
        : sourceTrack_(sourceTrack) {}
    ~SourceTrackObject() = default;

    const std::shared_ptr<OH_AVSourceTrack> sourceTrack_;
};

OH_AVErrCode OH_AVSourceTrack_SetParameter(OH_AVSourceTrack *sourceTrack, OH_AVFormat *param)
{
    CHECK_AND_RETURN_RET_LOG(sourceTrack != nullptr, AV_ERR_INVALID_VAL, "input sourceTrack is nullptr!");
    CHECK_AND_RETURN_RET_LOG(param != nullptr, AV_ERR_INVALID_VAL, "input param is nullptr!");
    CHECK_AND_RETURN_RET_LOG(param->format_ != nullptr, AV_ERR_INVALID_VAL, "input param->format is nullptr!");

    struct SourceTrackObject *sourceTrackObj = reinterpret_cast<SourceTrackObject *>(sourceTrack);
    CHECK_AND_RETURN_RET_LOG(sourceTrackObj->sourceTrack_ != nullptr, nullptr, "sourceTrack_ is nullptr!");

    int32_t ret = sourceTrackObj->sourceTrack_->SetParameter(sourceTrack, param->format_);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "sourceTrack SetParameter failed!");

    return AV_ERR_OK;
}

OH_AVFormat *OH_AVSourceTrack_GetTrackFormat(OH_AVSourceTrack *sourceTrack)
{
    CHECK_AND_RETURN_RET_LOG(sourceTrack != nullptr, AV_ERR_INVALID_VAL, "input sourceTrack is nullptr!");

    struct SourceTrackObject *sourceTrackObj = reinterpret_cast<SourceTrackObject *>(sourceTrack);
    CHECK_AND_RETURN_RET_LOG(sourceTrackObj->sourceTrack_ != nullptr, nullptr, "sourceTrack_ is nullptr!");
    
    Format format;
    int32_t ret = sourceTrackObj->sourceTrack_->GetTrackFormat(format);
    CHECK_AND_RETURN_RET_LOG(ret != MSERR_OK, AV_ERR_OPERATE_NOT_PERMIT, "sourceTrack GetTrackFormat failed!");

    OH_AVFormat *avFormat = OH_AVFormat_Create();
    avFormat->format_ = format;
    
    return avFormat;
}