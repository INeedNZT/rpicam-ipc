#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "core/frame_buffer.hpp"
#include "core/logging.hpp"

using OutputCallback = std::function<void(FrameBufferPtr &)>;

class Output
{
public:
	Output();
	virtual ~Output();
	virtual void QueueFrame(FrameBufferPtr fb_ptr);
	virtual void SetCallback(OutputCallback callback);
	virtual void StartOutputThread();

private:
	std::mutex frame_mutex_;
	std::condition_variable frame_cond_var_;
	std::queue<FrameBufferPtr> frame_queue_;
	virtual void outputThread();
	OutputCallback callback_;
};