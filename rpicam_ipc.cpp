#include "controller/video_camera_ctl.cpp"
#include "controller/web_ctl.hpp"

int main(int argc, char *argv[])
{
	OutputManager om;
	MongooseManager mgr;

	VideoCameraCtl vc(om);
	WebCtl wc(mgr, ".", "ws://localhost:8000");

	om.SetPreviewCallback([&mgr](FrameBufferPtr &fb_ptr)
						  {
		struct mg_connection *c;
		for (c = mgr.conns; c != NULL; c = c->next)
		{
			if (c->data[0] != 'W')
				continue;
			// Dynamically allocate the wrapper to controlle the life time of the shared ptr (fb_ptr)
			auto* fb_ptr_wrapper = new FrameBufferPtrWrapper { fb_ptr };
			mg_wakeup(&mgr, c->id, &fb_ptr_wrapper, sizeof(*fb_ptr_wrapper));
		} });

	std::thread video_camera_thread(std::bind(&VideoCameraCtl::StartCamera, &vc));
	std::thread web_thread(std::bind(&WebCtl::Start, &wc));

	vc.StartPreview();

	video_camera_thread.join();
	web_thread.join();

	return 0;
}
