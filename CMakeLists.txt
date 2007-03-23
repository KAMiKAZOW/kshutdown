
# DOC: http://qtnode.net/wiki/Qt_with_cmake

project(kshutdown)

cmake_minimum_required(VERSION 2.4.0)

find_package(KDE4 REQUIRED)

add_definitions(${KDE4_DEFINITIONS})

include_directories(${KDE4_INCLUDES})
link_directories(${KDE4_LIB_DIR})

add_subdirectory(src)
