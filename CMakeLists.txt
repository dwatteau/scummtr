cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

project ("ScummTR" C CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if (MSVC)
    # /wd4996?
    add_compile_options(/wd4244 /wd4267 /wd26451 /wd26495)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN)
    if(IS_BIG_ENDIAN)
        add_definitions(/DSCUMMTR_USE_BIG_ENDIAN)
    endif()
else()
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wshadow -Wuninitialized -Wundef -Wmultichar -Wwrite-strings -Wctor-dtor-privacy)
    add_compile_options(-Wno-non-virtual-dtor -Wno-conversion -Wno-parentheses -Wno-overloaded-virtual -Wno-format)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    endif()

    if(IS_BIG_ENDIAN)
        add_definitions(-DSCUMMTR_USE_BIG_ENDIAN)
    endif()

    if (WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
    endif()
endif()

add_subdirectory ("src/FontXY")
add_subdirectory ("src/ScummFont")
add_subdirectory ("src/ScummRp")
add_subdirectory ("src/ScummTr")
