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

#include "avcodeclist_server.h"
#include "avcodec_log.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecListServer"};
}

namespace OHOS {
namespace MediaAVCodec {
std::shared_ptr<IAVCodecListService> AVCodecListServer::Create()
{
    std::shared_ptr<AVCodecListServer> server = std::make_shared<AVCodecListServer>();
    if (!server->Init()) {
        AVCODEC_LOGE("failed to init AVCodecListServer");
        return nullptr;
    }
    return server;
}

AVCodecListServer::AVCodecListServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecListServer::~AVCodecListServer()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool AVCodecListServer::Init()
{
    codecListCore_ = std::make_shared<AVCodecListCore>();
    CHECK_AND_RETURN_RET_LOG(codecListCore_ != nullptr, false,
        "Failed to create codec list core");
    return true;
}

std::string AVCodecListServer::FindVideoDecoder(const Format &format)
{
    return codecListCore_->FindVideoDecoder(format);
}

std::string AVCodecListServer::FindVideoEncoder(const Format &format)
{
    return codecListCore_->FindVideoEncoder(format);
}

std::string AVCodecListServer::FindAudioDecoder(const Format &format)
{
    return codecListCore_->FindAudioDecoder(format);
}

std::string AVCodecListServer::FindAudioEncoder(const Format &format)
{
    return codecListCore_->FindAudioEncoder(format);
}

CapabilityData AVCodecListServer::GetCapabilityData(std::string codecName)
{
    return codecListCore_->GetCapabilityData(codecName);
}
} // namespace MediaAVCodec
} // namespace OHOS
