add_executable (scummfont "scummfont.cpp")

install(TARGETS scummfont RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
