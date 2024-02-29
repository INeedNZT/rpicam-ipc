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
    void SetPreviewCallback(OutputCallback callback);
    // void SetRemoteCallback();
    // void SetFileCallback();
    void StartPreviewThread();

private:
    std::unique_ptr<Output> output_;
    // enum State
	// {
	// 	DISABLED = 0,
	// 	WAITING_KEYFRAME = 1,
	// 	RUNNING = 2
	// };
    // State state_ = WAITING_KEYFRAME;
};
