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

#ifndef HCODEC_HCODEC_H
#define HCODEC_HCODEC_H

#include <queue>
#include <functional>
#include "securec.h"
#include "OMX_Component.h"  // third_party/openmax/api/1.1.2
#include "codecbase.h"
#include "avcodec_errors.h"
#include "avsharedmemorybase.h"  // foundation/multimedia/av_codec/services/utils/include/
#include "state_machine.h"
#include "v1_0/codec_types.h"
#include "v1_0/icodec_callback.h"
#include "v1_0/icodec_component.h"
#include "v1_0/icodec_component_manager.h"

namespace OHOS::MediaAVCodec {
class HCodec : public CodecBase, protected StateMachine {
public:
    static std::shared_ptr<HCodec> Create(const std::string &name);
    int32_t SetCallback(const std::shared_ptr<AVCodecCallback> &callback) override;
    int32_t Configure(const Format &format) override;
    sptr<Surface> CreateInputSurface() override;
    int32_t SetInputSurface(sptr<Surface> surface);
    int32_t SetOutputSurface(sptr<Surface> surface) override;

    std::shared_ptr<AVSharedMemoryBase> GetInputBuffer(uint32_t index) override;
    int32_t QueueInputBuffer(uint32_t index, const AVCodecBufferInfo &info, AVCodecBufferFlag flag) override;
    int32_t NotifyEos() override;
    std::shared_ptr<AVSharedMemoryBase> GetOutputBuffer(uint32_t index) override;
    int32_t ReleaseOutputBuffer(uint32_t index) override;
    int32_t RenderOutputBuffer(uint32_t index) override;

    int32_t SignalRequestIDRFrame() override;
    int32_t SetParameter(const Format& format) override;
    int32_t GetInputFormat(Format& format) override;
    int32_t GetOutputFormat(Format& format) override;

    int32_t Start() override;
    int32_t Stop() override;
    int32_t Flush() override;
    int32_t Reset() override;
    int32_t Release() override;

protected:
    enum MsgWhat : MsgType {
        INIT,
        SET_CALLBACK,
        CONFIGURE,
        CREATE_INPUT_SURFACE,
        SET_INPUT_SURFACE,
        SET_OUTPUT_SURFACE,
        START,
        GET_INPUT_FORMAT,
        GET_OUTPUT_FORMAT,
        SET_PARAMETERS,
        REQUEST_IDR_FRAME,
        FLUSH,
        GET_INPUT_BUFFER,
        QUEUE_INPUT_BUFFER,
        NOTIFY_EOS,
        GET_OUTPUT_BUFFER,
        RELEASE_OUTPUT_BUFFER,
        RENDER_OUTPUT_BUFFER,
        STOP,
        RELEASE,
        RESET,

        INNER_MSG_BEGIN = 1000,
        CODEC_EVENT,
        OMX_EMPTY_BUFFER_DONE,
        OMX_FILL_BUFFER_DONE,
        GET_BUFFER_FROM_SURFACE,
        CHECK_IF_STUCK,
        FORCE_SHUTDOWN,
    };

    enum BufferOperationMode {
        KEEP_BUFFER,
        RESUBMIT_BUFFER,
        FREE_BUFFER,
    };

    enum class BufferOwner {
        OWNED_BY_US,
        OWNED_BY_USER,
        OWNED_BY_OMX,
        OWNED_BY_SURFACE,
    };

    enum class BufferType {
        DYNAMIC_SURFACE_BUFFER,
        PRESET_SURFACE_BUFFER,
        PRESET_ASHM_BUFFER,
    };

    struct PortInfo {
        uint32_t width;
        uint32_t height;
        std::optional<uint32_t> stride;
        OMX_VIDEO_CODINGTYPE codingType;
        GraphicPixelFormat pixelFmt;
        double frameRate;
        std::optional<uint32_t> inputBufSize;
    };

    struct BufferInfo {
        BufferOwner owner;
        uint32_t bufferId;
        std::shared_ptr<OHOS::HDI::Codec::V1_0::OmxCodecBuffer> omxBuffer;
        sptr<SurfaceBuffer> surfaceBuffer;
        std::shared_ptr<AVSharedMemoryBase> sharedBuffer;
        bool isImageDataInSharedBuffer = false;

