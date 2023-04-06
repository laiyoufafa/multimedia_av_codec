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
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"

#ifdef SUPPORT_DEMUXER
#include "i_standard_demuxer_service.h"
#endif
#ifdef SUPPORT_MUXER
#include "i_standard_muxer_service.h"
#endif
#ifdef SUPPORT_CODEC
#include "i_standard_codec_service.h"
#endif
#ifdef SUPPORT_SOURCE
#include "i_standard_source_service.h"
#endif

#include "av_log.h"
#include "media_errors.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecClient"};
}

namespace OHOS {
namespace AVCodec {

static AVCodecClient avcodecClientInstance;
IAVCodecService &AVCodecServiceFactory::GetInstance()
{
    return avcodecClientInstance;
}

AVCodecClient::AVCodecClient() noexcept
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecClient::~AVCodecClient()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

bool AVCodecClient::IsAlived()
{
    if (avcodecProxy_ == nullptr) {
        avcodecProxy_ = GetAVCodecProxy();
    }

    return (avcodecProxy_ != nullptr) ? true : false;
}
#ifdef SUPPORT_CODEC
std::shared_ptr<ICodecService> AVCodecClient::CreateCodecService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        MEDIA_LOGE("codec service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avcodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_CODEC, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "codec proxy object is nullptr.");

    sptr<IStandardAVCodecService> avCodecProxy = iface_cast<IStandardAVCodecService>(object);
    CHECK_AND_RETURN_RET_LOG(avCodecProxy != nullptr, nullptr, "codec proxy is nullptr.");

    std::shared_ptr<CodecClient> codec = CodecClient::Create(avCodecProxy);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, nullptr, "failed to create codec client.");

    codecClientList_.push_back(codec);
    return codec;
}

int32_t AVCodecClient::DestroyCodecService(std::shared_ptr<ICodecService> codec)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(codec != nullptr, MSERR_NO_MEMORY, "input codec is nullptr.");
    codecClientList_.remove(codec);
    return MSERR_OK;
}
#endif

#ifdef SUPPORT_DEMUXER
std::shared_ptr<IDemuxerService> AVCodecClient::CreateDemuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        MEDIA_LOGE("demuxer service does not exist.");
        printf("demuxer service does not exist\n");
        return nullptr;
    }

    sptr<IRemoteObject> object = avcodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_DEMUXER, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "demuxer proxy object is nullptr.");

    sptr<IStandardDemuxerService> demuxerProxy = iface_cast<IStandardDemuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(demuxerProxy != nullptr, nullptr, "demuxer proxy is nullptr.");

    std::shared_ptr<DemuxerClient> demuxer = DemuxerClient::Create(demuxerProxy);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, nullptr, "failed to create demuxer client.");

    demuxerClientList_.push_back(demuxer);
    return demuxer;
}

int32_t AVCodecClient::DestroyDemuxerService(std::shared_ptr<IDemuxerService> demuxer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(demuxer != nullptr, MSERR_NO_MEMORY, "input demuxer is nullptr.");
    demuxerClientList_.remove(demuxer);
    return MSERR_OK;
}
#endif

#ifdef SUPPORT_MUXER
std::shared_ptr<IMuxerService> AVCodecClient::CreateMuxerService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        av_logE("muxer service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avcodecProxy_->GetSubSystemAbility(
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
    CHECK_AND_RETURN_RET_LOG(muxer != nullptr, MSERR_NO_MEMORY, "input muxer is nullptr.");
    muxerClientList_.remove(muxer);
    return MSERR_OK;
}
#endif

#ifdef SUPPORT_SOURCE
std::shared_ptr<IMuxerService> AVCodecClient::CreateSourceService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsAlived()) {
        av_logE("source service does not exist.");
        return nullptr;
    }

    sptr<IRemoteObject> object = avcodecProxy_->GetSubSystemAbility(
        IStandardAVCodecService::AVCodecSystemAbility::AVCODEC_SOURCE, listenerStub_->AsObject());
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "source proxy object is nullptr.");

    sptr<IStandardMuxerService> sourceProxy = iface_cast<IStandardMuxerService>(object);
    CHECK_AND_RETURN_RET_LOG(sourceProxy != nullptr, nullptr, "source proxy is nullptr.");

    std::shared_ptr<MuxerClient> source = MuxerClient::Create(sourceProxy);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, nullptr, "failed to create source client.");

    sourceClientList_.push_back(source);
    return source;
}

int32_t AVCodecClient::DestroySourceService(std::shared_ptr<ISourceService> source)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, MSERR_NO_MEMORY, "input source is nullptr.");
    sourceClientList_.remove(source);
    return MSERR_OK;
}
#endif

sptr<IStandardAVCodecService> AVCodecClient::GetAVCodecProxy()
{
    MEDIA_LOGD("enter");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "system ability manager is nullptr.");

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::AVCODEC_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "avcodec object is nullptr.");

    avcodecProxy_ = iface_cast<IStandardAVCodecService>(object);
    
    CHECK_AND_RETURN_RET_LOG(avcodecProxy_ != nullptr, nullptr, "avcodec proxy is nullptr.");

    pid_t pid = 0;
    deathRecipient_ = new(std::nothrow) AVCodecDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient_ != nullptr, nullptr, "failed to new AVCodecDeathRecipient.");

    deathRecipient_->SetNotifyCb(std::bind(&AVCodecClient::AVCodecServerDied, std::placeholders::_1));
    bool result = object->AddDeathRecipient(deathRecipient_);
    if (!result) {
        MEDIA_LOGE("failed to add deathRecipient");
        return nullptr;
    }

    listenerStub_ = new(std::nothrow) AVCodecListenerStub();
    CHECK_AND_RETURN_RET_LOG(listenerStub_ != nullptr, nullptr, "failed to new AVCodecListenerStub");
    return avcodecProxy_;
}

void AVCodecClient::AVCodecServerDied(pid_t pid)
{
    MEDIA_LOGE("avcodec server is died, pid:%{public}d!", pid);
    avcodecClientInstance.DoAVCodecServerDied();
}

void AVCodecClient::DoAVCodecServerDied()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (avcodecProxy_ != nullptr) {
        (void)avcodecProxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        avcodecProxy_ = nullptr;
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
#ifdef SUPPORT_MUXER
    for (auto &it : muxerClientList_) {
        auto muxerClient = std::static_pointer_cast<MuxerClient>(it);
        if (muxerClient != nullptr) {
            muxerClient->AVCodecServerDied();
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
} // namespace AVCodec
} // namespace OHOS
