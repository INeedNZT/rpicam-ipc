#pragma once

#include <memory>

#include <libcamera/controls.h>
#include <libcamera/request.h>

#include "metadata.hpp"

struct CompletedRequest
{
	CompletedRequest(unsigned int seq, libcamera::Request *r)
		: sequence(seq), buffers(r->buffers()), metadata(r->metadata()), request(r)
	{
		r->reuse();
	}
	unsigned int sequence;
	libcamera::Request::BufferMap buffers;
	libcamera::ControlList metadata;
	libcamera::Request *request;
	float framerate;
};

using CompletedRequestPtr = std::shared_ptr<CompletedRequest>;
