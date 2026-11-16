cmake_minimum_required(VERSION 3.30)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_SHARED_LIBS OFF)
    add_link_options(-static-libgcc -static-libstdc++)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_SHARED_LIBS ON)
endif()

add_executable(server src/server/main.cpp src/net.cpp)
add_executable(linux_client src/client/linux/main.cpp src/net.cpp)
