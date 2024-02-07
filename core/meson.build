boost_dep = dependency('boost', modules : ['program_options'], required : true)
thread_dep = dependency('threads', required : true)

rpicam_ipc_dep += [boost_dep, thread_dep]

rpicam_ipc_src += files([
    'buffer_sync.cpp',
    'dma_heaps.cpp',
    'video_camera.cpp',
    'video_camera_ctl.cpp',
])