
# DOC: http://qtnode.net/wiki/Qt_with_cmake

project(kshutdown)

cmake_minimum_required(VERSION 2.4.0)

find_package(KDE4 REQUIRED)

# find KWorkspace package
find_path(
	KWORKSPACE_INCLUDE_DIR NAMES kworkspace/kworkspace.h
	PATHS ${KDE4_INCLUDES}
)
find_library(
	KWORKSPACE_LIBS NAMES kworkspace
	PATHS ${KDE4_LIB_DIR}
)

# FIXME: ?
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(KWorkspace DEFAULT_MSG KWORKSPACE_LIBS KWORKSPACE_INCLUDE_DIR)

if (NOT KWORKSPACE_FOUND)
	message(FATAL_ERROR "KDE Workspace headers not found. Try to install \"kdebase-workspace-dev\" package.")
endif (NOT KWORKSPACE_FOUND)

add_definitions(${KDE4_DEFINITIONS})

include_directories(${KDE4_INCLUDES})
link_directories(${KDE4_LIB_DIR})

add_subdirectory(src)
