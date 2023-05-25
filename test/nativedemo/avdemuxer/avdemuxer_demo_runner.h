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
#ifndef AVDEMUXER_DEMO_RUNNER_H
#define AVDEMUXER_DEMO_RUNNER_H

#include "capi_demo/avsource_demo.h"

static void RunNativeDemuxer(const std::string filePath);
static void RunInnerSourceDemuxer(const std::string filePath);
void AVSourceDemuxerDemoCase(void);

#endif  // AVDEEMUXER_DEMO_RUNNER_H