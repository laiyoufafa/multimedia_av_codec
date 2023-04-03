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
#ifndef MEDIA_CLIENT_H
#define MEDIA_CLIENT_H

#include "i_media_service.h"
#include "i_standard_media_service.h"
#include "media_death_recipient.h"
#include "media_listener_stub.h"

#ifdef SUPPORT_DEMUXER
#include "demuxer_client.h"
#endif

#ifdef SUPPORT_CODEC
#include "codec_client.h"
#endif

#include "nocopyable.h"

namespace OHOS {
namespace AVCodec {
class MediaClient : public IMediaService, public NoCopyable {
public:
    MediaClient() noexcept;
    ~MediaClient();

#ifdef SUPPORT_DEMUXER
    std::shared_ptr<IDemuxerService> CreateDemuxerService() override;
    int32_t DestroyDemuxerService(std::shared_ptr<IDemuxerService> demuxer) override;
#endif

#ifdef SUPPORT_CODEC
    std::shared_ptr<IAVCodecService> CreateAVCodecService() override;
    int32_t DestroyAVCodecService(std::shared_ptr<IAVCodecService> avCodec) override;
#endif

private:
    sptr<IStandardAvcodecService> GetMediaProxy();
    bool IsAlived();
    static void MediaServerDied(pid_t pid);
    void DoMediaServerDied();

    sptr<IStandardAvcodecService> mediaProxy_ = nullptr;
    sptr<MediaListenerStub> listenerStub_ = nullptr;
    sptr<MediaDeathRecipient> deathRecipient_ = nullptr;

#ifdef SUPPORT_DEMUXER
    std::list<std::shared_ptr<IDemuxerService>> demuxerClientList_;
#endif
#ifdef SUPPORT_CODEC
    std::list<std::shared_ptr<IAVCodecService>> avCodecClientList_;
#endif
    std::mutex mutex_;
};
} // namespace AVCodec
} // namespace OHOS
#endif // MEDIA_CLIENT_H
