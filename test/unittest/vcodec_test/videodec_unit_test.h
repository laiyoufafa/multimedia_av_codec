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

#ifndef VIDEODEC_UNIT_TEST_H
#define VIDEODEC_UNIT_TEST_H

#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include "vdec_sample.h"

namespace OHOS {
namespace MediaAVCodec {
class VideoDecUnitTest : public testing::TestWithParam<std::string> {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    bool CreateVideoCodecByName(const std::string &decName);
    bool CreateVideoCodecByMime(const std::string &decMime);

    void CreateByNameWithParam(void);
    void CreateByMimeWithParam(void);
    void SetFormatWithParam(void);

protected:
    std::shared_ptr<VideoDecSample> videoDec_ = nullptr;
    std::shared_ptr<FormatMock> format_ = nullptr;
    std::shared_ptr<VDecCallbackTest> vdecCallback_ = nullptr;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // VIDEODEC_UNIT_TEST_H