        const char* Owner() const;
        void Dump(const std::string& prefix, const std::optional<PortInfo>& bufferFormat) const;

    private:
        void DumpSurfaceBuffer(const std::string& prefix) const;
        void DecideDumpInfo(std::optional<uint32_t>& assumeAlignedH, std::string& suffix, bool& dumpAsVideo) const;
        void DumpAshmemBuffer(const std::string& prefix, const std::optional<PortInfo>& bufferFormat) const;
        static constexpr char DUMP_PATH[] = "/data/misc/hcodecdump";
    };

protected:
    HCodec(OMX_VIDEO_CODINGTYPE codingType, bool isEncoder);
    ~HCodec() override;

    // configure
    virtual int32_t OnConfigure(const Format &format) = 0;
    int32_t SetVideoPortInfo(OMX_DIRTYPE portIndex, const PortInfo& info);
    virtual int32_t UpdateInPortFormat() = 0;
    virtual int32_t UpdateOutPortFormat() = 0;
    void PrintPortDefinition(const OMX_PARAM_PORTDEFINITIONTYPE& def);

    virtual int32_t OnSetOutputSurface(const sptr<Surface> &surface) { return AVCS_ERR_UNSUPPORT; }
    virtual int32_t OnSetParameters(const Format &format) { return AVCS_ERR_OK; }
    virtual sptr<Surface> OnCreateInputSurface() { return nullptr; }
    virtual int32_t OnSetInputSurface(sptr<Surface> &inputSurface) { return AVCS_ERR_UNSUPPORT; }
    virtual int32_t RequestIDRFrame() { return AVCS_ERR_UNSUPPORT; }

    // start
    virtual bool ReadyToStart() = 0;
    virtual int32_t AllocateBuffersOnPort(OMX_DIRTYPE portIndex) = 0;
    int32_t AllocateSharedBuffers(OMX_DIRTYPE portIndex, bool isImageData);
    std::shared_ptr<OHOS::HDI::Codec::V1_0::OmxCodecBuffer> AshmemToOmxBuffer(
        OMX_DIRTYPE portIndex, int32_t fd, uint32_t size);

    virtual int32_t SubmitAllBuffersOwnedByUs() = 0;
    virtual int32_t SubmitOutputBuffersToOmxNode() = 0;
    BufferInfo* FindBufferInfoByID(OMX_DIRTYPE portIndex, uint32_t bufferId);
    std::optional<size_t> FindBufferIndexByID(OMX_DIRTYPE portIndex, uint32_t bufferId);
    void PrintAllBufferInfo();
    virtual void OnGetBufferFromSurface() = 0;

    // input buffer circulation
    virtual void NotifyUserToFillThisInputBuffer(BufferInfo &info);
    virtual std::shared_ptr<AVSharedMemoryBase> OnUserGetInputBuffer(uint32_t bufferId);
    virtual int32_t OnUserQueueInputBuffer(uint32_t bufferId, const AVCodecBufferInfo &info,
        AVCodecBufferFlag flag, BufferOperationMode mode);
    void SetBufferInfoFromUser(BufferInfo& bufferInfo, const AVCodecBufferInfo &info, AVCodecBufferFlag flag);
    int32_t NotifyOmxToEmptyThisInputBuffer(BufferInfo& bufferInfo);
    virtual void OnOMXEmptyBufferDone(uint32_t bufferId, BufferOperationMode mode) = 0;
    virtual int32_t OnSignalEndOfInputStream() { return AVCS_ERR_UNSUPPORT; }

