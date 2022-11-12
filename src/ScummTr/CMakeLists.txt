add_executable(scummtr "main.cpp" "script.cpp" "script.hpp" "scummtr.cpp" "scummtr.hpp" "text.cpp" "text.hpp" "trblock.cpp" "trblock.hpp")

target_link_libraries(scummtr PUBLIC scummrplib scummiolib)

install(TARGETS scummtr RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
