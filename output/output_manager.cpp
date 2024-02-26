#include "output_manager.hpp"

OutputManager::OutputManager()
{
    output_ = std::make_unique<Output>();
}

OutputManager::~OutputManager()
{
    if (output_)
        output_.reset();
} 

void OutputManager::DistVideoFrame(void *mem, size_t size, int64_t timestamp_us, bool keyframe)
{
    output_.get()->QueueFrame(mem, size, timestamp_us, keyframe);
}

std::vector<uint8_t> OutputManager::GetFrameBuffer()
{
    return output_.get()->GetFrameBuffer();
}