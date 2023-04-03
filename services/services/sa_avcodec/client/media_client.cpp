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
#include "media_client.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"

#ifdef SUPPORT_DEMUXER
#include "i_standard_demuxer_service.h"
#endif

#include "av_log.h"
#include "media_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "MediaClient"};
}

namespace OHOS {
namespace AVCodec {

static MediaClient mediaClientInstance;
IMediaService &MediaServiceFactory::GetInstance()
{
    return mediaClientInstance;
}

MediaClient::MediaClient() noexcept
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

MediaClient::~MediaClient()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool MediaClient::IsAlived()
{
    if (mediaProxy_ == nullptr) {
        mediaProxy_ = GetMediaProxy();
    }

    return (mediaProxy_ != nullptr) ? true : false;
}
#ifdef SUPPORT_CODEC
std::shared_ptr<IAVCodecService> MediaClient::CreateAVCodecService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        MEDIA_LOGE("media service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = mediaProxy_->GetSubSystemAbility(
        IStandardMediaService::MediaSystemAbility::MEDIA_AVCODEC, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "avcodec proxy object is nullptr.");

    sptr<IStandardAVCodecService> avCodecProxy = iface_cast<IStandardAVCodecService>(object);
    CHECK_AND_RETURN_RET_LOG(avCodecProxy != nullptr, nullptr, "avcodec proxy is nullptr.");

    std::shared_ptr<AVCodecClient> avCodec = AVCodecClient::Create(avCodecProxy);
    CHECK_AND_RETURN_RET_LOG(avCodec != nullptr, nullptr, "failed to create avcodec client.");

    avCodecClientList_.push_back(avCodec);
    return avCodec;
}

int32_t MediaClient::DestroyAVCodecService(std::shared_ptr<IAVCodecService> avCodec)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(avCodec != nullptr, MSERR_NO_MEMORY, "input avcodec is nullptr.");
    avCodecClientList_.remove(avCodec);
    return MSERR_OK;
}
#endif

#ifdef SUPPORT_DEMUXER
std::shared_ptr<IDemuxerService> MediaClient::CreateDemuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        MEDIA_LOGE("media service does not exist.");
        printf("media service does not exist\n");
        return nullptr;
    }

    sptr<IRemoteObject> object = mediaProxy_->GetSubSystemAbility(
        IStandardAvcodecService::MediaSystemAbility::MEDIA_DEMUXER, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "demuxer proxy object is nullptr.");

    sptr<IStandardDemuxerService> demuxerProxy = iface_cast<IStandardDemuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy != nullptr, nullptr, "demuxer proxy is nullptr.");

    std::shared_ptr<DemuxerClient> demuxer = DemuxerClient::Create(demuxerProxy);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, nullptr, "failed to create demuxer client.");

    demuxerClientList_.push_back(demuxer);
    return demuxer;
}

int32_t MediaClient::DestroyDemuxerService(std::shared_ptr<IDemuxerService> demuxer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, MSERR_NO_MEMORY, "input demuxer is nullptr.");
    demuxerClientList_.remove(demuxer);
    return MSERR_OK;
}
#endif


sptr<IStandardAvcodecService> MediaClient::GetMediaProxy()
{
    MEDIA_LOGD("enter");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "system ability manager is nullptr.");

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::AVCODEC_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "media object is nullptr.");

    mediaProxy_ = iface_cast<IStandardAvcodecService>(object);
    
    CHECK_AND_RETURN_RET_LOG(mediaProxy_ != nullptr, nullptr, "media proxy is nullptr.");

    pid_t pid = 0;
    deathRecipient_ = new(std::nothrow) MediaDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient_ != nullptr, nullptr, "failed to new MediaDeathRecipient.");

    deathRecipient_->SetNotifyCb(std::bind(&MediaClient::MediaServerDied, std::placeholders::_1));
    bool result = object->AddDeathRecipient(deathRecipient_);
    if (!result) {
        MEDIA_LOGE("failed to add deathRecipient");
        return nullptr;
    }

    listenerStub_ = new(std::nothrow) MediaListenerStub();
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, nullptr, "failed to new MediaListenerStub");
    return mediaProxy_;
}

void MediaClient::MediaServerDied(pid_t pid)
{
    MEDIA_LOGE("media server is died, pid:%{public}d!", pid);
    mediaClientInstance.DoMediaServerDied();
}

void MediaClient::DoMediaServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (mediaProxy_ != nullptr) {
        (void)mediaProxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        mediaProxy_ = nullptr;
    }
    listenerStub_ = nullptr;
    deathRecipient_ = nullptr;

#ifdef SUPPORT_DEMUXER
    for (auto &it : demuxerClientList_) {
        auto demuxer = std::static_pointer_cast<DemuxerClient>(it);
        if (demuxer != nullptr) {
            demuxer->MediaServerDied();
        }
    }
#endif
#ifdef SUPPORT_CODEC
    for (auto &it : avCodecClientList_) {
        auto avCodecClient = std::static_pointer_cast<AVCodecClient>(it);
        if (avCodecClient != nullptr) {
            avCodecClient->MediaServerDied();
        }
    }
#endif

}
} // namespace AVCodec
} // namespace OHOS
