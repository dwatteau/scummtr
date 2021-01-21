cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No explicit CMAKE_BUILD_TYPE selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

project ("ScummTR" CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)

if (MSVC)
    # /wd4996?
    add_compile_options(/wd4244 /wd4267 /wd26451 /wd26495)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN)
else()
    # GCC 2.95 compatible warnings we always want
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wuninitialized -Wundef -Wmultichar -Wwrite-strings -Wctor-dtor-privacy -Wredundant-decls -Wunused -Wsign-compare)
    # GCC 2.95 compatible warnings we don't want (yet)
    add_compile_options(-Wno-non-virtual-dtor -Wno-conversion -Wno-overloaded-virtual -Wno-old-style-cast -Wno-format)

    # Flags which are compiler-specific, or only meaningful with a particular compiler
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options(-Widiomatic-parentheses -Wparentheses -Wimplicit-int-conversion)
    endif()

    # Warnings requiring a recent compiler. Do a single test, it's not critical
    set(SCUMMTR_MODERN_WARNING_FLAGS "-Winit-self;-Wtype-limits;-Wignored-qualifiers") # -Wzero-as-null-pointer-constant for C++11
    CHECK_CXX_COMPILER_FLAG("${SCUMMTR_MODERN_WARNING_FLAGS}" COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
    if(COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
        add_compile_options("${SCUMMTR_MODERN_WARNING_FLAGS};-Wshadow")
    endif()

    if (MINGW)
        add_definitions(-D__USE_MINGW_ANSI_STDIO)
    else()
        add_definitions(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)
        set(SCUMMTR_HARDENING_FLAGS "-fstack-protector;--param=ssp-buffer-size=4")
        CHECK_CXX_COMPILER_FLAG("${SCUMMTR_HARDENING_FLAGS}" COMPILER_SUPPORTS_HARDENING_FLAGS)
        if(COMPILER_SUPPORTS_HARDENING_FLAGS)
            add_compile_options("${SCUMMTR_HARDENING_FLAGS}")
            if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
                add_compile_options(-fPIE)
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
            endif()
        endif()
    endif()

    if (WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
    endif()
endif()

include_directories ("src")
add_subdirectory ("src/FontXY")
add_subdirectory ("src/ScummFont")
add_subdirectory ("src/ScummRp")
add_subdirectory ("src/ScummTr")
