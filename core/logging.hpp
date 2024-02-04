#include "camera.hpp"

#define LOG(level, text)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (Camera::GetVerbosity() >= level)                                                                     \
			std::cerr << text << std::endl;                                                                            \
	} while (0)
#define LOG_ERROR(text) std::cerr << text << std::endl
