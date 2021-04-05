add_executable (scummtr "main.cpp" "script.cpp" "scummtr.cpp" "text.cpp" "trblock.cpp")

target_link_libraries (scummtr PUBLIC scummrplib scummiolib)

install(TARGETS scummtr RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
