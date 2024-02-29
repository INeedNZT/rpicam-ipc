#pragma once
#include "mongoose.h"
#include "core/frame_buffer.hpp"

typedef mg_mgr MongooseManager;
typedef mg_connection MongooseConnection;

// Wrapper for transfer fb_ptr in mongoose
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