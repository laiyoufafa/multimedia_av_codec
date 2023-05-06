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
}

void AudioCodeDecoderUnitTest::TearDown(void)
{
  cout << "[TearDown]: over!!!" << endl;
}

int32_t AudioCodeDecoderUnitTest::ProceFunc(void)
{
    return AVCS_ERR_OK;
}

HWTEST_F(AudioCodeDecoderUnitTest, audioDecoder_Configure_01, TestSize.Level1)
{ 
}
int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}