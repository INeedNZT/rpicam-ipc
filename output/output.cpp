#include <cinttypes>
#include <stdexcept>
#include <thread>
#include <cstring>

#include "output.hpp"

extern bool bcm2835_encoder_available();

Output::Output()
{
}

Output::~Output()
{
}

void Output::QueueFrame(void *mem, size_t size, int64_t timestamp_us, uint32_t flags)
{
    // Queue the frame for output
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_queue_.push({mem, size, timestamp_us, flags});
}

void Output::GetFrameBuffer(void *mem)
{
    // Get the frame buffer
    std::lock_guard<std::mutex> lock(frame_mutex_);
    if (frame_queue_.empty())
    {
        throw std::runtime_error("No frames available in the queue.");
    }
    FrameBuffer frame = frame_queue_.front();
    frame_queue_.pop();
    memcpy(mem, frame.mem, frame.size);
}


void Output::outputBuffer(FrameBuffer &frame)
{
    // Output the buffer
    // if (frame.flags & FRAME_FLAG_KEYFRAME)
    // {
    //     LOG(1, "Output::outputBuffer() - Keyframe");
    // }
    // else
    // {
    //     LOG(1, "Output::outputBuffer() - Non-Keyframe");
    // }
}
// void OutputThread()
// {
//     LOG(1, "Output::outputThread()");
//     // Output thread
//     while (true)
//     {
//         std::unique_lock<std::mutex> lock(frame_mutex_);
//         if (frame_queue_.empty())
//         {
//             lock.unlock();
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             continue;
//         }
//         FrameBuffer frame = queue_.front();
//         outputBuffer(frame);
//         queue_.pop();
//         lock.unlock();
//         outputBuffer(frame);
//     }
// }