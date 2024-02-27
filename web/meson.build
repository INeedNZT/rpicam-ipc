web_server = executable(
    'web_server',
    ['web_server.cpp', 'mongoose.c'],
    dependencies: [libcamera_dep],
    include_directories : include_directories('..'),
    link_with : rpicam_ipc,
)

fs = import('fs')

fs.copyfile('rpicam.html')
fs.copyfile('http-live-player-worker.js')