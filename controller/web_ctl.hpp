#include <string>

#include "web/webu.hpp"

class WebCtl
{
public:
    WebCtl(MongooseManager &mgr, const std::string &web_root, const std::string &listening_port);
    ~WebCtl();

    void Start();
    void Stop();

private:
    MongooseManager& mgr_;
};