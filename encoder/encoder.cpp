#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <linux/videodev2.h>

#include "encoder.hpp"
#include "h264_encoder.hpp"

bool bcm2835_encoder_available()
{
    const char hw_codec[] = "/dev/video11";
    struct v4l2_capability caps;
    memset(&caps, 0, sizeof(caps));
    int fd = open(hw_codec, O_RDWR, 0);
    if (fd)
    {
        int ret = ioctl(fd, VIDIOC_QUERYCAP, &caps);
        close(fd);
        if (!ret && !strncmp((char *)caps.card, "bcm2835-codec-encode", sizeof(caps.card)))
            return true;
    }
    throw std::runtime_error("Unable to find bcm2835 H.264 codec");
}

// Only support H.264 for now
Encoder *Encoder::Create(const StreamInfo &info)
{
    if (bcm2835_encoder_available())
        return new H264Encoder(info);

    throw std::runtime_error("Unable to find an appropriate H.264 codec");
}
