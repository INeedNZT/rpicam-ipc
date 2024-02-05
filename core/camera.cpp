#include "camera.hpp"
#include "logging.hpp"


#include <sys/stat.h>
#include <sys/ioctl.h>

Camera::Camera() {
    // ...  
}

Camera::~Camera() {
    LOG(2, "Closing RPiCam application");
	StopCamera();
	// Teardown();
	CloseCamera();
}

unsigned int Camera::verbosity = 1;

enum class Platform
{
	MISSING,
	UNKNOWN,
	LEGACY,
	VC4,
	PISP,
};

libcamera::Stream *Camera::GetVideoStream(StreamInfo *info) const
{
	auto it = streams_.find("video");
	if (it == streams_.end())
		return nullptr;
	if (info)
	{
		libcamera::StreamConfiguration const &cfg = it->second->configuration();
		StreamInfo sinfo;
		sinfo.width = cfg.size.width;
		sinfo.height = cfg.size.height;
		sinfo.stride = cfg.stride;
		sinfo.pixel_format = cfg.pixelFormat;
		sinfo.colour_space = cfg.colorSpace;
		*info = sinfo;
	}
	return it->second;
}

StreamInfo Camera::GetStreamInfo(libcamera::Stream *stream) const
{
	StreamConfiguration const &cfg = stream->configuration();
	StreamInfo info;
	info.width = cfg.size.width;
	info.height = cfg.size.height;
	info.stride = cfg.stride;
	info.pixel_format = cfg.pixelFormat;
	info.colour_space = cfg.colorSpace;
	return info;
}

void Camera::OpenCamera()
{
    LOG(2, "Opening camera...");

    if (!camera_manager_)
    {
        // Create a new camera manager instance if it doesn't exist.
        camera_manager_.reset();
        camera_manager_ = std::make_unique<libcamera::CameraManager>();
        int ret = camera_manager_->start();
        if (ret)
            throw std::runtime_error("camera manager failed to start, code " + std::to_string(-ret));
    }

    std::vector<std::shared_ptr<libcamera::Camera>> cameras = GetCameras(camera_manager_.get());
    if (cameras.size() == 0)
		throw std::runtime_error("no cameras available");
     if (cameras.size() > 1)
		throw std::runtime_error("only one camera supported");
    
    std::string const &cam_id = cameras[0]->id();
    camera_ = camera_manager_->get(cam_id);
    if (!camera_)
		throw std::runtime_error("failed to find camera " + cam_id);
    if (camera_->acquire())
		throw std::runtime_error("failed to acquire camera " + cam_id);
	camera_acquired_ = true;

	LOG(2, "Acquired camera " << cam_id);

    // We're going to make a list of all the available sensor modes, but we only populate
	// the framerate field if the user has requested a framerate (as this requires us actually
	// to configure the sensor, which is otherwise best avoided).

	std::unique_ptr<libcamera::CameraConfiguration> config = camera_->generateConfiguration({ libcamera::StreamRole::VideoRecording });
	const libcamera::StreamFormats &formats = config->at(0).formats();
    for (const auto &pix : formats.pixelformats())
	{
		for (const auto &size : formats.sizes(pix))
		{
			double framerate = 0;
            // TODO: user configures framerate
			sensor_modes_.emplace_back(size, pix, framerate);
		}
	}
}

void Camera::CloseCamera()
{
    if (camera_acquired_)
    {
        camera_->release();
        camera_acquired_ = false;
    }

    camera_.reset();
    camera_manager_.reset();

    LOG(2, "Camera closed");
}

void Camera::ConfigureCamera()
{
    LOG(2, "Configuring video...");

    std::vector<libcamera::StreamRole> stream_roles = { libcamera::StreamRole::VideoRecording };
    configuration_ = camera_->generateConfiguration(stream_roles);
    if (!configuration_)
		throw std::runtime_error("failed to generate video configuration");

    // Now we get to override any of the default settings from user's settings.
	libcamera::StreamConfiguration &cfg = configuration_->at(0);
	cfg.pixelFormat = libcamera::formats::YUV420;
    cfg.bufferCount = 6; // 6 buffers is better than 4

    if (cfg.size.width >= 1280 || cfg.size.height >= 720)
		cfg.colorSpace = libcamera::ColorSpace::Rec709;
	else
		cfg.colorSpace = libcamera::ColorSpace::Smpte170m;
    
    //TODO: Not set Denoise for now

    setupCapture();

    LOG(2, "Video setup complete");
}

