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

#ifndef AVSOURCE_IMPL_H
#define AVSOURCE_IMPL_H

#include "avsource.h"
#include "nocopyable.h"
#include "i_avsource_service.h"

namespace OHOS {
namespace Media{
class AVSourceImpl : public AVSource, public NoCopyable {
public:
    AVSourceImpl();
    ~AVSourceImpl();

    uint32_t GetTrackCount() override;
    std::shared_ptr<SourceTrack> LoadSourceTrackByID(uint32_t trackId) override;
    int32_t Destroy() override;
    uint64_t GetSourceAttr() override;
    int32_t SetParameter(const Format &param, uint32_t trackId) override;
    std::shared_ptr<Format> GetTrackFormat(Format &param, uint32_t trackId) override;
    int32_t Init(const std::string &uri);

private:
    std::shared_ptr<IAVSourceService> sourceService_ = nullptr;
};

class AVSourceTrackImpl : public AVSourceTrack, public NoCopyable {
public:
    AVSourceTrackImpl(AVSource* source, uint32_t trackId);
    ~AVSourceTrackImpl();

    int32_t SetParameter(const Format &param) override;
    std::shared_ptr<Format> GetTrackFormat(Format &param) override;
private:
    uint32_t trackId_;
    std::shared_ptr<AVSourceImpl> sourceImpl_;
};
} // namespace Media
} // namespace OHOS
#endif // AVSOURCE_IMPL_H