    // output buffer circulation
    int32_t NotifyOmxToFillThisOutputBuffer(BufferInfo &info);
    void OnOMXFillBufferDone(const OHOS::HDI::Codec::V1_0::OmxCodecBuffer& omxBuffer, BufferOperationMode mode);
    void NotifyUserOutputBufferAvaliable(BufferInfo &bufferInfo);
    virtual std::shared_ptr<AVSharedMemoryBase> OnUserGetOutputBuffer(uint32_t bufferId);
    int32_t OnUserReleaseOutputBuffer(uint32_t bufferId, BufferOperationMode mode);
    virtual int32_t OnUserRenderOutputBuffer(uint32_t bufferId, BufferOperationMode mode) { return AVCS_ERR_UNSUPPORT; }

    // stop/release
    void ReclaimBuffer(OMX_DIRTYPE portIndex, BufferOwner owner);
    bool IsAllBufferOwnedByUsOrSurface(OMX_DIRTYPE portIndex);
    bool IsAllBufferOwnedByUsOrSurface();
    void EraseOutBuffersOwnedByUsOrSurface();
    void ClearBufferPool(OMX_DIRTYPE portIndex);
    virtual void EraseBufferFromPool(OMX_DIRTYPE portIndex, size_t i) = 0;
    void FreeOmxBuffer(OMX_DIRTYPE portIndex, const BufferInfo& info);

    // template
    template <typename T>
    static inline void InitOMXParam(T& param)
    {
        (void)memset_s(&param, sizeof(T), 0x0, sizeof(T));
        param.nSize = sizeof(T);
        param.nVersion.s.nVersionMajor = 1;
    }

    template <typename T>
    static inline void InitOMXParamExt(T& param)
    {
        (void)memset_s(&param, sizeof(T), 0x0, sizeof(T));
        param.size = sizeof(T);
        param.version.s.nVersionMajor = 1;
    }

    template <typename T>
    bool GetParameter(uint32_t index, T& param, bool isCfg = false)
    {
        int8_t* p = reinterpret_cast<int8_t*>(&param);
        std::vector<int8_t> inVec(p, p + sizeof(T));
        std::vector<int8_t> outVec;
        int32_t ret = isCfg ? compNode_->GetConfig(index, inVec, outVec) :
                              compNode_->GetParameter(index, inVec, outVec);
        if (ret != HDF_SUCCESS) {
            return false;
        }
        if (outVec.size() != sizeof(T)) {
            return false;
        }
        ret = memcpy_s(&param, sizeof(T), outVec.data(), outVec.size());
        if (ret != EOK) {
            return false;
        }
        return true;
    }

    template <typename T>
    bool SetParameter(uint32_t index, const T& param, bool isCfg = false)
    {
        const int8_t* p = reinterpret_cast<const int8_t*>(&param);
        std::vector<int8_t> inVec(p, p + sizeof(T));
        int32_t ret = isCfg ? compNode_->SetConfig(index, inVec) :
                              compNode_->SetParameter(index, inVec);
        if (ret != HDF_SUCCESS) {
            return false;
        }
        return true;
    }

    static inline uint32_t AlignTo(uint32_t side, uint32_t align)
    {
        if (align == 0) {
            return side;
        }
        return (side + align - 1) / align * align;
    }

protected:
    OMX_VIDEO_CODINGTYPE codingType_;
    bool isEncoder_;
    std::string componentName_;
    std::string ctorTime_;
    sptr<OHOS::HDI::Codec::V1_0::ICodecCallback> compCb_ = nullptr;
    sptr<OHOS::HDI::Codec::V1_0::ICodecComponent> compNode_ = nullptr;
    sptr<OHOS::HDI::Codec::V1_0::ICodecComponentManager> compMgr_ = nullptr;
    uint32_t componentId_ = 0;

    std::shared_ptr<AVCodecCallback> callback_;
    std::shared_ptr<Format> configFormat_;
    std::shared_ptr<Format> inputFormat_;
    std::shared_ptr<Format> outputFormat_;
    std::optional<PortInfo> sharedBufferFormat_;

