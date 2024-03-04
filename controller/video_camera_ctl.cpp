#include "video_camera_ctl.hpp"

VideoCameraCtl::VideoCameraCtl(OutputManager &output_manager) : output_manager_(output_manager), vcamera_(), timestamp_("%Y-%m-%d %H:%M:%S")
{
    std::cout << "VideoCameraCtl constructor" << std::endl;
    timestamp_.SetFontPath("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
    timestamp_.SetFontSize(24);
    timestamp_.SetPosition(15, 15);
    enable_timestamp_ = true;
}

VideoCameraCtl::~VideoCameraCtl()
{
    std::cout << "VideoCameraCtl destructor" << std::endl;
}

void VideoCameraCtl::StartCamera()
{
    try
    {
        using namespace std::placeholders;

        vcamera_.OpenCamera();
        vcamera_.ConfigureVideo();
        vcamera_.StartEncoder();
        vcamera_.SetEncodeOutputReadyCallback(std::bind(&OutputManager::DistVideoFrame, &output_manager_, _1, _2, _3, _4));
        vcamera_.StartCamera();

        for (unsigned int count = 0;; count++)
        {
            VideoCamera::Msg msg = vcamera_.Wait();
            if (msg.type == VideoCamera::MsgType::Timeout)
            {
                LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
                vcamera_.StopCamera();
                vcamera_.StartCamera();
                continue;
            }
            if (msg.type == VideoCamera::MsgType::Quit)
                return;
            else if (msg.type != VideoCamera::MsgType::RequestComplete)
                throw std::runtime_error("unrecognised message!");

            CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);

            if (enable_timestamp_)
            {
                addTimestampText(completed_request);
            }
            auto framerate = completed_request->framerate;
            std::cout << "framerate: " << framerate << std::endl;

            vcamera_.EncodeBuffer(completed_request);
        }
    }
    catch (std::exception const &e)
    {
        LOG_ERROR("ERROR: *** " << e.what() << " ***");
    }
}

void VideoCameraCtl::StopCamera()
{
    vcamera_.StopCamera();
}

void VideoCameraCtl::StartPreview()
{
    output_manager_.StartPreviewThread();
}

void VideoCameraCtl::StopPreview()
{
    // output_manager_.StopPreviewThread();
}

void VideoCameraCtl::EnableTimestamp(bool enable)
{
    enable_timestamp_ = enable;
}

void VideoCameraCtl::addTimestampText(CompletedRequestPtr &completed_request)
{
    // Get current time in seconds
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    int64_t timestamp_sec = duration.count();
    if (timestamp_sec != prev_timestamp_sec_)
    {
        timestamp_.SetTimestamp(timestamp_sec);
        prev_timestamp_sec_ = timestamp_sec;
    }

    // Add timestamp to the frame before encoding
    libcamera::Stream *stream = vcamera_.GetVideoStream();
    libcamera::FrameBuffer *buffer = completed_request->buffers[stream];
    BufferWriteSync r(&vcamera_, buffer);
    libcamera::Span span = r.Get()[0];
    void *mem = span.data();
    timestamp_.Draw2Canvas(reinterpret_cast<uint8_t *>(mem), 1920, 1080);
}