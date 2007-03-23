
set(
	kshutdown_SRC
	kshutdown.cpp
	main.cpp
	mainwindow.cpp
)

qt4_automoc(${kshutdown_SRC})

add_executable(kshutdown ${kshutdown_SRC})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kshutdown ${QT_LIBRARIES} kdeui kworkspace)

install(
	PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/kshutdown"
	DESTINATION bin
)