    std::vector<BufferInfo> inputBufferPool_;
    std::vector<BufferInfo> outputBufferPool_;
    bool isBufferCirculating_ = false;
    bool inputPortEos_ = false;
    bool outputPortEos_ = false;
    uint64_t etbCnt_ = 0;
    uint64_t fbdCnt_ = 0;
    std::chrono::time_point<std::chrono::steady_clock> firstFbdTime_;

private:
    struct BaseState : State {
    protected:
        BaseState(HCodec *codec, const std::string &stateName,
                  BufferOperationMode inputMode = KEEP_BUFFER, BufferOperationMode outputMode = KEEP_BUFFER)
            : State(stateName), codec_(codec), inputMode_(inputMode), outputMode_(outputMode) {}
        void OnMsgReceived(const MsgInfo &info) override;
        void ReplyErrorCode(MsgId id, int32_t err);
        void OnCodecEvent(const MsgInfo &info);
        virtual void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2);
        void OnGetFormat(const MsgInfo &info);
        void OnUserGetInputBuffer(const MsgInfo &info);
        void OnUserQueueInputBuffer(const MsgInfo &info);
        void OnUserGetOutputBuffer(const MsgInfo &info);
        void OnUserReleaseOutputBuffer(const MsgInfo &info);
        void OnUserRenderOutputBuffer(const MsgInfo &info);
        virtual void OnShutDown(const MsgInfo &info) = 0;
        void OnStateExited() override { codec_->stateGeneration_++; }

    protected:
        HCodec *codec_;
        BufferOperationMode inputMode_;
        BufferOperationMode outputMode_;
    };

    struct UninitializedState : BaseState {
        explicit UninitializedState(HCodec *codec) : BaseState(codec, "Uninitialized") {}
    private:
        void OnStateEntered() override;
        void OnMsgReceived(const MsgInfo &info) override;
        int32_t OnAllocateComponent(const std::string &name);
        void OnShutDown(const MsgInfo &info) override;
    };

    struct InitializedState : BaseState {
        explicit InitializedState(HCodec *codec) : BaseState(codec, "Initialized") {}
    private:
        void OnStateEntered() override;
        void ProcessShutDownFromRunning();
        void OnMsgReceived(const MsgInfo &info) override;
        void OnSetCallBack(const MsgInfo &info);
        void OnConfigure(const MsgInfo &info);
        void OnSetSurface(const MsgInfo &info, bool isInput);
        void OnStart(const MsgInfo &info);
        void OnShutDown(const MsgInfo &info) override;
    };

    struct StartingState : BaseState {
        explicit StartingState(HCodec *codec) : BaseState(codec, "Starting") {}
    private:
        void OnStateEntered() override;
        void OnStateExited() override;
        void OnMsgReceived(const MsgInfo &info) override;
        int32_t AllocateBuffers();
        void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2) override;
        void OnShutDown(const MsgInfo &info) override;
        void ReplyStartMsg(int32_t errCode);
        bool hasError_ = false;
    };

    struct RunningState : BaseState {
        explicit RunningState(HCodec *codec) : BaseState(codec, "Running", RESUBMIT_BUFFER, RESUBMIT_BUFFER) {}
    private:
        void OnStateEntered() override;
        void OnMsgReceived(const MsgInfo &info) override;
        void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2) override;
        void OnShutDown(const MsgInfo &info) override;
        void OnFlush(const MsgInfo &info);
        void OnSetParameters(const MsgInfo &info);
    };

    struct OutputPortChangedState : BaseState {
        explicit OutputPortChangedState(HCodec *codec)
            : BaseState(codec, "OutputPortChanged", RESUBMIT_BUFFER, FREE_BUFFER) {}
    private:
        void OnStateEntered() override;
        void OnMsgReceived(const MsgInfo &info) override;
        void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2) override;
        void OnShutDown(const MsgInfo &info) override;
        void HandleOutputPortDisabled();
        void HandleOutputPortEnabled();
        void OnFlush(const MsgInfo &info);
        void OnForceShutDown(const MsgInfo &info);
        void OnCheckIfStuck(const MsgInfo &info);
    };

    struct FlushingState : BaseState {
        explicit FlushingState(HCodec *codec) : BaseState(codec, "Flushing") {}
    private:
        void OnStateEntered() override;
        void OnMsgReceived(const MsgInfo &info) override;
        void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2) override;
        void OnShutDown(const MsgInfo &info) override;
        void ChangeStateIfWeOwnAllBuffers();
        bool IsFlushCompleteOnAllPorts();
        int32_t UpdateFlushStatusOnPorts(uint32_t data1, uint32_t data2);
        bool flushCompleteFlag_[2] {false, false};
    };

    struct StoppingState : BaseState {
        explicit StoppingState(HCodec *codec) : BaseState(codec, "Stopping"),
            omxNodeInIdleState_(false),
            omxNodeIsChangingToLoadedState_(false) {}
    private:
        void OnStateEntered() override;
        void OnMsgReceived(const MsgInfo &info) override;
        void OnCodecEvent(OHOS::HDI::Codec::V1_0::CodecEventType event, uint32_t data1, uint32_t data2) override;
        void OnShutDown(const MsgInfo &info) override;
        void ChangeStateIfWeOwnAllBuffers();
        void ChangeOmxNodeToLoadedState(bool forceToFreeBuffer);
        bool omxNodeInIdleState_;
        bool omxNodeIsChangingToLoadedState_;
    };

    class HdiCallback : public OHOS::HDI::Codec::V1_0::ICodecCallback {
    public:
        explicit HdiCallback(HCodec* codec) : codec_(codec) { }
        virtual ~HdiCallback() = default;
        int32_t EventHandler(OHOS::HDI::Codec::V1_0::CodecEventType event,
                             const OHOS::HDI::Codec::V1_0::EventInfo& info);
        int32_t EmptyBufferDone(int64_t appData, const OHOS::HDI::Codec::V1_0::OmxCodecBuffer& buffer);
        int32_t FillBufferDone(int64_t appData, const OHOS::HDI::Codec::V1_0::OmxCodecBuffer& buffer);
    private:
        HCodec* codec_;
    };

