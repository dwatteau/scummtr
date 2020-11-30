cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No explicit CMAKE_BUILD_TYPE selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

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
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wshadow -Wuninitialized -Wundef -Wmultichar -Wwrite-strings -Wctor-dtor-privacy -Wredundant-decls -Wmissing-declarations -Wunused)
    add_compile_options(-Wno-non-virtual-dtor -Wno-conversion -Wno-overloaded-virtual -Wno-old-style-cast -Wno-format)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options(-Wshadow -Widiomatic-parentheses)
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
