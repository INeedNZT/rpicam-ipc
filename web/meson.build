rpicam_ipc_web += files([
    'webu.cpp',
    'mongoose.c'
])

fs = import('fs')

fs.copyfile('index.html')
fs.copyfile('rpicam.html')
fs.copyfile('http-live-player-worker.js')
fs.copyfile('jquery.min.js')
fs.copyfile('bootstrap.bundle.min.js')
fs.copyfile('Chart.min.js')
fs.copyfile('adminlte.min.js')
fs.copyfile('main.js')

fs.copyfile('adminlte.min.css')
fs.copyfile('fontawesome.min.css')
fs.copyfile('main.css')

fonts = [
  'fa-brands-400.eot',
  'fa-brands-400.svg',
  'fa-brands-400.ttf',
  'fa-brands-400.woff',
  'fa-brands-400.woff2',
  'fa-regular-400.eot',
  'fa-regular-400.svg',
  'fa-regular-400.ttf',
  'fa-regular-400.woff',
  'fa-regular-400.woff2',
  'fa-solid-900.eot',
  'fa-solid-900.svg',
  'fa-solid-900.ttf',
  'fa-solid-900.woff',
  'fa-solid-900.woff2',
]

foreach file : fonts
  fs.copyfile('webfonts' / file)
endforeach