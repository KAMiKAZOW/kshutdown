if(KS_PURE_QT)
# TODO: see src.pro
elseif(KS_KF5)
# FIXME: deprecated
	ecm_install_icons(${ICON_INSTALL_DIR})
else()
	kde4_install_icons(${ICON_INSTALL_DIR})
endif()
