#include "web_ctl.hpp"
#include <iostream>

WebCtl::WebCtl(MongooseManager &mgr, const std::string &web_root, const std::string &listening_port)
    : mgr_(mgr)
{
    mg_mgr_init(&mgr_);
    mg_wakeup_init(&mgr_);
    mg_http_listen(&mgr_, listening_port.c_str(), WebU::DefaultHandler, NULL);
}

WebCtl::~WebCtl()
{
    mg_mgr_free(&mgr_);
}

void WebCtl::Start()
{
    std::cout << "WebCtl::Start" << std::endl;
    while (true)
    {
        mg_mgr_poll(&mgr_, 1000);
    }
}

void WebCtl::Stop()
{
    mg_mgr_free(&mgr_);
}
