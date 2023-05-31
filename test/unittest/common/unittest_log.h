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

#ifndef UNITTEST_LOG_H
#define UNITTEST_LOG_H

#include <cstdio>
#include "avcodec_log.h"
#include "securec.h"

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AVCodecUnitTest"};
}
namespace OHOS {
#define LOG_MAX_SIZE 200
#define UNITTEST_CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...)  \
    do {                                                        \
        if (!(cond)) {                                          \
            (void)printf("%s\n", fmt, ##__VA_ARGS__);           \
            return ret;                                         \
        }                                                       \
    } while (0)

#define UNITTEST_CHECK_AND_RETURN_LOG(cond, fmt, ...)           \
    do {                                                        \
        if (!(cond)) {                                          \
            (void)printf("%s\n", fmt, ##__VA_ARGS__);           \
            return;                                             \
        }                                                       \
    } while (0)

#define UNITTEST_CHECK_AND_BREAK_LOG(cond, fmt, ...)            \
        if (!(cond)) {                                          \
            (void)printf("%s\n", fmt, ##__VA_ARGS__);           \
            break;                                              \
        }

#define UNITTEST_CHECK_AND_CONTINUE_LOG(cond, fmt, ...)         \
        if (!(cond)) {                                          \
            (void)printf("%s\n", fmt, ##__VA_ARGS__);           \
            continue;                                           \
        }
#define UNITTEST_INFO_LOG(fmt, ...)                                        \
        do {                                                               \
            char ch[LOG_MAX_SIZE];                                         \
            (void)sprintf_s(ch, LOG_MAX_SIZE, fmt, ##__VA_ARGS__);         \
            (void)printf("%s", ch);                                        \
            (void)printf("\n");                                            \
            AVCODEC_LOG(::OHOS::HiviewDFX::HiLog::Info, "%{public}s", ch);   \
        } while (0)
}

#endif // UNITTEST_LOG_H