set(
	kshutdown_SRC
	actions/bootentry.cpp
	actions/extras.cpp
	actions/lock.cpp
	actions/test.cpp
	triggers/idlemonitor.cpp
	triggers/processmonitor.cpp
	bookmarks.cpp
	commandline.cpp
	config.cpp
	infowidget.cpp
	kshutdown.cpp
	main.cpp
	mainwindow.cpp
	mod.cpp
	password.cpp
	preferences.cpp
	progressbar.cpp
	stats.cpp
	udialog.cpp
	usystemtray.cpp
	utils.cpp
)

set(
	kshutdown_MOC_HEADERS
	actions/bootentry.h
	actions/extras.h
	triggers/idlemonitor.h
	triggers/processmonitor.h
	bookmarks.h
	infowidget.h
	kshutdown.h
	mainwindow.h
	password.h
	preferences.h
	progressbar.h
	stats.h
	udialog.h
	usystemtray.h
)
if(KS_PURE_QT)
	QT5_WRAP_CPP(kshutdown_MOC_SOURCES ${kshutdown_MOC_HEADERS})
elseif(KS_KF5)
	# automoc
else()
	qt4_wrap_cpp(kshutdown_MOC_SOURCES ${kshutdown_MOC_HEADERS})
endif()

add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(KS_PURE_QT)
	target_link_libraries(kshutdown ${Qt5Widgets_LIBRARIES} ${Qt5DBus_LIBRARIES})
elseif(KS_KF5)
	target_link_libraries(kshutdown Qt5::DBus Qt5::Widgets KF5::ConfigCore KF5::ConfigWidgets KF5::DBusAddons KF5::GlobalAccel KF5::I18n KF5::IdleTime KF5::Notifications KF5::NotifyConfig KF5::XmlGui)
else()
	target_link_libraries(kshutdown kdecore kdeui kio ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KUTILS_LIBS})
endif()

install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/kshutdown"
	DESTINATION bin
)
if(KS_PURE_QT)
# TODO: install in /usr/share/kshutdown?
	set(DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/apps CACHE FILEPATH "Replacement for KDE4 variable")
	set(XDG_APPS_INSTALL_DIR ${DATA_INSTALL_DIR}/share/applications CACHE FILEPATH "Install path for .desktop files")
elseif(KS_KF5)
	install(FILES kshutdown.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
else()
	install(FILES kshutdown.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kshutdown)
endif()
install(FILES kshutdown.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

add_subdirectory(actions)
add_subdirectory(images)
add_subdirectory(triggers)
