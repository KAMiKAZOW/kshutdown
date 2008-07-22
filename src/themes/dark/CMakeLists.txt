file(GLOB images "*.png")
install(FILES style.css theme.xml ${images} DESTINATION ${DATA_INSTALL_DIR}/kshutdown/themes/dark)
