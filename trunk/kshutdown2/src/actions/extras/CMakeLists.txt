file(GLOB desktop "*.desktop")
install(FILES ${desktop} DESTINATION ${DATA_INSTALL_DIR}/kshutdown/extras)

add_subdirectory(multimedia)
add_subdirectory(system)
