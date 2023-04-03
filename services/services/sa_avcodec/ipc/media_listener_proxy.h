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
#ifndef MEDIA_LISTENER_PROXY_H
#define MEDIA_LISTENER_PROXY_H

#include "i_standard_media_listener.h"
#include "media_death_recipient.h"
#include "nocopyable.h"

namespace OHOS {
namespace AVCodec {
class MediaListenerProxy : public IRemoteProxy<IStandardAvcodecListener>, public NoCopyable {
public:
    explicit MediaListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~MediaListenerProxy();

private:
    static inline BrokerDelegator<MediaListenerProxy> delegator_;
};
} // namespace AVCodec
} // namespace OHOS
#endif // MEDIA_LISTENER_PROXY_H
