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

#ifndef AVCODEC_DFX_H
#define AVCODEC_DFX_H

#include <string>
#include <refbase.h>
#include "nocopyable.h"
#include "hisysevent.h"

namespace OHOS {
namespace Media {
enum class FaultType : int32_t {
    FAULT_TYPE_INVALID = -1,
    FAULT_TYPE_FREEZE = 0,
    FAULT_TYPE_CRASH = 1,
    FAULT_TYPE_INNER_ERROR = 2,
};

struct SubAbilityCount {
    uint32_t codecCount = 0;
    uint32_t muxerCount = 0;
    uint32_t sourceCount = 0;
    uint32_t demuxerCount = 0;
    uint32_t codeclistCount = 0;
};

class __attribute__((visibility("default"))) AVCodecEvent : public NoCopyable {
public:
    AVCodecEvent() = default;
    ~AVCodecEvent() = default;
    bool CreateMsg(const char* format, ...) __attribute__((__format__(printf, 2, 3)));
    void BehaviorEventWrite(const std::string& eventName,
                            OHOS::HiviewDFX::HiSysEvent::EventType type,
                            const std::string& module);
    void FaultEventWrite(const std::string& eventName,
                         OHOS::HiviewDFX::HiSysEvent::EventType type,
                         FaultType faultType,
                         const std::string& module);

private:
    std::string msg_;
};

__attribute__((visibility("default"))) void BehaviorEventWrite(const std::string& status, const std::string& module);
__attribute__((visibility("default"))) void FaultEventWrite(FaultType faultType, const std::string& msg,
                                                            const std::string& module);
__attribute__((visibility("default"))) void StatisticTimeMemoryEventWrite(uint32_t useTime, const std::string& module);
__attribute__((visibility("default"))) void StatisticEventWrite(const SubAbilityCount& subAbilityCount,
                                                                const std::string& module);

#define AVCODEC_SYNC_TRACE AVCodecTrace trace(std::string(__FUNCTION__))

class __attribute__((visibility("default"))) AVCodecTrace : public NoCopyable {
public:
    explicit AVCodecTrace(const std::string& funcName);
    static void TraceBegin(const std::string& funcName, int32_t taskId);
    static void TraceEnd(const std::string& funcName, int32_t taskId);
    static void CounterTrace(const std::string& varName, int32_t val);
    ~AVCodecTrace();

private:
    bool isSync_ = false;
};
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_DFX_H