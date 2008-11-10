
# DOC: http://qtnode.net/wiki/Qt_with_cmake

set(
	kshutdown_SRC
	actions/extras.cpp
	config.cpp
	kshutdown.cpp
	main.cpp
	mainwindow.cpp
	preferences.cpp
	theme.cpp
	utils.cpp
)

set(
	kshutdown_MOC_HEADERS
	actions/extras.h
	kshutdown.h
	mainwindow.h
	preferences.h
)
qt4_wrap_cpp(kshutdown_MOC_SOURCES ${kshutdown_MOC_HEADERS})

add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kshutdown ${QT_LIBRARIES} kdeui kio kworkspace ${KDE4_KNOTIFYCONFIG_LIBS})

install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/kshutdown"
	DESTINATION bin
)
install(FILES kshutdown.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kshutdown.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kshutdown)

add_subdirectory(actions)
add_subdirectory(images)
add_subdirectory(themes)
