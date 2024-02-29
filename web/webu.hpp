#pragma once
#include "mongoose.h"
#include "core/frame_buffer.hpp"

typedef mg_mgr MongooseManager;
typedef mg_connection MongooseConnection;

struct FrameBufferPtrWrapper
{
    FrameBufferPtr fb_ptr;
};

class WebU
{
public:
    WebU();
    ~WebU();

    static void DefaultHandler(MongooseConnection *conn, int ev, void *ev_data);
};