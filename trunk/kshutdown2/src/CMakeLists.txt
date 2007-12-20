
# DOC: http://qtnode.net/wiki/Qt_with_cmake

set(
	kshutdown_SRC
	config.cpp
	kshutdown.cpp
	main.cpp
	mainwindow.cpp
	preferences.cpp
	theme.cpp
)

set(
	kshutdown_MOC_HEADERS
	kshutdown.h
	mainwindow.h
	preferences.h
)
qt4_wrap_cpp(kshutdown_MOC_SOURCES ${kshutdown_MOC_HEADERS})

add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kshutdown ${QT_LIBRARIES} kdeui kworkspace)

install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/kshutdown"
	DESTINATION bin
)

add_subdirectory(images)
