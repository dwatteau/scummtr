# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.3...3.10 FATAL_ERROR)

# Don't hardcode /W3 by default for MSVC
if(NOT CMAKE_VERSION VERSION_LESS 3.15)
    if(POLICY CMP0092)
        cmake_policy(SET CMP0092 NEW)
    endif()
endif()

# For CheckIPOSupported, in CMake 3.9+
if(NOT CMAKE_VERSION VERSION_LESS 3.9)
    if(POLICY CMP0069)
        cmake_policy(SET CMP0069 NEW)
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No explicit CMAKE_BUILD_TYPE selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_LTO "Enable Link Time Optimization, if available" ON)
option(SCUMMTR_COMPILER_IS_DJGPP "Compiler is DJGPP" OFF)
if(SCUMMTR_COMPILER_IS_DJGPP)
    set(CMAKE_SYSTEM_NAME "Generic")
endif()

project("ScummTR" CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

if(SCUMMTR_COMPILER_IS_DJGPP)
    set(CMAKE_CXX_EXTENSIONS ON) # for mkdir()
    set(CMAKE_EXECUTABLE_SUFFIX ".exe")
endif()

string(TIMESTAMP SCUMMTR_BUILD_DATE "%Y%m%d-WIP%H%M" UTC)

include(CheckCXXCompilerFlag)

if(ENABLE_LTO)
    if(CMAKE_VERSION VERSION_LESS 3.9)
        message(STATUS "LTO support: disabled (CMake 3.9 or later is required)")
    else()
        include(CheckIPOSupported)
        check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_CHECK_OUTPUT)

        if(LTO_SUPPORTED)
            message(STATUS "LTO support: enabled")
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(STATUS "LTO support: disabled (reason: ${LTO_CHECK_OUTPUT})")
        endif()
    endif()
endif()

if(MSVC)
    if(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Refusing to build: Visual Studio 2015 or better is required")
    endif()

    if (MSVC_VERSION GREATER 1913)
        add_compile_options(/Zc:__cplusplus)
    endif()

    add_definitions(/DSCUMMTR_BUILD_DATE=\"${SCUMMTR_BUILD_DATE}\")
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN)

    add_compile_options(/W3 /wd4244 /wd4267)
else()
    add_definitions(-DSCUMMTR_BUILD_DATE=\"${SCUMMTR_BUILD_DATE}\")
    if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
    endif()

    # GCC 2.95 compatible warnings we always want
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wundef -Wmultichar -Wwrite-strings -Wredundant-decls -Wunused -Wsign-compare)
    # GCC 2.95 compatible warnings we don't want (yet)
    add_compile_options(-Wno-non-virtual-dtor -Wno-conversion -Wno-overloaded-virtual -Wno-old-style-cast -Wno-format)

    # Flags which are compiler-specific, or only meaningful with a particular compiler
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        # TODO: add -Wshorten-64-to-32 back, and fix the issues for good
        add_compile_options(-Widiomatic-parentheses -Wparentheses -Wimplicit-int-conversion -Wno-shorten-64-to-32)
    else()
        message(WARNING "Assuming that your ${CMAKE_CXX_COMPILER_ID} compiler is GCC-compatible, but this may fail")
    endif()

    # -O3 has no benefit, and it's sometimes buggy
    string(REPLACE "-O3" "-O2" "CMAKE_CXX_FLAGS_RELEASE" "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "-O3" "-O2" "CMAKE_CXX_FLAGS_RELWITHDEBINFO" "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    # Warnings requiring a recent compiler. Do a single test, it's not critical
    set(SCUMMTR_MODERN_WARNING_FLAGS "-Winit-self;-Wtype-limits;-Wignored-qualifiers") # -Wzero-as-null-pointer-constant for C++11
    CHECK_CXX_COMPILER_FLAG("${SCUMMTR_MODERN_WARNING_FLAGS}" COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
    if(COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
        # Also add some old flags which only became reliable in more recent versions
        add_compile_options("${SCUMMTR_MODERN_WARNING_FLAGS};-Wshadow;-Wuninitialized;-Wctor-dtor-privacy")
    endif()

    if(MINGW)
        add_definitions(-D__USE_MINGW_ANSI_STDIO)
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "OpenBSD")
        # OpenBSD just does its hardening all by itself. However, we need -Wno-system-headers, there
        add_compile_options(-Wno-system-headers)
    elseif(NOT SCUMMTR_COMPILER_IS_DJGPP)
        add_definitions(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS)
        set(SCUMMTR_HARDENING_FLAGS "-fstack-protector;--param=ssp-buffer-size=4")
        CHECK_CXX_COMPILER_FLAG("${SCUMMTR_HARDENING_FLAGS}" COMPILER_SUPPORTS_HARDENING_FLAGS)
        if(COMPILER_SUPPORTS_HARDENING_FLAGS)
            add_compile_options("${SCUMMTR_HARDENING_FLAGS}")
            if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
                add_compile_options(-fPIE)
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
            endif()
        endif()
    endif()
endif()

if(NOT UNIX)
    set(CMAKE_INSTALL_BINDIR ".")
    set(CMAKE_INSTALL_DATADIR ".")
endif()
include(GNUInstallDirs)

include_directories("src")
add_subdirectory("src/common")
add_subdirectory("src/FontXY")
add_subdirectory("src/ScummFont")
add_subdirectory("src/ScummRp")
add_subdirectory("src/ScummTr")

if(UNIX)
    install(FILES man/FontXY.1 man/scummfont.1 man/scummrp.1 man/scummtr.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
else()
    install(DIRECTORY man/txt DESTINATION "${CMAKE_INSTALL_DATADIR}" FILES_MATCHING PATTERN "*.txt")
endif()
