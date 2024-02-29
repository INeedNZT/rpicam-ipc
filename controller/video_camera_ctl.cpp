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