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

#ifndef AVCODEC_XCOLLIE_H
#define AVCODEC_XCOLLIE_H

#include <string>
#include <map>
#include <mutex>

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) AVCodecXCollie {
public:
    static AVCodecXCollie &GetInstance();
    int32_t SetTimer(const std::string &name, bool recovery = false, uint32_t timeout = 10); // 10s
    int32_t SetTimerByLog(const std::string &name, uint32_t timeout = 10); // 10s
    void CancelTimer(int32_t id);
    int32_t Dump(int32_t fd);
    constexpr static uint32_t timerTimeout = 10;
private:
    AVCodecXCollie() = default;
    ~AVCodecXCollie() = default;
    void TimerCallback(void *data);

    std::mutex mutex_;
    std::map<int32_t, std::string> dfxDumper_;
    uint32_t threadDeadlockCount_ = 0;
};

class __attribute__((visibility("hidden"))) AVCodecXcollieTimer {
public:
    AVCodecXcollieTimer(const std::string &name, bool recovery = false, uint32_t timeout = 10)
    {
        id_ = AVCodecXCollie::GetInstance().SetTimer(name, recovery, timeout);
    };

    AVCodecXcollieTimer(const std::string &name, uint32_t timeout)
    {
        id_ = AVCodecXCollie::GetInstance().SetTimerByLog(name, timeout);
    }

    ~AVCodecXcollieTimer()
    {
        AVCodecXCollie::GetInstance().CancelTimer(id_);
    }
private:
    int32_t id_ = 0;
};

#define COLLIE_LISTEN(statement, args...) { AVCodecXcollieTimer xCollie(args); statement; }
} // namespace Media
} // namespace OHOS
#endif // AVCODEC_XCOLLIE_H