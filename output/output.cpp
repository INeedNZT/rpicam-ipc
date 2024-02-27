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

void Output::QueueFrame(void *mem, size_t size, int64_t timestamp_us, bool flags)
{
    // Queue the frame for output
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_queue_.push({mem, size, timestamp_us, flags});
    // std::cout << "QueueFrame: " << frame_queue_.size() << std::endl;
    frame_cond_var_.notify_one();
}

std::vector<uint8_t> Output::GetFrameBuffer()
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> lock(frame_mutex_);
    while (frame_queue_.empty())
    {
        std::cout << "No frames available in the queue. Try again later." << std::endl;
        frame_cond_var_.wait_for(lock, 200ms);
    }
    FrameBuffer frame = frame_queue_.front();
    frame_queue_.pop();
    bool keyframe = frame.flags;
    // 等待关键帧
    while (state_ == false)
    {
        if (keyframe)
            state_ = true;
        else
        {
            frame = frame_queue_.front();
            frame_queue_.pop();
            keyframe = frame.flags;
            continue;
        }
    }
    
    std::vector<uint8_t> buf_(frame.size);
    memcpy(&buf_[0], frame.mem, frame.size);

    return buf_;
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