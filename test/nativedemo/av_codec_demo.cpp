/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <climits>
#include <iostream>
#include "avcodec_audio_encoder_inner_demo.h"
#include "avcodec_audio_decoder_demo.h"
#include "avcodec_audio_encoder_demo.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::AudioDemo;
using namespace OHOS::Media::InnerAudioDemo;
using namespace std;

static int RunAudioDecoder()
{
    auto audioEnc = std::make_unique<ADecDemo>();
    if (audioEnc == nullptr) {
        cout << "audio decoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioEncoder()
{
    auto audioEnc = std::make_unique<AEncDemo>();
    if (audioEnc == nullptr) {
        cout << "audio encoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio encoder end" << endl;
    return 0;
}

static int RunAudioInnerDecoder()
{
    auto audioEnc = std::make_unique<ADecInnerDemo>();
    if (audioEnc == nullptr) {
        cout << "audio decoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio decoder end" << endl;
    return 0;
}

static int RunAudioInnerEncoder()
{
    auto audioEnc = std::make_unique<ADecInnerDemo>();
    if (audioEnc == nullptr) {
        cout << "audio encoder is null" << endl;
        return 0;
    }
    audioEnc->RunCase();
    cout << "demo audio encoder end" << endl;
    return 0;
}

int main(int argc, char *argv[])
{
    constexpr int minRequiredArgCount = 2;
    string path;
    if (argc >= minRequiredArgCount && argv[1] != nullptr) {
        path = argv[1];
    }
    cout << "Please select a demo scenario number(default Audio Decoder): " << endl;
    cout << "0:Audio Decoder" << endl;
    cout << "1:Audio Encoder" << endl;
    cout << "2:Audio Inner Decoder" << endl;
    cout << "3:Audio Inner Encoder" << endl;

    string mode;
    (void)getline(cin, mode);
    if (mode == "" || mode == "0") {
        (void)RunAudioDecoder();
    } else if (mode == "1") {
        (void)RunAudioEncoder();
    } else if (mode == "2") {
        (void)RunAudioInnerDecoder();
    } else if (mode == "3") {
        (void)RunAudioInnerEncoder();
    } else {
        cout << "no that selection" << endl;
    }
    return 0;
}
