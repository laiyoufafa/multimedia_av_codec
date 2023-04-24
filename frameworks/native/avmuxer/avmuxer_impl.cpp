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

#include "avmuxer_impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <functional>
#include <sys/types.h>
#include "securec.h"
#include "avcodec_log.h"
#include "avsharedmemorybase.h"
#include "avcodec_dfx.h"
#include "i_avcodec_service.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVMuxerImpl"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<AVMuxer> AVMuxerFactory::CreateAVMuxer(int32_t fd, AVOutputFormat format)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG((fcntl(fd, F_GETFL, 0) & O_RDWR) == O_RDWR, nullptr, "no permission to read and write fd");
    CHECK_AND_RETURN_RET_LOG(lseek(fd, 0, SEEK_CUR) != -1, nullptr, "the fd is not seekable");

    std::shared_ptr<AVMuxerImpl> impl = std::make_shared<AVMuxerImpl>(fd, format);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "create avmuxer implementation failed");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "init avmuxer implementation failed");
    return impl;
}

AVMuxerImpl::AVMuxerImpl(int32_t fd, AVOutputFormat format) : fd_(fd), format_(format)
{
    (void)fd_;
    (void)format_;
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMuxerImpl::~AVMuxerImpl()
{
    if (muxerClient_ != nullptr) {
        (void)AVCodecServiceFactory::GetInstance().DestroyMuxerService(muxerClient_);
        muxerClient_ = nullptr;
    }
    AVCODEC_LOGD("AVMuxerImpl:0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

int32_t AVMuxerImpl::Init()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    muxerClient_ = AVCodecServiceFactory::GetInstance().CreateMuxerService();
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "create avmuxer engine failed");
    return AVCS_ERR_OK;
}

int32_t AVMuxerImpl::SetLocation(float latitude, float longitude)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->SetLocation(latitude, longitude);
}

int32_t AVMuxerImpl::SetRotation(int32_t rotation)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->SetRotation(rotation);
}

int32_t AVMuxerImpl::SetParameter(const Format &generalFormat)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->SetParameter(generalFormat);
}

int32_t AVMuxerImpl::AddTrack(uint32_t &trackIndex, const Format &trackFormat)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->AddTrack(trackIndex, trackFormat);
}

int32_t AVMuxerImpl::Start()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->Start();
}

int32_t AVMuxerImpl::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    CHECK_AND_RETURN_RET_LOG(sampleBuffer != nullptr && info.offset >= 0 && info.size >= 0,
        AVCS_ERR_INVALID_VAL, "Invalid memory");

    std::shared_ptr<AVSharedMemoryBase> sharedSampleBuffer =
        std::make_shared<AVSharedMemoryBase>(info.size, AVSharedMemory::FLAGS_READ_ONLY, "sampleBuffer");
    int32_t ret = sharedSampleBuffer->Init();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, AVCS_ERR_NO_MEMORY, "create AVSharedMemoryBase failed");
    errno_t rc = memcpy_s(sharedSampleBuffer->GetBase(), sharedSampleBuffer->GetSize(), sampleBuffer + info.offset, info.size);
    CHECK_AND_RETURN_RET_LOG(rc == EOK, AVCS_ERR_UNKNOWN, "memcpy_s failed");

    return muxerClient_->WriteSampleBuffer(trackIndex, sharedSampleBuffer->GetBase(), info);
}

int32_t AVMuxerImpl::Stop()
{
    AVCodecTrace trace(std::string(__FUNCTION__));
    CHECK_AND_RETURN_RET_LOG(muxerClient_ != nullptr, AVCS_ERR_INVALID_OPERATION, "AVMuxer Engine does not exist");
    return muxerClient_->Stop();
}
} // namespace Media
} // namespace OHOS