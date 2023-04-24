/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "avmuxer_server.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVMuxerServer"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<IAVMuxer> AVMuxerServer::Create()
{
    std::shared_ptr<AVMuxerServer> muxerServer = std::make_shared<AVMuxerServer>();
    CHECK_AND_RETURN_RET_LOG(muxerServer != nullptr, nullptr, "Muxer Service does not exist");
    int32_t ret = muxerServer->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init muxer server");
    return muxerServer;
}

AVMuxerServer::AVMuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVMuxerServer::~AVMuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    std::lock_guard<std::mutex> lock(mutex_);

    // avmuxerEngine_ = nullptr;
}

int32_t AVMuxerServer::InitServer()
{


    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::SetLocation(float latitude, float longitude)
{
    // TODO:achieve it
    (void)latitude;
    (void)longitude;
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::SetRotation(int32_t rotation)
{
    // TODO:achieve it
    (void)rotation;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::SetParameter(const Format &generalFormat)
{
    // TODO:achieve it
    (void)generalFormat;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::AddTrack(uint32_t &trackIndex, const Format &trackFormat)
{
    // TODO:achieve it
    (void)trackIndex;
    (void)trackFormat;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    // TODO:achieve it
    (void)trackIndex;
    (void)sampleBuffer;
    (void)info; 
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t AVMuxerServer::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS