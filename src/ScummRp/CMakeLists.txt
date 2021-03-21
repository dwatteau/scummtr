add_library (scummrplib STATIC "backup.cpp" "block.cpp" "file.cpp" "io.cpp" "scummrp.cpp" "toc.cpp" "toolbox.cpp")

add_executable (scummrp "main.cpp")
target_link_libraries (scummrp PUBLIC scummrplib)

install(TARGETS scummrp RUNTIME DESTINATION bin)
