#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include "avmuxer.h"
#include "avcodec_info.h"

using namespace testing::ext;
using namespace OHOS::Media;

static const std::string TEST_FILE_PATH = "./";

class AvmuxerUnitTest : public testing::Test {
    public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
protected:
    int32_t fd_ {-1};
    std::shared_ptr<std::ifstream> avDataFile_ {nullptr};
    std::shared_ptr<AVMuxer> avmuxer_ {nullptr};
    uint8_t buffer_[3] = {'a', 'a', 'a'};
};

void AvmuxerUnitTest::SetUpTestCase() {}

void AvmuxerUnitTest::TearDownTestCase() {}

void AvmuxerUnitTest::SetUp()
{
    avDataFile_ = std::make_shared<std::ifstream>();
    avDataFile_->open("avDataMpegMpeg4.bin", std::ios::in | std::ios::binary);
    if (avDataFile_ == nullptr) {
        std::cout<<"open avDataMpegMpeg4.bin failed!"<<std::endl;
    }
}

void AvmuxerUnitTest::TearDown()
{
    if (avDataFile_ != nullptr) {
        avDataFile_->close();
        avDataFile_ = nullptr;
    }

    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

/**
 * @tc.name: Muxer_AddTrack_001
 * @tc.desc: Muxer AddTrack add audio track
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_001, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_001.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);

    close(fd_);
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    outputFormat = OUTPUT_FORMAT_MPEG_4;
    avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);

    close(fd_);
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    outputFormat = OUTPUT_FORMAT_MPEG_4;
    avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);
}

/**
 * @tc.name: Muxer_AddTrack_001
 * @tc.desc: Muxer AddTrack add audio track
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_AddTrack_001, TestSize.Level0)
{
    int audioTrackId = -1;
    int ret = 0;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_001.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> audioParams = std::make_shared<MediaDescription>();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer->AddTrack(audioTrackId, *audioParams);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(audioTrackId, 0);

    audioParams = std::make_shared<MediaDescription>();
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer->AddTrack(audioTrackId, *audioParams);
    EXPECT_NE(ret, 0);

    audioParams = std::make_shared<MediaDescription>();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer->AddTrack(audioTrackId, *audioParams);
    EXPECT_NE(ret, 0);

    audioParams = std::make_shared<MediaDescription>();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    ret = avmuxer->AddTrack(audioTrackId, *audioParams);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: Muxer_AddTrack_002
 * @tc.desc: Muxer AddTrack add video track
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_AddTrack_002, TestSize.Level0)
{
    int videoTrackId = -1;
    int ret = 0;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_002.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(videoTrackId, 0);

    videoParams = std::make_shared<MediaDescription>();
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    EXPECT_NE(ret, 0);

    videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    EXPECT_NE(ret, 0);

    videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: Muxer_AddTrack_003
 * @tc.desc: Muxer AddTrack after Start()
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_AddTrack_003, TestSize.Level0)
{
    int audioTrackId = -1;
    int videoTrackId = -1;
    int ret = 0;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);
    ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(videoTrackId, 0);

    avmuxer->Start();
    std::shared_ptr<MediaDescription> audioParams = std::make_shared<MediaDescription>();
    audioParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    audioParams->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    ret = avmuxer->AddTrack(audioTrackId, *audioParams);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: Muxer_Start_001
 * @tc.desc: Muxer Start after Stop()
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Start_001, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);

    EXPECT_NE(avmuxer->Start(), 0);
    int32_t videoTrackId = -1;
    int ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    ASSERT_EQ(ret, 0);
    EXPECT_GE(videoTrackId, 0);
    EXPECT_EQ(avmuxer->Start(), 0);
    EXPECT_NE(avmuxer->Start(), 0);
    EXPECT_EQ(avmuxer->Stop(), 0);
    EXPECT_NE(avmuxer->Start(), 0);
}

/**
 * @tc.name: Muxer_Stop_001
 * @tc.desc: Muxer Stop() before Start
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Stop_001, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack_003.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> videoParams = std::make_shared<MediaDescription>();
    videoParams->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::VIDEO_MPEG4);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    videoParams->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);
    videoParams->PutBuffer(MediaDescriptionKey::MD_KEY_CODEC_CONFIG, buffer_, 3);
    int32_t videoTrackId = -1;
    int ret = avmuxer->AddTrack(videoTrackId, *videoParams);
    ASSERT_EQ(ret, 0);
    EXPECT_GE(videoTrackId, 0);
    EXPECT_NE(avmuxer->Stop(), 0);
    EXPECT_EQ(avmuxer->Start(), 0);
    EXPECT_EQ(avmuxer->Stop(), 0);
    EXPECT_NE(avmuxer->Stop(), 0);
}