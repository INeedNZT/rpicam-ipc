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
    void GetFrameBuffer(void *mem);
private:
    std::unique_ptr<Output> output_;
};
