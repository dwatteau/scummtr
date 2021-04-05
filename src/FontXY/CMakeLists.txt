add_executable (FontXY "FontXY.cpp")

target_link_libraries (FontXY PUBLIC scummiolib)

install(TARGETS FontXY RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
