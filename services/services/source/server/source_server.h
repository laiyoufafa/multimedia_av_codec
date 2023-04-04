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

#ifndef SOURCE_SERVER_H
#define SOURCE_SERVER_H

#include <mutex>
#include "i_source_service.h"
#include "nocopyable.h"

namespace OHOS {
namespace AVCodec {
class SourceServer : public ISourceService, public NoCopyable {
public:
    static std::shared_ptr<ISourceService> Create();
    SourceServer();
    ~SourceServer();

    int32_t GetTrackCount() override;
    int32_t Destroy() override;
    int32_t SetParameter(const Format &param, uint32_t trackId) override;
    int32_t GetTrackFormat(Format &format, uint32_t trackId) override;
    uint64_t GetSourceAttr() override;

private:
    int32_t Init();
    std::mutex mutex_;
    // std::shared_ptr<IAVMuxerEngine> avmuxerEngine_ = nullptr;
    // uint32_t trackNum_ = 0;
};
}  // namespace AVCodec
}  // namespace OHOS
#endif