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
#ifndef AVMUXER_ENGINE_DEMO_H
#define AVMUXER_ENGINE_DEMO_H

#include "avmuxer_demo_base.h"
#include "i_muxer_engine.h"

namespace OHOS {
namespace Media {
class AVMuxerEngineDemo : public AVMuxerDemoBase {
public:
    AVMuxerEngineDemo() = default;
    ~AVMuxerEngineDemo() = default;
private:
    void DoRunMuxer() override;
    int DoWriteSampleBuffer(uint8_t *sampleBuffer, TrackSampleInfo &info) override;
    int DoAddTrack(int32_t &trackIndex, MediaDescription &trackDesc) override;
    void DoRunMultiThreadCase() override;
    void DoRunMuxer(const std::string &runMode);
    std::shared_ptr<IMuxerEngine> avmuxer_;
};
}  // namespace Media
}  // namespace OHOS
#endif  // AVMUXER_DEMO_H