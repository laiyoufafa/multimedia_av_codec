#include <vector>
#include <queue>
#include <mutex>
#include <gtest/gtest.h>
#include "native_avcodec_audiodecoder.h"
#include "audio_ffmpeg_adapter.h"
#include "format.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"

extern "C" {
#include <string.h>
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavcodec/avcodec.h"
}

using namespace std;
using namespace testing::ext;
using namespace OHOS::Media;

namespace {
const string CODEC_NAME = "OH.Media.Codec.MP3.FFMPEGMp3";
} // namespace

class ADecSignal {
public:
    std::mutex inMutex_;
    std::mutex outMutex_;
    std::condition_variable inCond_;
    std::condition_variable outCond_;
    std::queue<int32_t> inIdxQueue_;
    std::queue<int32_t> outIdxQueue_;
    std::queue<AVCodecBufferInfo> infoQueue_;
    std::queue<AVCodecBufferFlag> flagQueue_;
};

class BufferCallback : public AVCodecCallback {
public:
    explicit BufferCallback(ADecSignal *userData) : userData_(userData){}
    virtual ~BufferCallback() = default;
    ADecSignal *userData_;
    virtual void OnError(AVCodecErrorType errorType, int32_t errorCode) override;
    virtual void OnOutputFormatChanged(const Format &format) override;
    virtual void OnInputBufferAvailable(size_t index) override;
    virtual void OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag) override;
};

void BufferCallback::OnError(AVCodecErrorType errorType, int32_t errorCode)
{
    (void)errorType;
    cout << "Error errorCode=" << errorCode << endl;
}

void BufferCallback::OnOutputFormatChanged(const Format &format)
{
    (void)format;
    cout << "Format Changed" << endl;
}

void BufferCallback::OnInputBufferAvailable(size_t index)
{
    unique_lock<mutex> lock(userData_->inMutex_);
    userData_->inIdxQueue_.push(index);
    userData_->inCond_.notify_all();
}

void BufferCallback::OnOutputBufferAvailable(size_t index, AVCodecBufferInfo info, AVCodecBufferFlag flag)
{
    unique_lock<mutex> lock(userData_->outMutex_);
    userData_->outIdxQueue_.push(index);
    userData_->infoQueue_.push(info);
    userData_->flagQueue_.push(flag);
    userData_->outCond_.notify_all();
    (void)flag;
}


class AudioCodeDecoderUnitTest : public testing::Test {
  public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t ProceFunc();

  
  protected:
    int32_t index_;
    int64_t timeStamp_{0};

    ADecSignal *signal_;

    FILE *inFile_ {nullptr};
    FILE *dumpFd_ {nullptr};
    std::string codecName_;

    OHOS::Media::Format format_;
    std::shared_ptr<OHOS::Media::AudioFFMpegAdapter> adec_ {nullptr};
};

void AudioCodeDecoderUnitTest::SetUpTestCase(void)
{   
    cout << "[SetUpTestCase]: " << endl;
}

void AudioCodeDecoderUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AudioCodeDecoderUnitTest::SetUp(void)
{
    //TODO:need help
    codecName_ = "OH.Media.Codec.MP3.FFMPEGMp3";
    adec_ = std::make_shared<OHOS::Media::AudioFFMpegAdapter>(codecName_);
    //OH_AudioDecoder_CreateByName("OH.Media.Codec.MP3.FFMPEGMp3");
    ASSERT_NE(nullptr, adec_);
    // testFile_ = std::make_unique<std::ifstream>();
    // testFile_->open("/data/media/audio.mp3", std::ios::in | std::ios::binary);

    signal_ = new ADecSignal();
    ASSERT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->SetCallback(std::shared_ptr<AVCodecCallback>(new BufferCallback(signal_))));
}

void AudioCodeDecoderUnitTest::TearDown(void)
{
  cout << "[TearDown]: over!!!" << endl;
}


int32_t AudioCodeDecoderUnitTest::ProceFunc(void)
{   
    format_.PutIntValue("channel-count", 2);
    format_.PutIntValue("simplerate", 8000);
    format_.PutIntValue("bitrate", 128000);

    if (adec_->Configure(format_) != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }else if (adec_->Start() != AVCodecServiceErrCode::AVCS_ERR_OK) {
        return AVCodecServiceErrCode::AVCS_ERR_UNKNOWN;
    }
    return AVCodecServiceErrCode::AVCS_ERR_OK;
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_01, TestSize.Level1)
{ 
    format_.PutIntValue("width", 0);
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_CONFIGURE_MISMATCH_CHANNEL_COUNT, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_02, TestSize.Level1)
{ 
    format_.PutIntValue("channel-count", 2);
    format_.PutIntValue("simplerate", 8000);
    format_.PutIntValue("bitrate", 128000);

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Configure(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Start_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Pause_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Pause());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Resume_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Pause());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Resume());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Stop_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Flush_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Flush());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Reset_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Reset());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Release_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Release());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_NotifyEos_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->Stop());
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->NotifyEos());
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_SetParameter_01, TestSize.Level1)
{ 
    //尚未实现
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_INVALID_STATE, adec_->SetParameter(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetOutputFormat_01, TestSize.Level1)
{ 
    //尚未实现
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->GetOutputFormat(format_));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetInputBuffer_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;
    index_ = -1;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = 1024;
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    index_ = signal_->inIdxQueue_.front();
    buffer = adec_->GetInputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_QueueInputBuffer_01, TestSize.Level1)
{ 
    AVCodecBufferInfo info;
    AVCodecBufferFlag flag;

    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, ProceFunc());
    // case0 传参异常
    index_ = -1;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL, adec_->QueueInputBuffer(index_, info, flag));
    // case1 info未赋值
    index_ = 1024;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL, adec_->QueueInputBuffer(index_, info, flag));

    // case2 EOS帧数据
    index_ = 0;
    info.presentationTimeUs = 0;
    info.size = -1;
    info.offset = 0;
    flag = AVCODEC_BUFFER_FLAG_EOS;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_OK, adec_->QueueInputBuffer(index_, info, flag));
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_GetOutputBuffer_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());
    std::shared_ptr<OHOS::Media::AVSharedMemory> buffer = nullptr;

    //case1 传参异常
    index_ = -1;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);
    index_ = 1024;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_EQ(nullptr, buffer);

    //case2 传参正常
    index_ = 0;
    buffer = adec_->GetOutputBuffer(index_);
    EXPECT_NE(nullptr, buffer);
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_ReleaseOutputBuffer_01, TestSize.Level1)
{ 
    EXPECT_EQ(AVCS_ERR_OK, ProceFunc());

    //case1 传参异常
    index_ = -1;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL, adec_->ReleaseOutputBuffer(index_));
    index_ = 1024;
    EXPECT_NE(AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL, adec_->ReleaseOutputBuffer(index_));
    //case2 传参正常
    index_ = 0;
    EXPECT_EQ(AVCodecServiceErrCode::AVCS_ERR_INVALID_VAL, adec_->ReleaseOutputBuffer(index_));
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}