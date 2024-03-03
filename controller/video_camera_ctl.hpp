#include "core/video_camera.hpp"
#include "output/output_manager.hpp"
#include "text/timestamp.hpp"

class VideoCameraCtl
{
public:
    VideoCameraCtl(OutputManager& output_manager);
    ~VideoCameraCtl();

    void StartCamera();
    void StopCamera();

    void StartPreview();
    void StopPreview();

private:
    OutputManager& output_manager_;
    VideoCamera vcamera_;
};