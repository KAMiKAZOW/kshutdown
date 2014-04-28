project(kshutdown)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-std=c++0x)

if(KS_PURE_QT)
	find_package(Qt5 REQUIRED Widgets DBus)
	add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5DBus_DEFINITIONS})
	include_directories(${Qt5Widgets_INCLUDES} ${Qt5DBus_INCLUDES})

	add_definitions(-DKS_PURE_QT)
	set(CMAKE_AUTOMOC_MOC_OPTIONS -DKS_PURE_QT)
else()
	find_package(KDE4 REQUIRED)
	add_definitions(${KDE4_DEFINITIONS})

	include_directories(${KDE4_INCLUDES})
	link_directories(${KDE4_LIB_DIR})
	
	add_subdirectory(po)
endif()

add_subdirectory(src)
