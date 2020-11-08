include_directories ("../ScummRp")

add_executable (scummtr "trblock.cpp" "scummtr.cpp" "main.cpp" "text.cpp" "script.cpp" "../ScummRp/backup.cpp" "../ScummRp/block.cpp" "../ScummRp/file.cpp" "../ScummRp/scummrp.cpp" "../ScummRp/io.cpp" "../ScummRp/toc.cpp" "../ScummRp/toolbox.cpp")
