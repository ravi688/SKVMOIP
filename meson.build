#------------- Generated By Build Master 1.0.0 ------------------

# ------------------------------ DOCUMENTATION ---------------------------------
# Release build
# -------------------------
# $ meson setup --wipe <builddir> # wipe the build artifacts (like object files)
# $ meson setup <builddir> --reconfigure --buildtype=release # reconfigure the build directory for release build
# $ meson compile -C <builddir> # compile the project
#
# Debug build
# -------------------------
# $ meson setup --wipe <buildir> # wipe the build artifacts (like object files)
# $ meson setup <builddir> --reconfigure --buildtype=release # reconfigure the build directory for debug build
# $ meson compile -C <builddir> # compile the project
#
# Static Library
# -------------------------
# $ meson setup --wipe <buildir> # wipe the build artifacts (like object files)
# # NOTE: --buildtype=release or --buildtype=debug options can be added here  
# $ meson setup -C <builddir> --reconfigure --default-library=static # reconfigure the build directory for static library
# $ meson compile -C <builddir> # compile the project
# $ meson install -C <builddir> # install the static library
#
# Shared Library
# -------------------------
# $ meson setup --wipe <buildir> # whipe the build artifacts (like object files)
# # NOTE: --buildtype=release or --buildtype=debug options can be added here
# $ meson setup -C <builddir> --reconfigure --default-library=shared # reconfigure the build directory for shared library
# $ meson compile -C <builddir> # compile the project
# $ meson install -C <builddir> # install the shared library
#
# Artifact Installation Directories
# ---------------------------------
# Headers: /include/<ProjectNameInSmallCase>
# Static Libraries: /lib/lib<ProjectNameInSmallCase>.a-
# Shared Libraries: /bin/lib<ProjectNameInSmallCase>.dll
# PkgConfig (.pc) for static library: $PKG_CONFIG_PATH/<ProjectNameInSmallCase>_static.pc
# PkgConfig (.pc) for shared library: $PKG_CONFIG_PATH/<ProjectNameInSmallCase>_shared.pc
#
# -------------------------------- PROJECT CONFIGS -----------------------------

project('SKVMOIP', 'c', 'cpp',
  version : '1.0.0',
  meson_version: '>=1.1',
  default_options : [
    'warning_level=3',
    'buildtype=debug',
    'c_std=c17',
    'cpp_std=c++20'
  ]
)

# Variables
cuda_path = run_command(find_program('python'), '-c', 'import os; print(os.environ["CUDA_PATH"])', check : false).stdout().strip()
cuda_includes = cuda_path + '/include'
cuda_libs_path = cuda_path + '/lib/x64'
vulkan_sdk_path = run_command(find_program('python'), '-c', 'import os; print(os.environ["VK_SDK_PATH"])', check : false).stdout().strip()
vulkan_libs_path = vulkan_sdk_path + '/Lib/'
gtk3_cflags = run_command('pkg-config', 'gtk+-3.0', '--cflags', check : false).stdout().strip().split(' ')
gtk3_libs = run_command('pkg-config', 'gtk+-3.0', '--libs', check : false).stdout().strip().split(' ')
gui_sources = [
'source/GUI/AddUI.cpp',
'source/GUI/MachineUI.cpp',
'source/GUI/MainUI.cpp'
]
server_sources = [
'source/Encoder.cpp',
'source/HDMIEncodeNetStream.cpp',
'source/Win32/Win32ImagingDevice.cpp'
]
client_sources = [
'source/third_party/NvDecoder.cpp',
'source/Decoder.cpp',
'source/Window.cpp',
'source/Win32/Win32DrawSurface.cpp',
'source/Win32/Win32RawInput.cpp',
'source/Win32/Win32Window.cpp',
'source/HDMIDecoderNetStream.cpp',
'source/KMNetStream.cpp',
'source/HIDUsageID.cpp',
'source/RDPSession.cpp',
'source/PresentEngine.cpp',
'source/MachineData.cpp',
'source/FIFOPool.cpp',
'source/Event.cpp',
'source/Network/NetworkPacket.cpp',
'source/ErrorHandling.cpp'
]


# Defines
defines_bm_internal__ = [
'-DUSE_PERSISTENT_SETTINGS', '-DUSE_VULKAN_PRESENTATION', '-DUSE_VULKAN_FOR_COLOR_SPACE_CONVERSION', '-DUSE_DIRECT_FRAME_DATA_COPY'
]

# Release Build Defines
release_defines_bm_internal__ = [
'-DSKVMOIP_RELEASE'
] 

# Debug Build Defines
debug_defines_bm_internal__ = [
'-DSKVMOIP_DEBUG'
]

# Source files (common to all targets)
sources_bm_internal__ = files(
'source/Network/NetworkSocket.cpp', 'source/Network/NetworkAsyncQueueSocket.cpp', 'source/VideoSourceStream.cpp', 'source/Win32/Win32.cpp'
)

# Include directories
inc_bm_internal__ = include_directories(
'include', 'external-dependencies'
)

# Library Install Directory
lib_install_dir_bm_internal__ = get_option('libdir')/'skvmoip'

# Dependencies
dependencies_bm_internal__ = [
dependency('common'),
dependency('playvk'),
dependency('bufferlib')
]

# Linker Arguments
windows_link_args_bm_internal__ = [ 
'-lws2_32', '-lole32', '-loleaut32', '-lmfreadwrite', '-lmfplat', '-lmf', '-lmfuuid', '-lgdi32', '-lwmcodecdspuuid'
]
linux_link_args_bm_internal__ = [

]
darwin_link_args_bm_internal__ = [

]

# -------------------------------------------------------------------------------
# ------------------------------ FIXTURE ----------------------------------------

