
# DOC: http://qtnode.net/wiki/Qt_with_cmake

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
	password.cpp
	preferences.cpp
	progressbar.cpp
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
	udialog.h
	usystemtray.h
)
qt4_wrap_cpp(kshutdown_MOC_SOURCES ${kshutdown_MOC_HEADERS})

add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kshutdown kdecore kdeui kio ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KUTILS_LIBS})

install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/kshutdown"
	DESTINATION bin
)
install(FILES kshutdown.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kshutdown.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kshutdown)

add_subdirectory(actions)
add_subdirectory(images)
add_subdirectory(triggers)
