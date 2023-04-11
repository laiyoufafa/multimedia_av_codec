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

#include "avcodec_impl.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodec> CodecFactory::CreateByMime(const std::string &mime, bool encoder)
{
    std::shared_ptr<AVCodecImpl> impl = std::make_shared<AVCodecImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new AVCodecImpl");

    AVCodecType codeType = encoder ? AVCODEC_TYPE_ENCODER : AVCODEC_TYPE_DECODER;
    int32_t ret = impl->Init(codeType, true, mime);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVCodecImpl");

    return impl;
}

std::shared_ptr<AVCodec> CodecFactory::CreateByName(const std::string &name)
{
    std::shared_ptr<AVCodecImpl> impl = std::make_shared<AVCodecImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to new AVCodecImpl");

    int32_t ret = impl->Init(AVCODEC_TYPE_NONE, false, name);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVCodecImpl");

    return impl;
}

int32_t AVCodecImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    codecService_ = AVCodecServiceFactory::GetInstance().CreateCodecService();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, MSERR_UNKNOWN, "failed to create avcodec service");

    return codecService_->Init(type, isMimeType, name);
}

AVCodecImpl::AVCodecImpl()
{
    AVCODEC_LOGD("AVCodecImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecImpl::~AVCodecImpl()
{
    if (codecService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyAVCodecService(codecService_);
        codecService_ = nullptr;
    }
    AVCODEC_LOGD("AVCodecImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecImpl::Configure(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Configure(format);
}

int32_t AVCodecImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Start();
}

int32_t AVCodecImpl::Stop()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Stop();
}

int32_t AVCodecImpl::Flush()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Flush();
}

int32_t AVCodecImpl::Reset()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Reset();
}

int32_t AVCodecImpl::Release()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Release();
}

int32_t AVCodecImpl::SetOutputSurface(sptr<Surface> surface)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->SetOutputSurface(surface);
}

std::shared_ptr<AVBufferElement> AVCodecImpl::GetInputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, nullptr, "service died");
    return codecService_->GetInputBuffer(index);
}

int32_t AVCodecImpl::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVBufferElement> AVCodecImpl::GetOutputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, nullptr, "service died");
    return codecService_->GetOutputBuffer(index);
}

int32_t AVCodecImpl::GetOutputFormat(Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->GetOutputFormat(format);
}

int32_t AVCodecImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->ReleaseOutputBuffer(index, render);
}

int32_t AVCodecImpl::SetParameter(const Format &format)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->SetParameter(format);
}

int32_t AVCodecImpl::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AVCS_ERR_INVALID_VAL, "callback is nullptr");
    return codecService_->SetCallback(callback);
}

sptr<Surface> AVCodecImpl::CreateInputSurface()
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, nullptr, "service died");
    surface_ = codecService_->CreateInputSurface();
    return surface_;
}

// int32_t AVCodecImpl::SetInputSurface(sptr<PersistentSurface> surface)
// {
//     CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, MSERR_INVALID_OPERATION, "service died");
//     return codecService_->SetInputSurface(surface);
// }

int32_t AVCodecImpl::DequeueInputBuffer(size_t *index, int64_t timetUs)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->DequeueInputBuffer(surface);
}

int32_t AVCodecImpl::DequeueOutputBuffer(size_t *index, int64_t timetUs)
{
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->DequeueOutputBuffer(surface);
}


} // namespace Media
} // namespace OHOS
