#pragma once

#include <functional>
#include <unistd.h>

#include "core/stream_info.hpp"

typedef std::function<void(void *)> InputDoneCallback;
typedef std::function<void(void *, size_t, int64_t, bool)> OutputReadyCallback;

class Encoder
{
public:
	static Encoder *Create(StreamInfo const &info);

	Encoder() {}
	virtual ~Encoder() {}
	// This is where the application sets the callback it gets whenever the encoder
	// has finished with an input buffer, so the application can re-use it.
	void SetInputDoneCallback(InputDoneCallback callback) { input_done_callback_ = callback; }
	// This callback is how the application is told that an encoded buffer is
	// available. The application may not hang on to the memory once it returns
	// (but the callback is already running in its own thread).
	void SetOutputReadyCallback(OutputReadyCallback callback) { output_ready_callback_ = callback; }
	// Encode the given buffer. The buffer is specified both by an fd and size
	// describing a DMABUF, and by a mmapped userland pointer.
	virtual void EncodeBuffer(int fd, size_t size, void *mem, StreamInfo const &info, int64_t timestamp_us) = 0;

protected:
	InputDoneCallback input_done_callback_;
	OutputReadyCallback output_ready_callback_;
};
