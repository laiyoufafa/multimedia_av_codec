/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SOURCE_CLIENT_H
#define SOURCE_CLIENT_H

#include "i_source_service.h"
#include "i_standard_source_service.h"

namespace OHOS {
namespace Media {
class SourceClient : public ISourceService, public NoCopyable {
public:
    static std::shared_ptr<SourceClient> Create(const sptr<IStandardSourceService> &ipcProxy);
    explicit SourceClient(const sptr<IStandardSourceService> &ipcProxy);
    ~SourceClient();

    virtual int32_t Init(const std::string &uri) = 0;
    virtual int32_t GetTrackCount() = 0;
    virtual int32_t Destroy() = 0;
    virtual int32_t SetParameter(const Format &param, uint32_t trackId) = 0;
    virtual int32_t GetTrackFormat(Format &format, uint32_t trackId) = 0;
    virtual uint64_t GetSourceAttr() = 0;

    void AVCodecServerDied();
private:
    std::mutex mutex_;
    sptr<IStandardSourceService> sourceProxy_ = nullptr;
};
}  // namespace Media
}  // namespace OHOS
#endif  // SOURCE_CLIENT_H