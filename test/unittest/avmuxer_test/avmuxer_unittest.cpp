#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
 * @tc.name: Muxer_Create_001
 * @tc.desc: Muxer Create
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_001, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);
}

/**
 * @tc.name: Muxer_Create_002
 * @tc.desc: Muxer Create write only
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_002, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);
}

/**
 * @tc.name: Muxer_Create_003
 * @tc.desc: Muxer Create read only
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_003, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);
}

/**
 * @tc.name: Muxer_Create_004
 * @tc.desc: Muxer Create rand fd
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_004, TestSize.Level0)
{
    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(999999, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);

    avmuxer = AVMuxerFactory::CreateAVMuxer(0xfffffff, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);

    avmuxer = AVMuxerFactory::CreateAVMuxer(-1, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);

    avmuxer = AVMuxerFactory::CreateAVMuxer(-999, outputFormat);
    ASSERT_EQ(avmuxer, nullptr);
}

/**
 * @tc.name: Muxer_Create_005
 * @tc.desc: Muxer Create outputFormat
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_Create_005, TestSize.Level0)
{
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_Create.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    OutputFormat outputFormat = OUTPUT_FORMAT_MPEG_4;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    outputFormat = OUTPUT_FORMAT_M4A;
    avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    outputFormat = OUTPUT_FORMAT_UNKNOWN;
    avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, static_cast<OutputFormat>(-99));
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
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack.mp4");
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
 * @tc.name: Muxer_AddTrack_004
 * @tc.desc: Muxer AddTrack mimeType test
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_AddTrack_004, TestSize.Level0)
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

    std::shared_ptr<MediaDescription> avParam = std::make_shared<MediaDescription>();
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);

    for (int i = 0; i < testMp4MimeTypeList.size(); ++i) {
        std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
        ASSERT_NE(avmuxer, nullptr);
        avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, testMp4MimeTypeList[i]);
        ret = avmuxer->AddTrack(trackId, *avParam);
        EXPECT_EQ(ret, 0) << "AddTrack filed: i:" << i << " mimeType:"<<testMp4MimeTypeList[i];
        EXPECT_EQ(trackId, 0) << "i:" << i << " TrackId:" << trackId << " mimeType:"<<testMp4MimeTypeList[i];
    }

    // 需要替换系统的libohosffmpeg.z.so, 开启muxer的ipod
    outputFormat = OUTPUT_FORMAT_M4A;
    for (int i = 0; i < testM4aMimeTypeList.size(); ++i) {
        std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
        ASSERT_NE(avmuxer, nullptr);
        avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, testM4aMimeTypeList[i]);
        ret = avmuxer->AddTrack(trackId, *avParam);
        EXPECT_EQ(ret, 0) << "AddTrack filed: i:" << i << " mimeType:"<<testM4aMimeTypeList[i];
        EXPECT_EQ(trackId, 0) << "i:" << i << " TrackId:" << trackId << " mimeType:"<<testM4aMimeTypeList[i];
    }
}

/**
 * @tc.name: Muxer_AddTrack_005
 * @tc.desc: Muxer AddTrack while outputFormat unexpect
 * @tc.type: FUNC
 */
HWTEST_F(AvmuxerUnitTest, Muxer_AddTrack_005, TestSize.Level0)
{
    int trackId = -2;
    std::string outputFile = TEST_FILE_PATH + std::string("Muxer_AddTrack.mp4");
    fd_ = open(outputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    OutputFormat outputFormat = OUTPUT_FORMAT_UNKNOWN;
    std::shared_ptr<AVMuxer> avmuxer = AVMuxerFactory::CreateAVMuxer(fd_, outputFormat);
    ASSERT_NE(avmuxer, nullptr);

    std::shared_ptr<MediaDescription> avParam = std::make_shared<MediaDescription>();
    avParam->PutStringValue(MediaDescriptionKey::MD_KEY_CODEC_MIME, CodecMimeType::AUDIO_MPEG);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_SAMPLE_RATE, 44100);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_CHANNEL_COUNT, 2);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_WIDTH, 352);
    avParam->PutIntValue(MediaDescriptionKey::MD_KEY_HEIGHT, 288);

    int ret = avmuxer->AddTrack(trackId, *avParam);
    ASSERT_NE(ret, 0);
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