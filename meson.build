project('rpicam-ipc', 'c', 'cpp',
        meson_version : '>= 0.64.0',
        version : '1.4.2',
        default_options : [
            'werror=true',
            'warning_level=3',
            'cpp_std=c++17',
            'c_std=c11',
            'buildtype=debug',
        ])

c_arguments = ['-D_GNU_SOURCE']
cpp_arguments = ['-pedantic', '-Wno-unused-parameter', '-faligned-new']

# Needed for file sizes > 32-bits.
cpp_arguments += '-D_FILE_OFFSET_BITS=64'

cxx = meson.get_compiler('cpp')
cpu = host_machine.cpu()

if cxx.get_id() == 'gcc'
    cpp_arguments += '-Wno-psabi'
endif

libcamera_dep = dependency('libcamera', required : true)

rpicam_ipc_src = []
rpicam_ipc_dep = [libcamera_dep]

subdir('controller')
subdir('core')
subdir('encoder')
subdir('output')

add_project_arguments(cpp_arguments, language : 'cpp')
add_project_arguments(c_arguments, language : 'c')

rpicam_ipc = library(
    'rpicam_ipc',
    rpicam_ipc_src,
    include_directories : include_directories('.'),
    dependencies : rpicam_ipc_dep,
)

subdir('web')