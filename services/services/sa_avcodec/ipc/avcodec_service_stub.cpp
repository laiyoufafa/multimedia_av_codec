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
#include "avcodec_service_stub.h"
#include "av_log.h"
#include "media_errors.h"
#include "avcodec_server_manager.h"
// #include "player_xcollie.h"

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecServiceStub"};
}

namespace OHOS {
namespace AVCodec {
AVCodecServiceStub::AVCodecServiceStub()
{
    deathRecipientMap_.clear();
    avcodecListenerMap_.clear();
    Init();
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances create", FAKE_POINTER(this));
}

AVCodecServiceStub::~AVCodecServiceStub()
{
    MEDIA_LOGD("0x%{public}06" PRIXPTR " Instances destroy", FAKE_POINTER(this));
}

void AVCodecServiceStub::Init()
{
    avcodecFuncs_[GET_SUBSYSTEM] = &AVCodecServiceStub::GetSystemAbility;
}

int32_t AVCodecServiceStub::DestroyStubForPid(pid_t pid)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sptr<AVCodecDeathRecipient> deathRecipient = nullptr;
        sptr<IStandardAVCodecListener> avcodecListener = nullptr;

        auto itDeath = deathRecipientMap_.find(pid);
        if (itDeath != deathRecipientMap_.end()) {
            deathRecipient = itDeath->second;

            if (deathRecipient != nullptr) {
                deathRecipient->SetNotifyCb(nullptr);
            }

            (void)deathRecipientMap_.erase(itDeath);
        }

        auto itListener = avcodecListenerMap_.find(pid);
        if (itListener != avcodecListenerMap_.end()) {
            avcodecListener = itListener->second;

            if (avcodecListener != nullptr && avcodecListener->AsObject() != nullptr && deathRecipient != nullptr) {
                (void)avcodecListener->AsObject()->RemoveDeathRecipient(deathRecipient);
            }

            (void)avcodecListenerMap_.erase(itListener);
        }
    }

    AVCodecServerManager::GetInstance().DestroyStubObjectForPid(pid);
    return MSERR_OK;
}

int AVCodecServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    MEDIA_LOGI("Stub: OnRemoteRequest of code: %{public}u is received", code);

    auto remoteDescriptor = data.ReadInterfaceToken();
    if (AVCodecServiceStub::GetDescriptor() != remoteDescriptor) {
        MEDIA_LOGE("Invalid descriptor");
        return MSERR_INVALID_OPERATION;
    }

    auto itFunc = avcodecFuncs_.find(code);
    if (itFunc != avcodecFuncs_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            int32_t ret = (this->*memberFunc)(data, reply);
            if (ret != MSERR_OK) {
                MEDIA_LOGE("Calling memberFunc is failed.");
            }
            return MSERR_OK;
        }
    }
    MEDIA_LOGW("avcodecFuncs_: no member func supporting, applying default process");

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void AVCodecServiceStub::ClientDied(pid_t pid)
{
    MEDIA_LOGE("client pid is dead, pid:%{public}d", pid);
    (void)DestroyStubForPid(pid);
}

int32_t AVCodecServiceStub::SetDeathListener(const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, MSERR_NO_MEMORY, "set listener object is nullptr");

    sptr<IStandardAVCodecListener> avcodecListener = iface_cast<IStandardAVCodecListener>(object);
    CHECK_AND_RETURN_RET_LOG(avcodecListener != nullptr, MSERR_NO_MEMORY,
        "failed to convert IStandardAVCodecListener");

    pid_t pid = IPCSkeleton::GetCallingPid();
    sptr<AVCodecDeathRecipient> deathRecipient = new(std::nothrow) AVCodecDeathRecipient(pid);
    CHECK_AND_RETURN_RET_LOG(deathRecipient != nullptr, MSERR_NO_MEMORY, "failed to new AVCodecDeathRecipient");

    deathRecipient->SetNotifyCb(std::bind(&AVCodecServiceStub::ClientDied, this, std::placeholders::_1));

    if (avcodecListener->AsObject() != nullptr) {
        (void)avcodecListener->AsObject()->AddDeathRecipient(deathRecipient);
    }

    MEDIA_LOGD("client pid pid:%{public}d", pid);
    avcodecListenerMap_[pid] = avcodecListener;
    deathRecipientMap_[pid] = deathRecipient;
    return MSERR_OK;
}

int32_t AVCodecServiceStub::GetSystemAbility(MessageParcel &data, MessageParcel &reply)
{
    AVCodecSystemAbility id = static_cast<AVCodecSystemAbility>(data.ReadInt32());
    sptr<IRemoteObject> listenerObj = data.ReadRemoteObject();
    // int32_t xcollieId = PlayerXCollie::GetInstance().SetTimer("AVCodecServiceStub::GetSystemAbility", true);
    (void)reply.WriteRemoteObject(GetSubSystemAbility(id, listenerObj));
    // PlayerXCollie::GetInstance().CancelTimer(xcollieId);
    return MSERR_OK;
}
} // namespace AVCodec
} // namespace OHOS
