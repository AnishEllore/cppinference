cmake_minimum_required(VERSION 3.10)

project(cpp_inference_server)
set(CMAKE_CXX_STANDARD 17)

set(Torch_DIR "/home/cc/anish/libtorch/share/cmake/Torch")
set(CMAKE_MODULE_PATH /home/cc/anish/projectX/cmake/cmakemodule/)


find_package(LibHttpServer REQUIRED)
include_directories(${LIBHTTPSERVER_INCLUDE_DIRS})
include_directories(include)

# Find and include the Torch library
find_package(Torch REQUIRED)

# Add your project's executable or library target
add_executable(server server.cpp)

# Link against the libhttpserver and Torch libraries
target_link_libraries(server ${LIBHTTPSERVER_LIBRARIES} ${TORCH_LIBRARIES})