void Camera::StartCamera()
{
    // This makes all the Request objects that we shall need.
	makeRequests();

	// Framerate is a bit weird. If it was set programmatically, we go with that, but
	// otherwise it applies only to preview/video modes. For stills capture we set it
	// as long as possible so that we get whatever the exposure profile wants.
    if (!controls_.get(libcamera::controls::FrameDurationLimits))
    {
        int64_t frame_time = 1000000 / 30; // in us
        controls_.set(libcamera::controls::FrameDurationLimits,
                      libcamera::Span<const int64_t, 2>({frame_time, frame_time}));
    }

	if (!controls_.get(libcamera::controls::AeMeteringMode))
		controls_.set(libcamera::controls::AeMeteringMode, 0);
	if (!controls_.get(libcamera::controls::AeExposureMode))
		controls_.set(libcamera::controls::AeExposureMode, 0);
	if (!controls_.get(libcamera::controls::ExposureValue))
		controls_.set(libcamera::controls::ExposureValue, 0);
	if (!controls_.get(libcamera::controls::AwbMode))
		controls_.set(libcamera::controls::AwbMode, 0);
	if (!controls_.get(libcamera::controls::Brightness))
		controls_.set(libcamera::controls::Brightness, 0);
	if (!controls_.get(libcamera::controls::Contrast))
		controls_.set(libcamera::controls::Contrast, 1);
	if (!controls_.get(libcamera::controls::Saturation))
		controls_.set(libcamera::controls::Saturation, 1);
	if (!controls_.get(libcamera::controls::Sharpness))
		controls_.set(libcamera::controls::Sharpness, 1);

	// AF Controls, where supported and not already set
	if (!controls_.get(libcamera::controls::AfMode) && camera_->controls().count(&libcamera::controls::AfMode) > 0)
	{
		int afm = camera_->controls().at(&libcamera::controls::AfMode).max().get<int>();
		controls_.set(libcamera::controls::AfMode, afm);
	}
	if (!controls_.get(libcamera::controls::AfRange) && camera_->controls().count(&libcamera::controls::AfRange) > 0)
		controls_.set(libcamera::controls::AfRange, 0);
	if (!controls_.get(libcamera::controls::AfSpeed) && camera_->controls().count(&libcamera::controls::AfSpeed) > 0)
		controls_.set(libcamera::controls::AfSpeed, 0);

	if (camera_->start(&controls_))
		throw std::runtime_error("failed to start camera");
	controls_.clear();
	camera_started_ = true;
	last_timestamp_ = 0;

	camera_->requestCompleted.connect(this, &Camera::requestComplete);

	for (std::unique_ptr<libcamera::Request> &request : requests_)
	{
		if (camera_->queueRequest(request.get()) < 0)
			throw std::runtime_error("Failed to queue request");
	}

	LOG(2, "Camera started!");
}

void Camera::StopCamera()
{
	{
		// We don't want QueueRequest to run asynchronously while we stop the camera.
		std::lock_guard<std::mutex> lock(camera_stop_mutex_);
		if (camera_started_)
		{
			if (camera_->stop())
				throw std::runtime_error("failed to stop camera");

			camera_started_ = false;
		}
	}

	if (camera_)
		camera_->requestCompleted.disconnect(this, &Camera::requestComplete);

	// An application might be holding a CompletedRequest, so queueRequest will get
	// called to delete it later, but we need to know not to try and re-queue it.
	completed_requests_.clear();

	msg_queue_.Clear();

	requests_.clear();

	controls_.clear(); // no need for mutex here

	LOG(2, "Camera stopped!");
}

void Camera::StartEncoder()
{
	StreamInfo info;
	
	GetVideoStream(&info);
	if (!info.width || !info.height || !info.stride)
		throw std::runtime_error("video steam is not configured");
	encoder_ = std::unique_ptr<Encoder>(Encoder::Create(info));
}

void Camera::StopEncoder()
{
	encoder_.reset();
}

