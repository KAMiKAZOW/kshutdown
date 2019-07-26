set(kshutdown_SRC
	bookmarks.cpp
	commandline.cpp
	config.cpp
	infowidget.cpp
	kshutdown.cpp
	log.cpp
	main.cpp
	mainwindow.cpp
	mod.cpp
	password.cpp
	plugins.cpp
	preferences.cpp
	progressbar.cpp
	udialog.cpp
	usystemtray.cpp
	utils.cpp
	actions/bootentry.cpp
	actions/extras.cpp
	actions/lock.cpp
	actions/test.cpp
	triggers/idlemonitor.cpp
	triggers/processmonitor.cpp
)

if(KS_PURE_QT)
	QT5_ADD_RESOURCES(qrc_kshutdown.cpp kshutdown.qrc)
	add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES} qrc_kshutdown.cpp)
else()
	add_executable(kshutdown ${kshutdown_SRC} ${kshutdown_MOC_SOURCES})
endif()

# HACK: Add -std=gnu++14 option (cmake horror):
target_compile_features(kshutdown PUBLIC cxx_std_14)

if(KS_PURE_QT)
	target_link_libraries(kshutdown
		Qt5::DBus
		Qt5::Widgets
	)
else()
	target_link_libraries(kshutdown
		Qt5::DBus
		Qt5::Widgets
		KF5::ConfigCore
		KF5::ConfigWidgets
		KF5::Crash
		KF5::DBusAddons
		KF5::GlobalAccel
		KF5::I18n
		KF5::IdleTime
		KF5::Notifications
		KF5::NotifyConfig
		KF5::XmlGui
	)
endif()

# TODO: create symlink to "kshutdown-qt" for compat. with Qt5 Build (?)
install(TARGETS kshutdown DESTINATION bin)
install(FILES kshutdown.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

if(KS_KF5)
	install(FILES kshutdown.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
endif()

add_subdirectory(actions)
add_subdirectory(images)
add_subdirectory(triggers)
