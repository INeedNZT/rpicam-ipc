#pragma once

#include <sys/mman.h>

#include <condition_variable>
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <iostream>
#include <queue>
#include <sstream>
#include <mutex>
#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/formats.h>

#include <linux/dma-buf.h>

#include "dma_heaps.hpp"
#include "completed_request.hpp"
#include "encoder/encoder.hpp"

class Camera {
public:
    Camera();
    ~Camera();

    static unsigned int verbosity;
	static unsigned int GetVerbosity() { return verbosity; };

    std::string GetCameraModel() const;
    libcamera::Stream *GetStream(StreamInfo *info = nullptr) const;
    
    void OpenCamera();
    void CloseCamera();
    void ConfigureCamera();

    void StartCamera();
    void StopCamera();

    void StartEncoder();
    void StopEncoder();

    enum class MsgType
	{
		RequestComplete,
		Timeout,
		Quit
	};
	typedef std::variant<CompletedRequestPtr> MsgPayload;
	struct Msg
	{
		Msg(MsgType const &t) : type(t) {}
		template <typename T>
		Msg(MsgType const &t, T p) : type(t), payload(std::forward<T>(p))
		{
		}
		MsgType type;
		MsgPayload payload;
	};
    struct SensorMode
	{
		SensorMode()
			: size({}), format({}), fps(0)
		{
		}
		SensorMode(libcamera::Size _size, libcamera::PixelFormat _format, double _fps)
			: size(_size), format(_format), fps(_fps)
		{
		}
		unsigned int depth() const
		{
			// This is a really ugly way of getting the bit depth of the format.
			// But apart from duplicating the massive bayer format table, there is
			// no other way to determine this.
			std::string fmt = format.toString();
			unsigned int mode_depth = fmt.find("8") != std::string::npos ? 8 :
									  fmt.find("10") != std::string::npos ? 10 :
									  fmt.find("12") != std::string::npos ? 12 : 16;
			return mode_depth;
		}
		libcamera::Size size;
		libcamera::PixelFormat format;
		double fps;
		std::string ToString() const
		{
			std::stringstream ss;
			ss << format.toString() << "," << size.toString() << "/" << fps;
			return ss.str();
		}
	};

    Msg Wait();

    static std::vector<std::shared_ptr<libcamera::Camera>> GetCameras(const libcamera::CameraManager *cm)
    {
        std::vector<std::shared_ptr<libcamera::Camera>> cameras = cm->cameras();
        // Do not show USB webcams as these are not supported in rpicam-apps!
        auto rem = std::remove_if(cameras.begin(), cameras.end(),
                                  [](auto &cam)
                                  { return cam->id().find("/usb") != std::string::npos; });
        cameras.erase(rem, cameras.end());
        std::sort(cameras.begin(), cameras.end(), [](auto l, auto r)
                  { return l->id() > r->id(); });
        return cameras;
    };

private:
    template <typename T>
    class MessageQueue
    {
    public:
        template <typename U>
        void Post(U &&msg)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::forward<U>(msg));
            cond_.notify_one();
        }
        T Wait()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this]
                       { return !queue_.empty(); });
            T msg = std::move(queue_.front());
            queue_.pop();
            return msg;
        }
        void Clear()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_ = {};
        }

    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };

    bool camera_acquired_ = false;
    bool camera_started_ = false;
    uint64_t last_timestamp_;
    uint64_t sequence_ = 0;
    
    std::unique_ptr<Encoder> encoder_;
    std::shared_ptr<libcamera::Camera> camera_;
    std::unique_ptr<libcamera::CameraManager> camera_manager_;
    std::vector<SensorMode> sensor_modes_;
    std::unique_ptr<libcamera::CameraConfiguration> configuration_;
    std::map<libcamera::FrameBuffer *, std::vector<libcamera::Span<uint8_t>>> mapped_buffers_;
    std::map<libcamera::Stream *, std::vector<std::unique_ptr<libcamera::FrameBuffer>>> frame_buffers_;
    std::vector<std::unique_ptr<libcamera::Request>> requests_;
    libcamera::ControlList controls_;
    std::mutex control_mutex_;
    std::map<std::string, libcamera::Stream *> streams_;
    DmaHeap dma_heap_;
    MessageQueue<Msg> msg_queue_;
    std::mutex camera_stop_mutex_;
    
    std::set<CompletedRequest *> completed_requests_;
    std::mutex completed_requests_mutex_;

    void setupCapture();
    void makeRequests();
    void requestComplete(libcamera::Request *request);
    void queueRequest(CompletedRequest *completed_request);
};