void Camera::EncodeBuffer(CompletedRequestPtr &completed_request)
{
	assert(encoder_);
	libcamera::Stream stream = GetVideoStream();
	StreamInfo info = GetStreamInfo(stream);
	libcamera::FrameBuffer *buffer = completed_request->buffers[stream];
	BufferReadSync r(this, buffer);
	libcamera::Span span = r.Get()[0];
	void *mem = span.data();
	if (!buffer || !mem)
		throw std::runtime_error("no buffer to encode");
	auto ts = completed_request->metadata.get(libcamera::controls::SensorTimestamp);
	int64_t timestamp_ns = ts ? *ts : buffer->metadata().timestamp;
	{
		std::lock_guard<std::mutex> lock(encode_buffer_queue_mutex_);
		encode_buffer_queue_.push(completed_request); // creates a new reference
	}
	encoder_->EncodeBuffer(buffer->planes()[0].fd.get(), span.size(), mem, info, timestamp_ns / 1000);
}

Camera::Msg Camera::Wait()
{
	return msg_queue_.Wait();
}

void Camera::setupCapture()
{
    // First finish setting up the configuration.
	for (auto &config : *configuration_)
		config.stride = 0;
	libcamera::CameraConfiguration::Status validation = configuration_->validate();
	if (validation == libcamera::CameraConfiguration::Invalid)
		throw std::runtime_error("failed to valid stream configurations");
	else if (validation == libcamera::CameraConfiguration::Adjusted)
		LOG(1, "Stream configuration adjusted");

	if (camera_->configure(configuration_.get()) < 0)
		throw std::runtime_error("failed to configure streams");
	LOG(2, "Camera streams configured");

    LOG(2, "Available controls:");
	for (auto const &[id, info] : camera_->controls())
		LOG(2, "    " << id->name() << " : " << info.toString());
    
    // Next allocate all the buffers we need, mmap them and store them on a free list.

	for (libcamera::StreamConfiguration &config : *configuration_)
	{
		libcamera::Stream *stream = config.stream();
		std::vector<std::unique_ptr<libcamera::FrameBuffer>> fb;

		for (unsigned int i = 0; i < config.bufferCount; i++)
		{
			std::string name("rpicam-ipc" + std::to_string(i));
			libcamera::UniqueFD fd = dma_heap_.alloc(name.c_str(), config.frameSize);

			if (!fd.isValid())
				throw std::runtime_error("failed to allocate capture buffers for stream");

			std::vector<libcamera::FrameBuffer::Plane> plane(1);
			plane[0].fd = libcamera::SharedFD(std::move(fd));
			plane[0].offset = 0;
			plane[0].length = config.frameSize;

			fb.push_back(std::make_unique<libcamera::FrameBuffer>(plane));
			void *memory = mmap(NULL, config.frameSize, PROT_READ | PROT_WRITE, MAP_SHARED, plane[0].fd.get(), 0);
			mapped_buffers_[fb.back().get()].push_back(
						libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory), config.frameSize));
		}

		frame_buffers_[stream] = std::move(fb);
	}
	LOG(2, "Buffers allocated and mapped");

    // The requests will be made when StartCamera() is called.
}

void Camera::makeRequests()
{
    std::map<libcamera::Stream *, std::queue<libcamera::FrameBuffer *>> free_buffers;

    for (auto &kv : frame_buffers_)
	{
		free_buffers[kv.first] = {};
		for (auto &b : kv.second)
			free_buffers[kv.first].push(b.get());
	}

    while (true)
	{
		for (libcamera::StreamConfiguration &config : *configuration_)
		{
			libcamera::Stream *stream = config.stream();
			if (stream == configuration_->at(0).stream())
			{
				if (free_buffers[stream].empty())
				{
					LOG(2, "Requests created");
					return;
				}
				std::unique_ptr<libcamera::Request> request = camera_->createRequest();
				if (!request)
					throw std::runtime_error("failed to make request");
				requests_.push_back(std::move(request));
			}
			else if (free_buffers[stream].empty())
				throw std::runtime_error("concurrent streams need matching numbers of buffers");

			libcamera::FrameBuffer *buffer = free_buffers[stream].front();
			free_buffers[stream].pop();
			if (requests_.back()->addBuffer(stream, buffer) < 0)
				throw std::runtime_error("failed to add buffer to request");
		}
	}
}

