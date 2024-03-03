#include "video_camera_ctl.hpp"

VideoCameraCtl::VideoCameraCtl(OutputManager& output_manager) : output_manager_(output_manager), vcamera_()
{
    std::cout << "VideoCameraCtl constructor" << std::endl;
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
            // Add timestamp to the frame before encoding
            
            auto ts = completed_request.get()->metadata.get(libcamera::controls::SensorTimestamp).value();
            char format[] = "%Y-%m-%d %H:%M:%S";
            Timestamp timestamp(format);
            timestamp.SetFontPath(std::string("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf").c_str());
            timestamp.SetTimestamp(ts);
            // timestamp.SetPosition(10, 10);
            libcamera::Stream *stream = vcamera_.GetVideoStream();
            libcamera::FrameBuffer *buffer = completed_request->buffers[stream];
            BufferWriteSync r(&vcamera_, buffer);
            libcamera::Span span = r.Get()[0];
            void *mem = span.data();
            timestamp.Draw2Canvas(reinterpret_cast<uint8_t*>(mem), 1920, 1080);
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