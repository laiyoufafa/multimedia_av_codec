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

#include "source_client.h"
#include "media_errors.h"
#include "media_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "SourceClient"};
}

namespace OHOS {
namespace AVCodec {
std::shared_ptr<SourceClient> SourceClient::Create(const sptr<IStandardSourceService> &ipcProxy)
{
    std::shared_ptr<SourceClient> avSourceClient = std::make_shared<SourceClient>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(avSourceClient != nullptr, nullptr, "Failed to create source client");
    return avSourceClient;
}

SourceClient::SourceClient(const sptr<IStandardSourceService> &ipcProxy)
    : sourceProxy_(ipcProxy)
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

SourceClient::~SourceClient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sourceProxy_ != nullptr) {
        (void)sourceProxy_->DestroyStub();
        sourceProxy_ = nullptr;
    }
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void SourceClient::AVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sourceProxy_ = nullptr;
}

int32_t SourceClient::GetTrackCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, nullptr, "source service does not exist.");

    MEDIA_LOGD("GetTrackCount");
    return sourceProxy_->GetTrackCount();
}

int32_t SourceClient::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, nullptr, "source service does not exist.");

    MEDIA_LOGD("Destroy");
    return sourceProxy_->Destroy();
}

int32_t SourceClient::SetParameter(const Format &param, uint32_t trackId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, nullptr, "source service does not exist.");

    MEDIA_LOGD("SetParameter");
    return sourceProxy_->SetParameter(param, trackId);
}

int32_t SourceClient::GetTrackFormat(Format &format, uint32_t trackId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, nullptr, "source service does not exist.");

    MEDIA_LOGD("GetTrackFormat");
    return sourceProxy_->GetTrackFormat(format, trackId);
}

uint64_t SourceClient::GetSourceAttr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceProxy_ != nullptr, nullptr, "source service does not exist.");

    MEDIA_LOGD("GetSourceAttr");
    return sourceProxy_->GetSourceAttr();
}
}  // namespace AVCodec
}  // namespace OHOS