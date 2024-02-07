#pragma once

#include <atomic>
#include <cstdio>
#include <streambuf>
#include <fstream>
#include <libcamera/control_ids.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "core/logging.hpp"


class Output
{
public:
	Output();
	virtual ~Output();
	
	virtual void QueueFrame(void *mem, size_t size, int64_t timestamp_us, bool flags);
	virtual std::vector<uint8_t> GetFrameBuffer();

protected:
	struct FrameBuffer
	{
		void *mem;
		size_t size;
		int64_t timestamp_us;
		uint32_t flags;
	};

private:
	std::mutex frame_mutex_;
	std::condition_variable frame_cond_var_;
	std::queue<FrameBuffer> frame_queue_;
	virtual void outputBuffer(FrameBuffer &frame);
};
