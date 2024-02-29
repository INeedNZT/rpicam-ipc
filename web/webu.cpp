#include "webu.hpp"
#include <string>

// Assuming the resoulution is 1920x1080
std::string json = R"({"action": "init", "width": 1920, "height": 1080})";

WebU::WebU()
{
}

WebU::~WebU()
{
}

void WebU::DefaultHandler(MongooseConnection *conn, int ev, void *ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        if (mg_http_match_uri(hm, "/rpicam"))
        {
            mg_ws_upgrade(conn, hm, NULL);
            conn->data[0] = 'W';
        }
        else
        {
            static struct mg_http_serve_opts opts;
            opts.root_dir = ".";
            mg_http_serve_dir(conn, hm, &opts);
        }
    }
    else if (ev == MG_EV_WS_MSG)
    {
        using namespace std;
        struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;

        if (string(wm->data.ptr) == "REQUESTSTREAM ")
        {
            printf("Start Stream\n");
        }
        else if (string(wm->data.ptr) == "STOPSTREAM ")
        {
            printf("Stop Stream\n");
        }
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        printf("WS connection opened\n");
        mg_ws_send(conn, json.c_str(), json.length(), WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_WAKEUP)
    {
        struct mg_str *data = (struct mg_str *)ev_data;
        FrameBufferPtrWrapper* fb_ptr_wrapper = *(FrameBufferPtrWrapper **)data->ptr;
        FrameBufferPtr* fb_ptr = fb_ptr_wrapper->fb_ptr;
        mg_ws_send(conn, fb_ptr->get()->mem, fb_ptr->get()->size, WEBSOCKET_OP_BINARY);
        // After sending frame buffer, delete the wrapper, the share ptr
        // will automatically delete the frame buffer as well when it's
        // no longer in use
        // FIXME: This is a memory leak, the wrapper should be deleted
        // delete fb_ptr_wrapper;
        // fb_ptr_wrapper = nullptr;
    }
}
