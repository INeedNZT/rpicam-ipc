#include "video_camera.hpp"

#define LOG(level, text)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (VideoCamera::GetVerbosity() >= level)                                                                     \
			std::cerr << text << std::endl;                                                                            \
	} while (0)
#define LOG_ERROR(text) std::cerr << text << std::endl
