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
#ifndef AVCODEC_LISTENER_PROXY_H
#define AVCODEC_LISTENER_PROXY_H

#include "i_standard_avcodec_listener.h"
#include "avcodec_death_recipient.h"
#include "nocopyable.h"

namespace OHOS {
namespace MediaAVCodec {
class AVCodecListenerProxy : public IRemoteProxy<IStandardAVCodecListener>, public NoCopyable {
public:
    explicit AVCodecListenerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AVCodecListenerProxy();

private:
    static inline BrokerDelegator<AVCodecListenerProxy> delegator_;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVCODEC_LISTENER_PROXY_H