# Compiler configuration
add_project_arguments('-m64', language : 'c')
add_project_arguments('-m64', language : 'cpp')
# Linker configuration
link_args_bm_internal__ = []
os_name_bm_internal__ = host_machine.system()
if os_name_bm_internal__ == 'windows'
  link_args_bm_internal__ += windows_link_args_bm_internal__
elif os_name_bm_internal__ == 'linux'
  link_args_bm_internal__ += linux_link_args_bm_internal__
elif os_name_bm_internal__ == 'darwin'
  link_args_bm_internal__ += darwin_link_args_bm_internal__
endif
add_project_link_arguments('-m64', link_args_bm_internal__, language : 'c')
add_project_link_arguments('-m64', link_args_bm_internal__, language : 'cpp')

# Build type specific defines
project_build_mode_defines_bm_internal__ = defines_bm_internal__
if get_option('buildtype') == 'release'
  add_project_arguments(release_defines_bm_internal__, language : 'c')
  add_project_arguments(release_defines_bm_internal__, language : 'cpp')
  project_build_mode_defines_bm_internal__ += release_defines_bm_internal__
else
  add_project_arguments(debug_defines_bm_internal__, language : 'c')
  add_project_arguments(debug_defines_bm_internal__, language : 'cpp')
  project_build_mode_defines_bm_internal__ += debug_defines_bm_internal__
endif

# pkg-config package installation
# Try PKG_CONFIG_PATH first, typicallly it succeeds on MINGW64 (MSYS2)
python_pkg_config_path_result_bm_internal__ = run_command(find_program('python'), '-c', 'import os; print(os.environ["PKG_CONFIG_PATH"])', check : false)
pkgconfig_install_path_bm_internal__ = ''
if python_pkg_config_path_result_bm_internal__.returncode() == 0
  str = python_pkg_config_path_result_bm_internal__.stdout()
  # Unix
  if str.startswith('/')
    pkgconfig_install_path_bm_internal__ = str.replace(';', ':').split(':')[0]
  # Windows
  else
    pkgconfig_install_path_bm_internal__ = str.split(';')[0]
  endif
endif
if pkgconfig_install_path_bm_internal__ == ''
  # Otherwise use pkg-config to query its lookup directories
  message('PKG_CONFIG_PATH seems to be empty, trying another method')
  python_pkg_config_path_result_bm_internal__ = run_command('pkg-config', '--variable', 'pc_path', 'pkg-config', check : false)
  if python_pkg_config_path_result_bm_internal__.returncode() == 0
    str = python_pkg_config_path_result_bm_internal__.stdout()
    if str.startswith('/')
      pkgconfig_install_path_bm_internal__ = str.replace(';', ':').split(':')[0]
    else
      pkgconfig_install_path_bm_internal__ = str.split(';')[0]
    endif
  # Finally if the above attempts fail, use 'libdir' value
  else
    pkgconfig_install_path_bm_internal__ = get_option('libdir')
  endif
endif
message('pkg config path: ' + pkgconfig_install_path_bm_internal__)

#-------------------------------------------------------------------------------
#--------------------------------BUILD TARGETS----------------------------------
pkgmod = import('pkgconfig')

# -------------- Target: server ------------------
server_sources_bm_internal__ = [
server_sources,
'source/main.server.cpp'
]
server_include_dirs_bm_internal__ = [

]
server_dependencies_bm_internal__ = [

]
server_link_args_bm_internal__ = {
'windows' : ['-L' + meson.project_source_root() + '/' +  './external-dependencies/x264', '-lx264'],
'linux' : [],
'darwin' : []
}
server_defines_bm_internal__ = [
'-DBUILD_SERVER'
]
server = executable('server',
	server_sources_bm_internal__ + sources_bm_internal__,
	dependencies: dependencies_bm_internal__ + server_dependencies_bm_internal__,
	include_directories: [inc_bm_internal__, server_include_dirs_bm_internal__],
	install: false,
	c_args: server_defines_bm_internal__ + project_build_mode_defines_bm_internal__,
	cpp_args: server_defines_bm_internal__ + project_build_mode_defines_bm_internal__, 
	link_args: server_link_args_bm_internal__[host_machine.system()],
	gnu_symbol_visibility: 'hidden'
)

# -------------- Target: client ------------------
client_sources_bm_internal__ = [
client_sources,
gui_sources,
'source/main.client.cpp'
]
client_include_dirs_bm_internal__ = [
'./external-dependencies/NvidiaCodec/include/',
cuda_includes
]
client_dependencies_bm_internal__ = [
dependency('gtk+-3.0'),
dependency('vulkanheaders')
]
client_link_args_bm_internal__ = {
'windows' : ['-L' +  cuda_libs_path, '-l:cuda.lib', '-L' + meson.project_source_root() + '/' +  './external-dependencies/NvidiaCodec/', '-l:nvcuvid.lib', '-L' +  vulkan_libs_path, '-lvulkan-1'],
'linux' : [],
'darwin' : []
}
client_defines_bm_internal__ = [
'-DBUILD_CLIENT'
]
client = executable('client',
	client_sources_bm_internal__ + sources_bm_internal__,
	dependencies: dependencies_bm_internal__ + client_dependencies_bm_internal__,
	include_directories: [inc_bm_internal__, client_include_dirs_bm_internal__],
	install: false,
	c_args: client_defines_bm_internal__ + project_build_mode_defines_bm_internal__,
	cpp_args: client_defines_bm_internal__ + project_build_mode_defines_bm_internal__, 
	link_args: client_link_args_bm_internal__[host_machine.system()],
	gnu_symbol_visibility: 'hidden'
)


#-------------------------------------------------------------------------------
#--------------------------------Header Intallation----------------------------------
# Header installation
install_subdir('include/SKVMOIP', install_dir : get_option('includedir'))


