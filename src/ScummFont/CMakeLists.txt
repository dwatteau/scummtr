add_executable (scummfont "scummfont.cpp")

target_link_libraries (scummfont PUBLIC scummiolib)

install(TARGETS scummfont RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
