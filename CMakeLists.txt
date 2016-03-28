project(kshutdown)

if(KS_KF5)
	cmake_minimum_required(VERSION 3.0.0)
else()
	cmake_minimum_required(VERSION 2.4.0)
endif()

add_definitions(-std=c++0x -Wextra -Wpedantic -Wswitch-enum)

if(KS_PURE_QT)
	find_package(Qt5 REQUIRED Widgets DBus)
	add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5DBus_DEFINITIONS})
	include_directories(${Qt5Widgets_INCLUDES} ${Qt5DBus_INCLUDES})

	add_definitions(-DKS_PURE_QT)
	set(CMAKE_AUTOMOC_MOC_OPTIONS -DKS_PURE_QT)
elseif(KS_KF5)
	find_package(Qt5 REQUIRED Widgets DBus)

	find_package(ECM REQUIRED NO_MODULE)
	set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

	include(KDEInstallDirs)
	include(KDECompilerSettings)
	include(KDECMakeSettings)
	include(ECMInstallIcons)
	include(ECMSetupVersion)
	include(FeatureSummary)
	find_package(KF5 REQUIRED COMPONENTS Config ConfigWidgets DBusAddons GlobalAccel I18n IdleTime Notifications NotifyConfig XmlGui)

	feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

	add_definitions(-DKS_KF5)
# TODO: need this?
	set(CMAKE_AUTOMOC_MOC_OPTIONS -DKS_KF5)

	add_subdirectory(po)
else()
	set(CMAKE_AUTOMOC TRUE)
	set(CMAKE_AUTOMOC_MOC_OPTIONS -DKS_NATIVE_KDE)

	find_package(KDE4 REQUIRED)
	add_definitions(${KDE4_DEFINITIONS})

	include_directories(${KDE4_INCLUDES})
	link_directories(${KDE4_LIB_DIR})

	add_subdirectory(po)
endif()

add_subdirectory(src)
