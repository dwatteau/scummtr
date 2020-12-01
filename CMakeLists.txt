cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No explicit CMAKE_BUILD_TYPE selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

project ("ScummTR" C CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)

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
    # GCC 2.95 compatible warnings we always want
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wshadow -Wuninitialized -Wundef -Wmultichar -Wwrite-strings -Wctor-dtor-privacy -Wredundant-decls -Wmissing-declarations -Wunused)
    # GCC 2.95 compatible warnings we don't want (yet)
    add_compile_options(-Wno-non-virtual-dtor -Wno-conversion -Wno-overloaded-virtual -Wno-old-style-cast -Wno-format)

    # Flags which are compiler-specific, or only meaningful with a particular compiler
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options(-Wshadow -Widiomatic-parentheses)
    endif()

    # Warnings requiring a recent compiler. Do a single test, it's not critical
    set(SCUMMTR_MODERN_WARNING_FLAGS "-Winit-self;-Wtype-limits;-Wignored-qualifiers")
    CHECK_CXX_COMPILER_FLAG("${SCUMMTR_MODERN_WARNING_FLAGS}" COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
    if(COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
        add_compile_options("${SCUMMTR_MODERN_WARNING_FLAGS}")
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
