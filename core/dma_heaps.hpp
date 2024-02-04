#pragma once

#include <stddef.h>

#include <libcamera/base/unique_fd.h>

class DmaHeap
{
public:
	DmaHeap();
	~DmaHeap();
	bool isValid() const { return dmaHeapHandle_.isValid(); }
	libcamera::UniqueFD alloc(const char *name, std::size_t size) const;

private:
	libcamera::UniqueFD dmaHeapHandle_;
};
