
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

#include "native_avcodec_codeclist.h"
#include "native_avmagic.h"

using namespace OHOS::AVCodec;

char *OH_AVCodec_FindVideoEncoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    codeclist->FindVideoEncoder(format->format_);
    return nullptr;
}

char *OH_AVCodec_FindVideoDecoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    codeclist->FindVideoDecoder(format->format_);
    return nullptr;
}

char *OH_AVCodec_FindAudioEncoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    codeclist->FindAudioEncoder(format->format_);
    return nullptr;
}

char *OH_AVCodec_FindAudioDecoder(const OH_AVFormat *format)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    codeclist->FindAudioDecoder(format->format_);
    return nullptr;
}

OH_AVCapability *OH_AVCodec_GetCapability(const char *name)
{
    std::shared_ptr<AVCodecList> codeclist = AVCodecListFactory::CreateAVCodecList();
    std::string nameStr;
    nameStr.push_back(name)
    CapabilityData capabilityData = codeclist->GetCapability(nameStr);
    return OH_AVCapability(capabilityData);
}