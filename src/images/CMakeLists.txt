if(KS_PURE_QT)
	# TODO
elseif(KS_KF5)
	# TODO: ecm_install_icons(${ICON_INSTALL_DIR})
else()
	kde4_install_icons(${ICON_INSTALL_DIR})
endif()
