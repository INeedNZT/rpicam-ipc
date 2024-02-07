
#include <chrono>

#include "video_camera.hpp"
#include "output/output_manager.hpp"

class VideoCameraCtl
{
public:
    VideoCameraCtl() : output_manager_(), vcamera_(){
                                              // VideoCameraCtl();
                                          };
    ~VideoCameraCtl(){

    };
    void Start()
    {
        while (true)
        {
            event_loop();
        }
    };
    void GetFrameBuffer(void *mem)
    {
        output_manager_.GetFrameBuffer(mem);
    };
    // Stop(){
    //     vcamera_.StopCamera();
    // };
private:
    OutputManager output_manager_;
    VideoCamera vcamera_;
    void event_loop()
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

    void GetFrameBuffer(VideoCameraCtl *vc, void *mem) {
        vc->GetFrameBuffer(mem);
    }
}