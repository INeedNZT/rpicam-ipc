#include "video_camera_ctl.hpp"

VideoCameraCtl::VideoCameraCtl() : output_manager_(), vcamera_()
{
}

VideoCameraCtl::~VideoCameraCtl()
{
}

void VideoCameraCtl::Start()
{
    try
    {
        event_loop();
    }
    catch (std::exception const &e)
    {
        LOG_ERROR("ERROR: *** " << e.what() << " ***");
    }
}

void VideoCameraCtl::Stop()
{
    vcamera_.StopCamera();
}

uint8_t *VideoCameraCtl::GetFrameBuffer(int &size)
{
    auto buffer = output_manager_.GetFrameBuffer();
    size = buffer.size();
    uint8_t *data = new uint8_t[size];
    std::copy(buffer.begin(), buffer.end(), data);
    return data;
}

void VideoCameraCtl::event_loop()
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
        vcamera_.EncodeBuffer(completed_request);
    }
}