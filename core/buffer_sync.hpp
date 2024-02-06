#pragma once

#include <libcamera/framebuffer.h>

class VideoCamera;

class BufferWriteSync
{
public:
	BufferWriteSync(VideoCamera *vcamera, libcamera::FrameBuffer *fb);
	~BufferWriteSync();

	const std::vector<libcamera::Span<uint8_t>> &Get() const;

private:
	libcamera::FrameBuffer *fb_;
	std::vector<libcamera::Span<uint8_t>> planes_;
};

class BufferReadSync
{
public:
	BufferReadSync(VideoCamera *vcamera, libcamera::FrameBuffer *fb);
	~BufferReadSync();

	const std::vector<libcamera::Span<uint8_t>> &Get() const;

private:
	std::vector<libcamera::Span<uint8_t>> planes_;
};
