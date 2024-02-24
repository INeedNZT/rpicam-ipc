#include <chrono>

#include "core/video_camera.hpp"
#include "output/output_manager.hpp"


class VideoCameraCtl
{
public:
    VideoCameraCtl();
    ~VideoCameraCtl();

    void Start();
    void Stop();

    uint8_t* GetFrameBuffer(int &size);
  
private:
    OutputManager output_manager_;
    VideoCamera vcamera_;

    void event_loop();
};


extern "C"
{
    VideoCameraCtl *Create()
    {
        return new VideoCameraCtl();
    }

    void Start(VideoCameraCtl * vc)
    {
        vc->Start();
    }

    uint8_t* GetFrameBuffer(VideoCameraCtl *vc, int &size) {
        return vc->GetFrameBuffer(size);
    }
}