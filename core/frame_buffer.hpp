#pragma once

#include <memory>
#include <cstdlib>
#include <iostream>
#include <cstring>

struct FrameBuffer
{
    FrameBuffer(void *mem, size_t size, int64_t timestamp_us, uint32_t flags)
        : size(size), timestamp_us(timestamp_us), flags(flags)
    {
        this->mem = malloc(size);
        memcpy(this->mem, mem, size);
        // 打印mem
        std::cout << "frame buffer中的mem: " << this->mem << std::endl;
    }
    ~FrameBuffer()
    {
        std::cout << "进入析构函数\n" << std::endl;
        if (this->mem)
        {
            std::cout << "frame buffer中的mem被释放了\n" << std::endl;
            free(this->mem);
        }
    }

    void *mem;
    size_t size;
    int64_t timestamp_us;
    uint32_t flags;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;