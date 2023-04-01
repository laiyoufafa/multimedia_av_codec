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

#include "av_codec_server.h"
#include "iservice_registry.h"
#include "av_codec_log.h"
#include "av_codec_errors.h"
#include "system_ability_definition.h"
#include "av_codec_server_manager.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServer"};
}

namespace OHOS {
namespace AVCodec {
REGISTER_SYSTEM_ABILITY_BY_ID(AVCodecServer, AV_CODEC_SERVICE_ID, true)
AVCodecServer::AVCodecServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    // AV_CODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServer::~AVCodecServer()
{
    // AV_CODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecServer::OnDump()
{
    // AV_CODEC_LOGD("AVCodecServer OnDump");
}

void AVCodecServer::OnStart()
{
    // AV_CODEC_LOGD("AVCodecServer OnStart");
    bool res = Publish(this);
    if (res) {
        // AV_CODEC_LOGD("AVCodecServer OnStart res=%{public}d", res);
    }
}

void AVCodecServer::OnStop()
{
    // AV_CODEC_LOGD("AVCodecServer OnStop");
}

sptr<IRemoteObject> AVCodecServer::GetSubSystemAbility(IStandardAVCodecService::AVCodecSystemAbility subSystemId,
    const sptr<IRemoteObject> &listener)
{
    int32_t ret = AVCodecServiceStub::SetDeathListener(listener);
    CHECK_AND_RETURN_RET_LOG(ret == MSERR_OK, nullptr, "failed set death listener");

    switch (subSystemId) {
        case AVCodecSystemAbility::AV_CODEC_CODECLIST: {
            return AVCodecServerManager::GetInstance().CreateStubObject(AVCodecServerManager::CODECLIST);
        }
        case AVCodecSystemAbility::AV_CODEC_CODEC: {
            return AVCodecServerManager::GetInstance().CreateStubObject(AVCodecServerManager::CODEC);
        }
        case AVCodecSystemAbility::AV_CODEC_MUXER: {
            return AVCodecServerManager::GetInstance().CreateStubObject(AVCodecServerManager::MUXER);
        }
        case AVCodecSystemAbility::AV_CODEC_DEMUXER: {
            return AVCodecServerManager::GetInstance().CreateStubObject(AVCodecServerManager::DEMUXER);
        }
        default: {
            AV_CODEC_LOGE("subSystemId is invalid");
            return nullptr;
        }
    }
}

int32_t AVCodecServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    if (fd <= 0) {
        // AV_CODEC_LOGW("Failed to check fd");
        return OHOS::INVALID_OPERATION;
    }
    if (AVCodecServerManager::GetInstance().Dump(fd, args) != OHOS::NO_ERROR) {
        // AV_CODEC_LOGW("Failed to call AVCodecServerManager::Dump");
        return OHOS::INVALID_OPERATION;
    }

    return OHOS::NO_ERROR;
}
} // namespace AVCodec
} // namespace OHOS