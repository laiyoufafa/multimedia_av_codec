#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fcntl.h>
#include "avmuxer.h"
#include "avcodec_info.h"
#include "avmuxer_unit_test.h"

using namespace testing::ext;
using namespace OHOS::Media;

static const std::string TEST_FILE_PATH = "./";


void AVMuxerUnitTest::SetUpTestCase() {}

void AVMuxerUnitTest::TearDownTestCase() {}

void AVMuxerUnitTest::SetUp()
{
    avmuxer_ = std::make_shared<AVMuxerSample>();
    avDataFile_ = std::make_shared<std::ifstream>();
    avDataFile_->open("avDataMpegMpeg4.bin", std::ios::in | std::ios::binary);
    if (avDataFile_ == nullptr) {
        std::cout<<"open avDataMpegMpeg4.bin failed!"<<std::endl;
    }
}

void AVMuxerUnitTest::TearDown()
{
    if (avDataFile_ != nullptr) {
        avDataFile_->close();
        avDataFile_ = nullptr;
    }

    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }

    if (avmuxer_ != nullptr) {
        avmuxer_->Destroy();
        avmuxer_ = nullptr;
    }
}

/**
 * @tc.name: Muxer_Create_001
 * @tc.desc: Muxer Create
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_Create_0100, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);
}

/**
 * @tc.name: Muxer_Create_002
 * @tc.desc: Muxer Create write only
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_Create_002, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_FALSE(isCreated);
}

/**
 * @tc.name: Muxer_Create_003
 * @tc.desc: Muxer Create read only
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_Create_003, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_FALSE(isCreated);
}

/**
 * @tc.name: Muxer_Create_004
 * @tc.desc: Muxer Create rand fd
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_Create_004, TestSize.Level0)
{
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(999999, outputFormat);
    ASSERT_FALSE(isCreated);

    avmuxer_->Destroy();
    isCreated = avmuxer_->CreateMuxer(0xfffffff, outputFormat);
    ASSERT_FALSE(isCreated);

    avmuxer_->Destroy();
    isCreated = avmuxer_->CreateMuxer(-1, outputFormat);
    ASSERT_FALSE(isCreated);

    avmuxer_->Destroy();
    isCreated = avmuxer_->CreateMuxer(-999, outputFormat);
    ASSERT_FALSE(isCreated);
}

/**
 * @tc.name: Muxer_Create_005
 * @tc.desc: Muxer Create outputFormat
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_Create_005, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    avmuxer_->Destroy();
    outputFormat = OUTPUT_FORMAT_M4A;
    isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    avmuxer_->Destroy();
    outputFormat = OUTPUT_FORMAT_UNKNOWN;
    isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    avmuxer_->Destroy();
    isCreated = avmuxer_->CreateMuxer(fd_, static_cast<OutputFormat>(-99));
    ASSERT_TRUE(isCreated);
}

/**
 * @tc.name: avmuxer_AddTrack_0100
 * @tc.desc: Muxer AddTrack add audio track
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_AddTrack_0100, TestSize.Level0)
{
    int audioTrackId = -1;
    int ret = 0;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> audioParams = FormatMockFactory::CreateFormat();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer_->AddTrack(audioTrackId, audioParams);
    EXPECT_EQ(ret, AV_ERR_OK);
    EXPECT_GE(audioTrackId, 0);

    audioParams = FormatMockFactory::CreateFormat();
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer_->AddTrack(audioTrackId, audioParams);
    EXPECT_NE(ret, AV_ERR_OK);

    audioParams = FormatMockFactory::CreateFormat();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer_->AddTrack(audioTrackId, audioParams);
    EXPECT_NE(ret, AV_ERR_OK);

    audioParams = FormatMockFactory::CreateFormat();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    ret = avmuxer_->AddTrack(audioTrackId, audioParams);
    EXPECT_NE(ret, AV_ERR_OK);
}

/**
 * @tc.name: avmuxer_AddTrack_0200
 * @tc.desc: Muxer AddTrack add video track
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_AddTrack_0200, TestSize.Level0)
{
    int32_t videoTrackId = -1;
    int32_t ret = AV_ERR_INVALID_VAL;
    std::string outputFile = TEST_FILE_PATH + std::string("avmuxer_AddTrack_002.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    EXPECT_EQ(ret, AV_ERR_OK);
    EXPECT_GE(videoTrackId, 0);

    videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    EXPECT_NE(ret, AV_ERR_OK);

    videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    EXPECT_NE(ret, AV_ERR_OK);

    videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    EXPECT_NE(ret, AV_ERR_OK);
}

/**
 * @tc.name: avmuxer_AddTrack_0300
 * @tc.desc: Muxer AddTrack after Start()
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_AddTrack_0300, TestSize.Level0)
{
    int32_t audioTrackId = -1;
    int32_t videoTrackId = -1;
    int32_t ret = AV_ERR_INVALID_VAL;
    std::string outputFile = TEST_FILE_PATH + std::string("avmuxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);
    ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    EXPECT_EQ(ret, AV_ERR_OK);
    EXPECT_GE(videoTrackId, 0);

    avmuxer_->Start();
    std::shared_ptr<FormatMock> audioParams = FormatMockFactory::CreateFormat();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer_->AddTrack(audioTrackId, audioParams);
    EXPECT_NE(ret, AV_ERR_OK);
}

/**
 * @tc.name: Muxer_AddTrack_004
 * @tc.desc: Muxer AddTrack mimeType test
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_AddTrack_004, TestSize.Level0)
{
    int trackId = -1;
    int ret = 0;
    const std::vector<std::string_view> testMp4MimeTypeList = 
    {
        CodecMimeType::AUDIO_MPEG, 
        // CodecMimeType::AUDIO_FLAC,
        // CodecMimeType::AUDIO_RAW,
        CodecMimeType::AUDIO_AAC,
        // CodecMimeType::AUDIO_VORBIS,
        // CodecMimeType::AUDIO_OPUS, 
        // CodecMimeType::AUDIO_AMR_NB,
        // CodecMimeType::AUDIO_AMR_WB,
        CodecMimeType::VIDEO_AVC,
        CodecMimeType::VIDEO_MPEG4,
        CodecMimeType::IMAGE_JPG,
        CodecMimeType::IMAGE_PNG,
        CodecMimeType::IMAGE_BMP,
    };

    const std::vector<std::string_view> testM4aMimeTypeList = 
    {
        CodecMimeType::AUDIO_AAC,
        CodecMimeType::VIDEO_AVC,
        CodecMimeType::VIDEO_MPEG4,
        CodecMimeType::IMAGE_JPG,
        CodecMimeType::IMAGE_PNG,
        CodecMimeType::IMAGE_BMP,
    };

    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;

    std::shared_ptr<FormatMock> avParam = FormatMockFactory::CreateFormat();
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);

    for (uint32_t i = 0; i < testMp4MimeTypeList.size(); ++i) {
        bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
        ASSERT_TRUE(isCreated);
        avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, testMp4MimeTypeList[i]);
        ret = avmuxer_->AddTrack(trackId, avParam);
        EXPECT_EQ(ret, AV_ERR_OK) << "AddTrack filed: i:" << i << " mimeType:"<<testMp4MimeTypeList[i];
        EXPECT_EQ(trackId, 0) << "i:" << i << " TrackId:" << trackId << " mimeType:"<<testMp4MimeTypeList[i];
    }

    // need to change libohosffmpeg.z.so, muxer build config add ipod
    avmuxer_->Destroy();
    outputFormat = OUTPUT_FORMAT_M4A;
    for (uint32_t i = 0; i < testM4aMimeTypeList.size(); ++i) {
        bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
        ASSERT_TRUE(isCreated);
        avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, testM4aMimeTypeList[i]);
        ret = avmuxer_->AddTrack(trackId, avParam);
        EXPECT_EQ(ret, AV_ERR_OK) << "AddTrack filed: i:" << i << " mimeType:"<<testM4aMimeTypeList[i];
        EXPECT_EQ(trackId, 0) << "i:" << i << " TrackId:" << trackId << " mimeType:"<<testM4aMimeTypeList[i];
    }
}

/**
 * @tc.name: Muxer_AddTrack_005
 * @tc.desc: Muxer AddTrack while outputFormat unexpect
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, Muxer_AddTrack_005, TestSize.Level0)
{
    int trackId = -2;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_UNKNOWN;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> avParam = FormatMockFactory::CreateFormat();
    avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);

    int ret = avmuxer_->AddTrack(trackId, avParam);
    ASSERT_NE(ret, 0);
}

/**
 * @tc.name: Muxer_Start_001
 * @tc.desc: Muxer Start after Stop()
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_Start_0100, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("avmuxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);

    EXPECT_NE(avmuxer_->Start(), 0);
    int32_t videoTrackId = -1;
    int32_t ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    ASSERT_EQ(ret, AV_ERR_OK);
    EXPECT_GE(videoTrackId, 0);
    EXPECT_EQ(avmuxer_->Start(), 0);
    EXPECT_NE(avmuxer_->Start(), 0);
    EXPECT_EQ(avmuxer_->Stop(), 0);
    EXPECT_NE(avmuxer_->Start(), 0);
}

/**
 * @tc.name: avmuxer_Stop_0100
 * @tc.desc: Muxer Stop() before Start
 * @tc.type: FUNC
 */
HWTEST_F(AVMuxerUnitTest, avmuxer_Stop_0100, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("avmuxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    bool isCreated = avmuxer_->CreateMuxer(fd_, outputFormat);
    ASSERT_TRUE(isCreated);

    std::shared_ptr<FormatMock> videoParams = FormatMockFactory::CreateFormat();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);
    int32_t videoTrackId = -1;
    int32_t ret = avmuxer_->AddTrack(videoTrackId, videoParams);
    ASSERT_EQ(ret, AV_ERR_OK);
    EXPECT_GE(videoTrackId, 0);
    EXPECT_NE(avmuxer_->Stop(), 0);
    EXPECT_EQ(avmuxer_->Start(), 0);
    EXPECT_EQ(avmuxer_->Stop(), 0);
    EXPECT_NE(avmuxer_->Stop(), 0);
}