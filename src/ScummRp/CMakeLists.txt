add_library(scummrplib STATIC "block.cpp" "block.hpp" "rptypes.hpp" "scummrp.cpp" "scummrp.hpp" "toc.cpp" "toc.hpp")

add_executable(scummrp "main.cpp")
target_link_libraries(scummrp PUBLIC scummrplib scummiolib)

install(TARGETS scummrp RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
