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
#ifndef AVCODEC_CLIENT_H
#define AVCODEC_CLIENT_H

#include "i_avcodec_service.h"
#include "avcodec_death_recipient.h"
#include "avcodec_listener_stub.h"
#include "i_standard_avcodec_service.h"
#ifdef SUPPORT_MUXER
#include "muxer_client.h"
#endif
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class AVCodecClient : public IAVCodecService, public NoCopyable {
public:
    AVCodecClient() noexcept;
    ~AVCodecClient();

#ifdef SUPPORT_MUXER
    std::shared_ptr<IMuxerService> CreateMuxerService() override;
    int32_t DestroyMuxerService(std::shared_ptr<IMuxerService> muxer) override;
#endif

private:
    sptr<IStandardAVCodecService> GetAVCodecProxy();
    bool IsAlived();
    static void AVCodecServerDied(pid_t pid);
    void DoAVCodecServerDied();

    sptr<IStandardAVCodecService> avCodecProxy_ = nullptr;
    sptr<AVCodecListenerStub> listenerStub_ = nullptr;
    sptr<AVCodecDeathRecipient> deathRecipient_ = nullptr;

#ifdef SUPPORT_MUXER
    std::list<std::shared_ptr<IMuxerService>> muxerClientList_;
#endif

    std::mutex mutex_;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_CLIENT_H