void Camera::requestComplete(libcamera::Request *request)
{
    if (request->status() == libcamera::Request::RequestCancelled)
	{
		// If the request is cancelled while the camera is still running, it indicates
		// a hardware timeout. Let the application handle this error.
		if (camera_started_)
			msg_queue_.Post(Msg(MsgType::Timeout));

		return;
	}

	struct dma_buf_sync dma_sync {};
	dma_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ;
	for (auto const &buffer_map : request->buffers())
	{
		auto it = mapped_buffers_.find(buffer_map.second);
		if (it == mapped_buffers_.end())
			throw std::runtime_error("failed to identify request complete buffer");

		int ret = ::ioctl(buffer_map.second->planes()[0].fd.get(), DMA_BUF_IOCTL_SYNC, &dma_sync);
		if (ret)
			throw std::runtime_error("failed to sync dma buf on request complete");
	}

	CompletedRequest *r = new CompletedRequest(sequence_++, request);
	CompletedRequestPtr payload(r, [this](CompletedRequest *cr) { this->queueRequest(cr); });
	{
		std::lock_guard<std::mutex> lock(completed_requests_mutex_);
		completed_requests_.insert(r);
	}

	// We calculate the instantaneous framerate in case anyone wants it.
	// Use the sensor timestamp if possible as it ought to be less glitchy than
	// the buffer timestamps.
	auto ts = payload->metadata.get(libcamera::controls::SensorTimestamp);
	uint64_t timestamp = ts ? *ts : payload->buffers.begin()->second->metadata().timestamp;
	if (last_timestamp_ == 0 || last_timestamp_ == timestamp)
		payload->framerate = 0;
	else
		payload->framerate = 1e9 / (timestamp - last_timestamp_);
	last_timestamp_ = timestamp;

	// Post the completed request to the queue for encoding.
	msg_queue_.Post(Msg(MsgType::RequestComplete, std::move(payload)));
}

void Camera::queueRequest(CompletedRequest *completed_request)
{
    libcamera::Request::BufferMap buffers(std::move(completed_request->buffers));

    // This function may run asynchronously so needs protection from the
    // camera stopping at the same time.
    std::lock_guard<std::mutex> stop_lock(camera_stop_mutex_);

    // An application could be holding a CompletedRequest while it stops and re-starts
    // the camera, after which we don't want to queue another request now.
    bool request_found;
    {
        std::lock_guard<std::mutex> lock(completed_requests_mutex_);
        auto it = completed_requests_.find(completed_request);
        if (it != completed_requests_.end())
        {
            request_found = true;
            completed_requests_.erase(it);
        }
        else
            request_found = false;
    }

    libcamera::Request *request = completed_request->request;
    delete completed_request;
    assert(request);

    if (!camera_started_ || !request_found)
        return;

    for (auto const &p : buffers)
    {
        struct dma_buf_sync dma_sync
        {
        };
        dma_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;

        auto it = mapped_buffers_.find(p.second);
        if (it == mapped_buffers_.end())
            throw std::runtime_error("failed to identify queue request buffer");

        int ret = ::ioctl(p.second->planes()[0].fd.get(), DMA_BUF_IOCTL_SYNC, &dma_sync);
        if (ret)
            throw std::runtime_error("failed to sync dma buf on queue request");

        if (request->addBuffer(p.first, p.second) < 0)
            throw std::runtime_error("failed to add buffer to request in QueueRequest");
    }

    {
        std::lock_guard<std::mutex> lock(control_mutex_);
        request->controls() = std::move(controls_);
    }

    if (camera_->queueRequest(request) < 0)
        throw std::runtime_error("failed to queue request");
}

static void set_pipeline_configuration(Platform platform)
{
	// Respect any pre-existing value in the environment variable.
	char const *existing_config = getenv("LIBCAMERA_RPI_CONFIG_FILE");
	if (existing_config && existing_config[0])
		return;

	// Otherwise point it at whichever of these we find first (if any) for the given platform.
	static const std::vector<std::pair<Platform, std::string>> config_files = {
		{ Platform::VC4, "/usr/local/share/libcamera/pipeline/rpi/vc4/rpi_apps.yaml" },
		{ Platform::VC4, "/usr/share/libcamera/pipeline/rpi/vc4/rpi_apps.yaml" },
	};

	for (auto &config_file : config_files)
	{
		struct stat info;
		if (config_file.first == platform && stat(config_file.second.c_str(), &info) == 0)
		{
			setenv("LIBCAMERA_RPI_CONFIG_FILE", config_file.second.c_str(), 1);
			break;
		}
	}
}

