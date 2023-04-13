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
#ifndef AVCODEC_LIST_IMPL_H
#define AVCODEC_LIST_IMPL_H

#include "avcodec_info.h"
#include "avcodec_list.h"
#include "nocopyable.h"
#include "i_avcodeclist_service.h"

namespace OHOS {
namespace MediaAVCodec {
class AVCodecListImpl : public AVCodecList, public NoCopyable {
public:
    AVCodecListImpl();
    ~AVCodecListImpl();
    int32_t Init();

    std::string FindVideoDecoder(const Format &format) override;
    std::string FindVideoEncoder(const Format &format) override;
    std::string FindAudioDecoder(const Format &format) override;
    std::string FindAudioEncoder(const Format &format) override;
    CapabilityData GetCapabilityData(std::string codecName) override;
    
private:
    std::shared_ptr<IAVCodecListService> codecListService_ = nullptr;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // AVCODEC_LIST_IMPL_H

