#include <chrono>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/stat.h>

#include "controller/video_camera_ctl.cpp"

using namespace std::placeholders;


// The main even loop for the application.
void getFrameBuffer(VideoCameraCtl *vc)
{
	while (true)
	{
		int size;
		uint8_t * frame = vc->GetFrameBuffer(size);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

int main(int argc, char *argv[])
{
	VideoCameraCtl vc;
	std::thread t1(getFrameBuffer, vc);
	Start(vc);
	return 0;
}
