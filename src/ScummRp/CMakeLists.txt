add_library (scummrplib STATIC "block.cpp" "scummrp.cpp" "toc.cpp")

add_executable (scummrp "main.cpp")
target_link_libraries (scummrp PUBLIC scummrplib scummiolib)
