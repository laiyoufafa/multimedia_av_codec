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
#include "avcodec_client.h"
#include "avcodec_xcollie.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "avcodec_dfx.h"

#ifdef SUPPORT_DEMUXER
#include "i_standard_demuxer_service.h"
#endif
#ifdef SUPPORT_CODEC
#include "i_standard_codec_service.h"
#endif
#ifdef SUPPORT_CODECLIST
#include "i_standard_codeclist_service.h"
#endif

#ifdef SUPPORT_SOURCE
#include "i_standard_source_service.h"
#endif

#include "avcodec_errors.h"
#include "avcodec_log.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecClient"};
}

namespace OHOS {
namespace Media {
static AVCodecClient avCodecClientInstance;
IAVCodecService &AVCodecServiceFactory::GetInstance()
{
    return avCodecClientInstance;
}

AVCodecClient::AVCodecClient() noexcept
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecClient::~AVCodecClient()
{
    AVCODEC_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool AVCodecClient::IsAlived()
{
    if (avCodecProxy_ == nullptr) {
        avCodecProxy_ = GetAVCodecProxy();
    }

    return avCodecProxy_ != nullptr;
}
#ifdef SUPPORT_CODEC
std::shared_ptr<ICodecService> AVCodecClient::CreateCodecService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("av_codec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_CODEC, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "codec proxy object is nullptr.");

    sptr<IStandardCodecService> codecProxy = iface_cast<IStandardCodecService>(object);
    CHECK_AND_RETURN_RET_LOG(codecProxy != nullptr, nullptr, "codec proxy is nullptr.");

    std::shared_ptr<CodecClient> codecClient = CodecClient::Create(codecProxy);
    CHECK_AND_RETURN_RET_LOG(codecClient != nullptr, nullptr, "failed to create codec client.");

    codecClientList_.push_back(codecClient);
    return codecClient;
}

int32_t AVCodecClient::DestroyCodecService(std::shared_ptr<ICodecService> codecClient)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecClient != nullptr, AVCS_ERR_NO_MEMORY, "codec client is nullptr.");
    codecClientList_.remove(codecClient);
    return AVCS_ERR_OK;
}
#endif
#ifdef SUPPORT_CODECLIST
std::shared_ptr<ICodecListService> AVCodecClient::CreateCodecListService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("av_codec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_CODECLIST, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "codeclist proxy object is nullptr.");

    sptr<IStandardCodecListService> codecListProxy = iface_cast<IStandardCodecListService>(object);
    CHECK_AND_RETURN_RET_LOG(codecListProxy != nullptr, nullptr, "codeclist proxy is nullptr.");

    std::shared_ptr<CodecListClient> codecListClient = CodecListClient::Create(codecListProxy);
    CHECK_AND_RETURN_RET_LOG(codecListClient != nullptr, nullptr, "failed to create codeclist client.");

    codecListClientList_.push_back(codecListClient);
    return codecListClient;
}

int32_t AVCodecClient::DestroyCodecListService(std::shared_ptr<ICodecListService> codecListClient)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codecListClient != nullptr, AVCS_ERR_NO_MEMORY, "codeclist client is nullptr.");
    codecListClientList_.remove(codecListClient);
    return AVCS_ERR_OK;
}
#endif

#ifdef SUPPORT_DEMUXER
std::shared_ptr<IDemuxerService> AVCodecClient::CreateDemuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("av_codec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_DEMUXER, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "demuxer proxy object is nullptr.");

    sptr<IStandardDemuxerService> demuxerProxy = iface_cast<IStandardDemuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy != nullptr, nullptr, "demuxer proxy is nullptr.");

    std::shared_ptr<DemuxerClient> demuxerClient = DemuxerClient::Create(demuxerProxy);
    CHECK_AND_RETURN_RET_LOG(demuxerClient != nullptr, nullptr, "failed to create demuxer client.");

    demuxerClientList_.push_back(demuxerClient);
    return demuxerClient;
}

int32_t AVCodecClient::DestroyDemuxerService(std::shared_ptr<IDemuxerService> demuxerClient)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxerClient != nullptr, AVCS_ERR_NO_MEMORY, "demuxer client is nullptr.");
    demuxerClientList_.remove(demuxerClient);
    return AVCS_ERR_OK;
}
#endif

#ifdef SUPPORT_MUXER
std::shared_ptr<IMuxerService> AVCodecClient::CreateMuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("avcodec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_MUXER, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "muxer proxy object is nullptr.");

    sptr<IStandardMuxerService> muxerProxy = iface_cast<IStandardMuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(muxerProxy != nullptr, nullptr, "muxer proxy is nullptr.");

    std::shared_ptr<MuxerClient> muxer = MuxerClient::Create(muxerProxy);
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, nullptr, "failed to create muxer client.");

    muxerClientList_.push_back(muxer);
    return muxer;
}

