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

    void EnableTimestamp(bool enable);

private:
    OutputManager& output_manager_;
    VideoCamera vcamera_;
    Timestamp timestamp_;
    bool enable_timestamp_;
    int64_t prev_timestamp_sec_;
    void addTimestampText(CompletedRequestPtr& completed_request);
};