private:
    void InitCreationTime();
    int32_t DoSyncCall(MsgWhat msgType, std::function<void(ParamSP)> oper);
    int32_t DoSyncCallAndGetReply(MsgWhat msgType, std::function<void(ParamSP)> oper, ParamSP &reply);
    int32_t InitWithName(const std::string &name);
    void ReleaseComponent();
    void CleanUpOmxNode();
    void ChangeOmxToTargetState(OHOS::HDI::Codec::V1_0::CodecStateType &state,
                                OHOS::HDI::Codec::V1_0::CodecStateType targetState);
    bool RollOmxBackToLoaded();

    int32_t ForceShutdown(int32_t generation);
    void SignalError(AVCodecErrorType errorType, int32_t errorCode);
    void DeferMessage(const MsgInfo &info);
    void ProcessDeferredMessages();
    void ReplyToSyncMsgLater(const MsgInfo& msg);
    bool GetFirstSyncMsgToReply(MsgInfo& msg);

private:
    static constexpr size_t MAX_HCODEC_BUFFER_SIZE = 8192 * 4096 * 4; // 8K RGBA
    static constexpr uint64_t THREE_SECONDS_IN_US = 3'000'000;
    static constexpr char BUFFER_ID[] = "buffer-id";
    static constexpr double FRAME_RATE_COEFFICIENT = 65536.0;

    std::shared_ptr<UninitializedState> uninitializedState_;
    std::shared_ptr<InitializedState> initializedState_;
    std::shared_ptr<StartingState> startingState_;
    std::shared_ptr<RunningState> runningState_;
    std::shared_ptr<OutputPortChangedState> outputPortChangedState_;
    std::shared_ptr<FlushingState> flushingState_;
    std::shared_ptr<StoppingState> stoppingState_;

    int32_t stateGeneration_ = 0;
    bool isShutDownFromRunning_ = false;
    bool notifyCallerAfterShutdownComplete_ = false;
    bool keepComponentAllocated_ = false;
    bool hasFatalError_ = false;
    std::list<MsgInfo> deferredQueue_;
    std::map<MsgType, std::queue<std::pair<MsgId, ParamSP>>> syncMsgToReply_;
}; // class HCodec
} // namespace OHOS::MediaAVCodec
#endif // HCODEC_HCODEC_H
