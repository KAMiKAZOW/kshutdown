project(kshutdown)

cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0063 NEW) # disables cmake warning

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-Wextra -Wpedantic -Wswitch-enum)

find_package(Qt5 REQUIRED Widgets DBus)

if(KS_PURE_QT)
	add_definitions(-DKS_PURE_QT)

	set(DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share)
	set(ICON_INSTALL_DIR ${DATA_INSTALL_DIR}/icons)
	set(XDG_APPS_INSTALL_DIR ${DATA_INSTALL_DIR}/applications)

# TODO: https://cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
	set(CMAKE_AUTOMOC TRUE)
else()
	add_definitions(-DKS_KF5)

	find_package(ECM REQUIRED NO_MODULE)
	set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

	include(KDEInstallDirs)
	include(KDECompilerSettings)
# TODO: include(KDEFrameworkCompilerSettings) (?)
	include(KDECMakeSettings)

	find_package(KF5 REQUIRED COMPONENTS
		Config
		ConfigWidgets
		Crash
		DBusAddons
		GlobalAccel
		I18n
		IdleTime
		Notifications
		NotifyConfig
		XmlGui
	)

	add_subdirectory(po)
endif()

include(FeatureSummary)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(src)
