// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved
//
// Example Websocket server. See https://mongoose.ws/tutorials/websocket-server/

#include "mongoose.h"
#include "controller/video_camera_ctl.hpp"
#include <string>

static const char *s_listen_on = "ws://localhost:8000";
static const char *s_web_root = ".";

struct mg_mgr mgr;

std::string json = R"({"action": "init", "width": 1920, "height": 1080})";

VideoCameraCtl vc;

// 一个vector用来存储所有的连接的id
std::vector<unsigned long> connections;

static void sendDataThread(void *arg)
{
  // 将vc转换为VideoCameraCtl类型
  printf("sendDataThread\n");
  struct mg_connection *c;
  for (c = mgr.conns; c != NULL; c = c->next)
  {
    if (c->data[0] != 'W')
      continue;
    int size;
    const void *buffer_ptr = vc.GetFrameBuffer(size);
    mg_ws_send(c, buffer_ptr, size, WEBSOCKET_OP_BINARY);
  }
}

// This RESTful server implements the following endpoints:
//   /websocket - upgrade to Websocket, and implement websocket echo server
//   /rest - respond with JSON string {"result": 123}
//   any other URI serves static files from s_web_root
static void fn(struct mg_connection *c, int ev, void *ev_data)
{
  if (ev == MG_EV_OPEN)
  {
    // c->is_hexdumping = 1;
  }
  else if (ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_http_match_uri(hm, "/rpicam"))
    {
      // Upgrade to websocket. From now on, a connection is a full-duplex
      // Websocket connection, which will receive MG_EV_WS_MSG events.
      mg_ws_upgrade(c, hm, NULL);
      c->data[0] = 'W'; 
    }
    else if (mg_http_match_uri(hm, "/rest"))
    {
      // Serve REST response
      mg_http_reply(c, 200, "", "{\"result\": %d}\n", 123);
    }
    else
    {
      // Serve static files
      static struct mg_http_serve_opts opts; // 默认初始化
      opts.root_dir = s_web_root;            // 单独设置需要的成员变量
      mg_http_serve_dir(c, hm, &opts);
    }
  }
  else if (ev == MG_EV_WS_MSG)
  {
    using namespace std;
    // Got websocket frame. Received data is wm->data. Echo it back!
    struct mg_ws_message *wm = (struct mg_ws_message *)ev_data;

    if (string(wm->data.ptr) == "REQUESTSTREAM ")
    {
      printf("start\n");
    }
    else if (string(wm->data.ptr) == "STOPSTREAM ")
    {
      printf("stop\n");
    }
    // 用空格split字符串wm->data.ptr
    printf("Got message: [%.*s]\n", (int)wm->data.len, wm->data.ptr);
  }
  else if (ev == MG_EV_WS_OPEN)
  {
    printf("WS connection opened\n");
    mg_ws_send(c, json.c_str(), json.length(), WEBSOCKET_OP_TEXT);
    connections.push_back(c->id);
  }
  else if (ev == MG_EV_CLOSE)
  {
    printf("WS connection closed\n");
  }
}

int main(void)
{
  // Start camera first
  std::thread t1(&VideoCameraCtl::Start, &vc);
  mg_mgr_init(&mgr); // Initialise event manager
  printf("Starting WS listener on %s/websocket\n", s_listen_on);
  mg_http_listen(&mgr, s_listen_on, fn, NULL); // Create HTTP listener
  mg_timer_add(&mgr, 25, MG_TIMER_REPEAT, sendDataThread, &mgr);
  for (;;)
    mg_mgr_poll(&mgr, 20); // Infinite event loop
  mg_mgr_free(&mgr);
  return 0;
}
