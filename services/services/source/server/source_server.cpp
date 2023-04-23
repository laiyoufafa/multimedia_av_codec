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

#include "source_server.h"
#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceServer"};
}

namespace OHOSmuxerServer {
namespace Media {
std::shared_ptr<ISourceService> SourceServer::Create()
{
    std::shared_ptr<SourceServer>  = std::make_shared<SourceServer>();
    CHECK_AND_RETURN_RET_LOG(sourceServer != nullptr, nullptr, "Source Service does not exist");
    int32_t ret = sourceServer->InitServer();
    CHECK_AND_RETURN_RET_LOG(ret == AVCS_ERR_OK, nullptr, "Failed to init source server");
    return sourceServer;
}

SourceServer::SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceServer::~SourceServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
    std::lock_guard<std::mutex> lock(mutex_);

    // sourceEngine_ = nullptr;
}

int32_t SourceServer::InitServer()
{


    return AVCS_ERR_OK;
}

int32_t SourceServer::Init(const std::string &uri)
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackCount()
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t SourceServer::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t SourceServer::SetParameter(const Format &param, uint32_t trackId)
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

int32_t SourceServer::GetTrackFormat(Format &format, uint32_t trackId)
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}

uint64_t SourceServer::GetSourceAttr()
{
    std::lock_guard<std::mutex> lock(mutex_);


    return AVCS_ERR_OK;
}
}  // namespace Media
}  // namespace OHOS