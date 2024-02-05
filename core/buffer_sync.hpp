#pragma once

#include <libcamera/framebuffer.h>

class Camera;

class BufferWriteSync
{
public:
	BufferWriteSync(Camera *camera, libcamera::FrameBuffer *fb);
	~BufferWriteSync();

	const std::vector<libcamera::Span<uint8_t>> &Get() const;

private:
	libcamera::FrameBuffer *fb_;
	std::vector<libcamera::Span<uint8_t>> planes_;
};

class BufferReadSync
{
public:
	BufferReadSync(Camera *app, libcamera::FrameBuffer *fb);
	~BufferReadSync();

	const std::vector<libcamera::Span<uint8_t>> &Get() const;

private:
	std::vector<libcamera::Span<uint8_t>> planes_;
};
