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

#include "muxer_server.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MuxerServer"};
}

namespace OHOS {
namespace Media {
std::shared_ptr<IMuxerService> MuxerServer::Create()
{
    std::shared_ptr<MuxerServer> muxerServer = std::make_shared<MuxerServer>();
    CHECK_AND_RETURN_RET_LOG(muxerServer != nullptr, nullptr, "Muxer Service does not exist");
    int32_t ret = muxerServer->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init muxer server");
    return muxerServer;
}

MuxerServer::MuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MuxerServer::~MuxerServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    std::lock_guard<std::mutex> lock(mutex_);

    // avmuxerEngine_ = nullptr;
}

int32_t MuxerServer::InitServer()
{


    return AVCS_ERR_OK;
}

int32_t MuxerServer::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t MuxerServer::SetLocation(float latitude, float longitude)
{
    // TODO:achieve it
    (void)latitude;
    (void)longitude;
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t MuxerServer::SetRotation(int32_t rotation)
{
    // TODO:achieve it
    (void)rotation;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t MuxerServer::SetParameter(const Format &generalFormat)
{
    // TODO:achieve it
    (void)generalFormat;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t MuxerServer::AddTrack(const Format &trackFormat)
{
    // TODO:achieve it
    (void)trackFormat;
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t MuxerServer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t MuxerServer::WriteSampleBuffer(uint32_t trackIndex, uint8_t *sampleBuffer, AVCodecBufferInfo info)
{
    // TODO:achieve it
    (void)trackIndex;
    (void)sampleBuffer;
    (void)info; 
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}

int32_t MuxerServer::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);

    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS