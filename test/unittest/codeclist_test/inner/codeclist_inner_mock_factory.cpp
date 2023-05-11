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

#include "codeclist_mock.h"
#include "codeclist_inner_mock.h"

namespace OHOS {
namespace Media {
std::shared_ptr<CodecListMock> CodecListMockFactory::CreateCodecList()
{
    auto codeclist = AVCodecListFactory::CreateAVCodecList();
    if (codeclist != nullptr) {
        return std::make_shared<CodecListInnerMock>(codeclist);
    }
    return nullptr;
}
}  // namespace Media
}  // namespace OHOS