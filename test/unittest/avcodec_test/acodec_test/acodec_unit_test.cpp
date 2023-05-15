/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "acodec_unit_test.h"
#include "gtest/gtest.h"
#include "avcodec_errors.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
using namespace OHOS::Media::ACodecTestParam;

void ACodecUnitTest::SetUpTestCase(void) {}

void ACodecUnitTest::TearDownTestCase(void) {}

void ACodecUnitTest::SetUp(void)
{
    createCodecSuccess_ = false;
    std::shared_ptr<ACodecSignal> acodecSignal = std::make_shared<ACodecSignal>();
    adecCallback_ = std::make_shared<ADecCallbackTest>(acodecSignal);
    ASSERT_NE(nullptr, adecCallback_);

    aencCallback_ = std::make_shared<AEncCallbackTest>(acodecSignal);
    ASSERT_NE(nullptr, aencCallback_);

    audioCodec_ = std::make_shared<ACodecMock>(acodecSignal);
    ASSERT_NE(nullptr, audioCodec_);

    defaultFormat_ = AVCodecMockFactory::CreateFormat();
    ASSERT_NE(nullptr, defaultFormat_);
    (void)defaultFormat_->PutIntValue("channel_count", MAX_CHANNEL);
    (void)defaultFormat_->PutIntValue("sample_rate", DEFAULT_SAMPLERATE);
    (void)defaultFormat_->PutLongValue("bitrate", DEFAULT_BITRATE);
    (void)defaultFormat_->PutIntValue("aac_is_adts", 1);

    testInfo_ = ::testing::UnitTest::GetInstance()->current_test_info();
    string prefix = "/data/test/media/";
    string fileName = testInfo_->name();
    string suffix = ".es";
    audioCodec_->SetOutPath(prefix + fileName + suffix);
}

bool ACodecUnitTest::CreateAudioCodecByMime(const std::string &decMime, const std::string &encMime)
{
    if (audioCodec_->CreateAudioDecMockByMime(decMime) == false ||
        audioCodec_->CreateAudioEncMockByMime(encMime) == false ||
        audioCodec_->SetCallbackDec(adecCallback_) != AVCS_ERR_OK ||
        audioCodec_->SetCallbackEnc(aencCallback_) != AVCS_ERR_OK) {
        return false;
    }
    createCodecSuccess_ = true;
    return true;
}

bool ACodecUnitTest::CreateAudioCodecByName(const std::string &decName, const std::string &encName)
{
    if (audioCodec_->CreateAudioDecMockByName(decName) == false ||
        audioCodec_->CreateAudioEncMockByName(encName) == false ||
        audioCodec_->SetCallbackDec(adecCallback_) != AVCS_ERR_OK ||
        audioCodec_->SetCallbackEnc(aencCallback_) != AVCS_ERR_OK) {
        return false;
    }
    createCodecSuccess_ = true;
    return true;
}

void ACodecUnitTest::TearDown(void)
{
    if (audioCodec_ != nullptr && createCodecSuccess_) {
        EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ReleaseDec());
        EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ReleaseEnc());
    }
    if (defaultFormat_ != nullptr) {
        defaultFormat_->Destroy();
    }
}

/**
 * @tc.name: audio_codec_Configure_0100
 * @tc.desc: audio create
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_Configure_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
}

/**
 * @tc.name: audio_codec_0100
 * @tc.desc: audio decodec aac->aac
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_TRUE(defaultFormat_->PutIntValue("profile", 0));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    system("hidumper -s 3002 -a codec");
    sleep(10); // start run 10s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopEnc());
}

/**
 * @tc.name: audio_decodec_flush_0100
 * @tc.desc: audio decodec flush
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_decodec_flush_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(3); // start run 3s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->FlushDec());
    sleep(7); // start run 7s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopEnc());
}

/**
 * @tc.name: audio_encodec_flush_0100
 * @tc.desc: audio encodec flush
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_encodec_flush_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(3); // start run 3s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->FlushEnc());
    sleep(7); // start run 7s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StopEnc());
}

/**
 * @tc.name: audio_codec_reset_0100
 * @tc.desc: audio reset at end of stream
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_reset_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(10); // start run 10s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetEnc());
}

/**
 * @tc.name: audio_codec_reset_0200
 * @tc.desc: audio reset at running state
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_reset_0200, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(2); // start run 2s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetEnc());
}

/**
 * @tc.name: audio_codec_SetParameter_0100
 * @tc.desc: audio codec SetParameter
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_SetParameter_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(2); // start run 2s
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->SetParameterDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->SetParameterEnc(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetEnc());
}

/**
 * @tc.name: audio_codec_GetOutputMediaDescription_0100
 * @tc.desc: audio codec GetOutputMediaDescription
 * @tc.type: FUNC
 * @tc.require: issueI5OWXY issueI5OXCD
 */
HWTEST_F(ACodecUnitTest, audio_codec_GetOutputMediaDescription_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_aac", "avdec_aac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->StartEnc());
    sleep(2); // start run 2s
    EXPECT_NE(nullptr, audioCodec_->GetOutputMediaDescriptionDec());
    EXPECT_NE(nullptr, audioCodec_->GetOutputMediaDescriptionEnc());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->ResetEnc());
}

/**
 * @tc.name: audio_codec_format_vorbis_0100
 * @tc.desc: test audio codec format vorbis
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ACodecUnitTest, audio_codec_format_vorbis_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_vorbis", "avdec_vorbis"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
}

/**
 * @tc.name: audio_codec_format_flac_0100
 * @tc.desc: test audio codec format flac
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ACodecUnitTest, audio_codec_format_flac_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_flac", "avdec_flac"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
}

/**
 * @tc.name: audio_codec_format_mp3_0100
 * @tc.desc: test audio codec format flac
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ACodecUnitTest, audio_codec_format_mp3_0100, TestSize.Level0)
{
    ASSERT_TRUE(CreateAudioCodecByName("avdec_mp3", "avdec_mp3"));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureDec(defaultFormat_));
    ASSERT_EQ(AVCS_ERR_OK, audioCodec_->ConfigureEnc(defaultFormat_));
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareDec());
    EXPECT_EQ(AVCS_ERR_OK, audioCodec_->PrepareEnc());
}

/**
 * @tc.name: audio_codec_format_none_0100
 * @tc.desc: test audio codec format none
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ACodecUnitTest, audio_codec_format_none_0100, TestSize.Level0)
{
    CreateAudioCodecByMime("", "");
    ASSERT_NE(AVCS_ERR_OK, audioCodec_->ReleaseDec());
    ASSERT_NE(AVCS_ERR_OK, audioCodec_->ReleaseEnc());
}