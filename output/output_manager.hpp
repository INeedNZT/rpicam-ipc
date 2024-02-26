#pragma once

#include <thread>

#include "output.hpp"
#include "core/logging.hpp"

#include <memory>

class OutputManager
{
public:
    OutputManager();
    ~OutputManager();

    void DistVideoFrame(void *mem, size_t size, int64_t timestamp_us, bool flags);
    std::vector<uint8_t> GetFrameBuffer();

private:
    std::unique_ptr<Output> output_;
    enum State
	{
		DISABLED = 0,
		WAITING_KEYFRAME = 1,
		RUNNING = 2
	};
    State state_ = WAITING_KEYFRAME;
};
