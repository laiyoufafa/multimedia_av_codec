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

#include "avcodec_audio_decoder_impl.h"
#include "i_avcodec_service.h"
#include "avcodec_log.h"
#include "avcodec_errors.h"
#include "avcodec_dfx.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecAudioDecoderImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVCodecAudioDecoder> AudioDecoderFactory::CreateByMime(const std::string &mime)
{
    AVCODEC_SYNC_TRACE;
    std::shared_ptr<AVCodecAudioDecoderImpl> impl = std::make_shared<AVCodecAudioDecoderImpl>();

    int32_t ret = impl->Init(AVCODEC_TYPE_AUDIO_DECODER, true, mime);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVCodecAudioDecoderImpl");

    return impl;
}

std::shared_ptr<AVCodecAudioDecoder> AudioDecoderFactory::CreateByName(const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    std::shared_ptr<AVCodecAudioDecoderImpl> impl = std::make_shared<AVCodecAudioDecoderImpl>();

    int32_t ret = impl->Init(AVCODEC_TYPE_AUDIO_DECODER, false, name);
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "failed to init AVCodecAudioDecoderImpl");

    return impl;
}

int32_t AVCodecAudioDecoderImpl::Init(AVCodecType type, bool isMimeType, const std::string &name)
{
    AVCODEC_SYNC_TRACE;
    codecService_ = AVCodecServiceFactory::GetInstance().CreateCodecService();
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_UNKNOWN, "failed to create codec service");

    return codecService_->Init(type, isMimeType, name);
}

AVCodecAudioDecoderImpl::AVCodecAudioDecoderImpl()
{
    AVCODEC_LOGD("AVCodecAudioDecoderImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecAudioDecoderImpl::~AVCodecAudioDecoderImpl()
{
    if (codecService_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyCodecService(codecService_);
        codecService_ = nullptr;
    }
    AVCODEC_LOGD("AVCodecAudioDecoderImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVCodecAudioDecoderImpl::Configure(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Configure(format);
}

int32_t AVCodecAudioDecoderImpl::Prepare()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return AVCS_ERR_OK;
}

int32_t AVCodecAudioDecoderImpl::Start()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Start();
}

int32_t AVCodecAudioDecoderImpl::Stop()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Stop();
}

int32_t AVCodecAudioDecoderImpl::Flush()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Flush();
}

int32_t AVCodecAudioDecoderImpl::Reset()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Reset();
}

int32_t AVCodecAudioDecoderImpl::Release()
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->Release();
}

std::shared_ptr<AVSharedMemory> AVCodecAudioDecoderImpl::GetInputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, nullptr, "service died");
    return codecService_->GetInputBuffer(index);
}

int32_t AVCodecAudioDecoderImpl::QueueInputBuffer(uint32_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->QueueInputBuffer(index, info, flag);
}

std::shared_ptr<AVSharedMemory> AVCodecAudioDecoderImpl::GetOutputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, nullptr, "service died");
    return codecService_->GetOutputBuffer(index);
}

int32_t AVCodecAudioDecoderImpl::GetOutputFormat(Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->GetOutputFormat(format);
}

int32_t AVCodecAudioDecoderImpl::ReleaseOutputBuffer(uint32_t index)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->ReleaseOutputBuffer(index);
}

int32_t AVCodecAudioDecoderImpl::SetParameter(const Format &format)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    return codecService_->SetParameter(format);
}

int32_t AVCodecAudioDecoderImpl::SetCallback(const std::shared_ptr<AVCodecCallback> &callback)
{
    AVCODEC_SYNC_TRACE;
    CHECK_AND_RETURN_RET_LOG(codecService_ != nullptr, AVCS_ERR_INVALID_OPERATION, "service died");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, AVCS_ERR_INVALID_VAL, "callback is nullptr");
    return codecService_->SetCallback(callback);
}
} // namespace Media
} // namespace OHOS
