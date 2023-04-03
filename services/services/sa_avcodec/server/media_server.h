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
#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H

#include "media_service_stub.h"
#include "system_ability.h"
#include "nocopyable.h"

namespace OHOS {
namespace AVCodec {
class AvcodecServer : public SystemAbility, public MediaServiceStub {
    DECLARE_SYSTEM_ABILITY(AvcodecServer);
public:
    explicit AvcodecServer(int32_t systemAbilityId, bool runOnCreate = true);
    ~AvcodecServer();

    // IStandardAvcodecService override
    sptr<IRemoteObject> GetSubSystemAbility(IStandardAvcodecService::MediaSystemAbility subSystemId,
        const sptr<IRemoteObject> &listener) override;

protected:
    // SystemAbility override
    void OnDump() override;
    void OnStart() override;
    void OnStop() override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
};
} // namespace AVCodec
} // namespace OHOS
#endif // MEDIA_SERVER_H