int32_t AVCodecClient::DestroyMuxerService(std::shared_ptr<IMuxerService> muxer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, AVCS_ERR_NO_MEMORY, "input muxer is nullptr.");
    muxerClientList_.remove(muxer);
    return AVCS_ERR_OK;
}
#endif

#ifdef SUPPORT_SOURCE
std::shared_ptr<ISourceService> AVCodecClient::CreateSourceService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        AVCODEC_LOGE("av_codec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avCodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_SOURCE, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "source proxy object is nullptr.");

    sptr<IStandardSourceService> sourceProxy = iface_cast<IStandardSourceService>(object);
    CHECK_AND_RETURN_RET_LOG(sourceProxy != nullptr, nullptr, "source proxy is nullptr.");

    std::shared_ptr<SourceClient> sourceClient = SourceClient::Create(sourceProxy);
    CHECK_AND_RETURN_RET_LOG(sourceClient != nullptr, nullptr, "failed to create source client.");

    sourceClientList_.push_back(sourceClient);
    return sourceClient;
}

int32_t AVCodecClient::DestroySourceService(std::shared_ptr<ISourceService> sourceClient)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sourceClient != nullptr, AVCS_ERR_NO_MEMORY, "source client is nullptr.");
    sourceClientList_.remove(sourceClient);
    return AVCS_ERR_OK;
}
#endif

sptr<IStandardAVCodecService> AVCodecClient::GetAVCodecProxy()
{
    AVCODEC_LOGD("enter");
    sptr<ISystemAbilityManager> samgr = nullptr;
    COLLIE_LISTEN(samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager(),
                  "AVCodecClient::GetAVCodecProxy");
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "system ability manager is nullptr.");

    sptr<IRemoteObject> object = nullptr;
    COLLIE_LISTEN(object = samgr->GetSystemAbility(OHOS::AV_CODEC_SERVICE_ID), "AVCodecClient::GetAVCodecProxy");
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "avcodec object is nullptr.");

    avCodecProxy_ = iface_cast<IStandardAVCodecService>(object);

    CHECK_AND_RETURN_RET_LOG(avCodecProxy_ != nullptr, nullptr, "avcodec proxy is nullptr.");

    pid_t pid = 0;
    deathRecipient_ = new (std::nothrow) AVCodecDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient_ != nullptr, nullptr, "failed to new AVCodecDeathRecipient.");

    deathRecipient_->SetNotifyCb(std::bind(&AVCodecClient::AVCodecServerDied, std::placeholders::_1));
    bool result = object->AddDeathRecipient(deathRecipient_);
    if (!result) {
        AVCODEC_LOGE("failed to add deathRecipient");
        return nullptr;
    }

    listenerStub_ = new (std::nothrow) AVCodecListenerStub();
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, nullptr, "failed to new AVCodecListenerStub");
    return avCodecProxy_;
}

void AVCodecClient::AVCodecServerDied(pid_t pid)
{
    AVCODEC_LOGE("av_codec server is died, pid:%{public}d!", pid);
    avCodecClientInstance.DoAVCodecServerDied();
    FaultEventWrite(FaultType::FAULT_TYPE_CRASH, "AV_CODEC server is died", "AV_CODEC client");
}

void AVCodecClient::DoAVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (avCodecProxy_ != nullptr) {
        (void)avCodecProxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        avCodecProxy_ = nullptr;
    }
    listenerStub_ = nullptr;
    deathRecipient_ = nullptr;

#ifdef SUPPORT_DEMUXER
    for (auto &it : demuxerClientList_) {
        auto demuxerClient = std::static_pointer_cast<DemuxerClient>(it);
        if (demuxerClient != nullptr) {
            demuxerClient->AVCodecServerDied();
        }
    }
#endif
#ifdef SUPPORT_CODEC
    for (auto &it : codecClientList_) {
        auto codecClient = std::static_pointer_cast<CodecClient>(it);
        if (codecClient != nullptr) {
            codecClient->AVCodecServerDied();
        }
    }
#endif
#ifdef SUPPORT_CODECLIST
    for (auto &it : codecListClientList_) {
        auto codecListClient = std::static_pointer_cast<CodecListClient>(it);
        if (codecListClient != nullptr) {
            codecListClient->AVCodecServerDied();
        }
    }
#endif
#ifdef SUPPORT_MUXER
    for (auto &it : muxerClientList_) {
        auto muxer = std::static_pointer_cast<MuxerClient>(it);
        if (muxer != nullptr) {
            muxer->AVCodecServerDied();
        }
    }
#endif
#ifdef SUPPORT_SOURCE
    for (auto &it : sourceClientList_) {
        auto sourceClient = std::static_pointer_cast<SourceClient>(it);
        if (sourceClient != nullptr) {
            sourceClient->AVCodecServerDied();
        }
    }
#endif
}
} // namespace Media
} // namespace OHOS
