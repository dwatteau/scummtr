add_executable (scummtr "main.cpp" "script.cpp" "scummtr.cpp" "text.cpp" "trblock.cpp")

target_include_directories (scummtr PUBLIC "../ScummRp")
target_link_libraries (scummtr PUBLIC scummrplib)

install(TARGETS scummtr RUNTIME DESTINATION bin)
