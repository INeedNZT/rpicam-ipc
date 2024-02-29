#include "output.hpp"

Output::Output()
{
}

Output::~Output()
{
}

void Output::QueueFrame(FrameBufferPtr fb_ptr)
{
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_queue_.push(fb_ptr);
    frame_cond_var_.notify_one();
}

void Output::SetCallback(OutputCallback callback)
{
    callback_ = callback;
}

void Output::StartOutputThread()
{
    // TODO
    std::thread output_thread_(&Output::outputThread, this);
    output_thread_.detach();
}

void Output::outputThread()
{
    while (true)
    {
        FrameBufferPtr fb_ptr = nullptr;
        {
            using namespace std::chrono_literals;
            std::unique_lock<std::mutex> lock(frame_mutex_);
            if (!frame_queue_.empty())
            {
                fb_ptr = frame_queue_.front();
                frame_queue_.pop();
            }
            else
                frame_cond_var_.wait_for(lock, 200ms);
        }
        if (fb_ptr)
            callback_(fb_ptr);
    }
}