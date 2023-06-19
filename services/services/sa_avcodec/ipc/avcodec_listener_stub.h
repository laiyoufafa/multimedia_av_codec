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
#ifndef AVCODEC_LISTENER_STUB_H
#define AVCODEC_LISTENER_STUB_H

#include "i_standard_avcodec_listener.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class AVCodecListenerStub : public IRemoteStub<IStandardAVCodecListener>, public NoCopyable {
public:
    AVCodecListenerStub();
    virtual ~AVCodecListenerStub();
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_LISTENER_STUB_H
