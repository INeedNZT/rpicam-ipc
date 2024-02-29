#include "output_manager.hpp"

OutputManager::OutputManager()
{
    output_ = std::make_unique<Output>();
}

OutputManager::~OutputManager()
{
}

void OutputManager::DistVideoFrame(void *mem, size_t size, int64_t timestamp_us, bool keyframe)
{
    FrameBufferPtr fb_ptr = std::make_shared<FrameBuffer>(mem, size, timestamp_us, keyframe);
    output_.get()->QueueFrame(fb_ptr);
}

void OutputManager::SetPreviewCallback(OutputCallback callback)
{
    output_.get()->SetCallback(callback);
}

void OutputManager::StartPreviewThread()
{
    output_.get()->StartOutputThread();
}