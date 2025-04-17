# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.3...3.10 FATAL_ERROR)

# Don't hardcode /W3 in default MSVC settings -- we'll add it ourselves if we want it
if(NOT CMAKE_VERSION VERSION_LESS 3.15)
    if(POLICY CMP0092)
        cmake_policy(SET CMP0092 NEW)
    endif()
endif()

# LTO support through CheckIPOSupported -- requires CMake 3.9+
if(NOT CMAKE_VERSION VERSION_LESS 3.9)
    if(POLICY CMP0069)
        cmake_policy(SET CMP0069 NEW)
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No explicit CMAKE_BUILD_TYPE selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type: Debug, Release, MinSizeRel, RelWithDebInfo (default)" FORCE)
endif()

option(SCUMMTR_COMPILER_IS_DJGPP "Compiler is DJGPP" OFF)
if(SCUMMTR_COMPILER_IS_DJGPP)
    set(CMAKE_SYSTEM_NAME "Generic")
endif()

if(NOT DEFINED ENABLE_LTO)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Exclude DJGPP because of: https://gitlab.kitware.com/cmake/cmake/-/issues/21696
        if(NOT SCUMMTR_COMPILER_IS_DJGPP)
            set(_ENABLE_LTO_DEFAULT ON)
        endif()
    else()
        set(_ENABLE_LTO_DEFAULT OFF)
    endif()
endif()
option(ENABLE_LTO "Enable Link Time Optimization, if available" ${_ENABLE_LTO_DEFAULT})

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
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "LTO support with CMAKE_BUILD_TYPE set to \"Debug\" makes no sense: disable it, or use RelWithDebInfo")
    elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        message(WARNING "LTO support was asked, but using without CMAKE_BUILD_TYPE set to \"Release\" is not optimal")
    endif()

    if(CMAKE_VERSION VERSION_LESS 3.9)
        message(WARNING "LTO support: not available (CMake 3.9 or later is required)")
    else()
        include(CheckIPOSupported)
        check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_CHECK_OUTPUT)

        if(LTO_SUPPORTED)
            message(STATUS "LTO support: enabled")
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(WARNING "LTO support: not available (CMake was unable to set it up on your system)")
        endif()
    endif()
else()
    message(STATUS "LTO support: disabled (set ENABLE_LTO or Release mode if you want it)")
endif()

if(MSVC)
    if(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Refusing to build: Visual Studio 2015 or better is required")
    endif()

    if(MSVC_VERSION GREATER 1913)
        # Don't lie about your `__cplusplus` value, MSVC...
        #
        # (Note: this means that some releases of Visual Studio 2017 (and earlier) will
        # continue lying, since they don't have this flag... so their support is just
        # best effort.)
        add_compile_options(/Zc:__cplusplus)
    endif()

    if(MSVC_VERSION GREATER 1919)
        # Print better diagnosis, like modern GCC/Clang do
        # XXX: it may have been added earlier, but good luck finding any info about this
       add_compile_options(/diagnostics:caret)
    endif()

    # Looks like CMake doesn't set up all the proper LTO flags for MSVC...
    # e.g. "Not all modules are compiled with -Gy"...
    # https://discourse.cmake.org/t/cmake-ipo-issue-msvc-ltcg-incremental/826/2
    # https://gitlab.kitware.com/cmake/cmake/-/issues/20484
    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        add_compile_options(/GL /Gy)
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
        endif()
    endif()

    add_definitions(/DSCUMMTR_BUILD_DATE=\"${SCUMMTR_BUILD_DATE}\")
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN)

    add_compile_options(/W3)

    # Some /W4 flags we bring to /W3 -- courtesy of Wireshark
    # /w34295 (no equivalent):             array is too small to include a terminating null character
    # /w34100 (like -Wunused-parameter):   unreferenced formal parameter
    # /w34189 (like -Wunused-variable):    local variable is initialized but not referenced
    # TODO: anything else?
    add_compile_options(/w34295 /w34100 /w34189)

    # /wd4244 (like -Wno-conversion):      conversion from 'type1' to 'type2', possible loss of data
    # /wd4267 (like -Wno-sign-conversion): conversion from 'size_t' to 'type', possible loss of data
    # /wd4068 (like -Wno-unknown-pragmas): ignore unknown `#pragma` directives
    add_compile_options(/wd4244 /wd4267 /wd4068)

else()

    #
    # Here, we assume the compiler is GCC-compatible.
    #

    add_definitions(-DSCUMMTR_BUILD_DATE=\"${SCUMMTR_BUILD_DATE}\")
    if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN)
    endif()

    # GCC 2.95 compatible warnings we always want
    add_compile_options(-Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wundef -Wmultichar -Wwrite-strings -Wredundant-decls -Wunused -Wsign-compare -Wformat)
    # GCC 2.95 compatible warnings we don't want, for very good reasons
    add_compile_options(-Wno-unknown-pragmas)
    # GCC 2.95 compatible warnings we don't want to fix... yet
    add_compile_options(-Wno-conversion -Wno-overloaded-virtual -Wno-old-style-cast)
    # GCC 2.95 compatible warnings we don't want to fix but REALLY SHOULD
    # TODO: it looks like this one now doesn't find anything anymore?
    add_compile_options(-Wno-non-virtual-dtor)

    # Flags which are compiler-specific, or only meaningful with a particular compiler
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fcheck-new)
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options(-Widiomatic-parentheses -Wparentheses -Wimplicit-int-conversion)
        # TODO: fix this, instead of silencing
        add_compile_options(-Wno-shorten-64-to-32)
    else()
        message(WARNING "Assuming that your ${CMAKE_CXX_COMPILER_ID} compiler is GCC-compatible, but this may fail")
    endif()

    # -O3 has no benefit for us, and it's sometimes buggy
    set(default_fallback_opt_flag "-O2")
    foreach(cfg Release RelWithDebInfo MinSizeRel Debug)
        string(TOUPPER "${cfg}" UCFG)
        string(REPLACE "-O3" "${default_fallback_opt_flag}" "CMAKE_CXX_FLAGS_${UCFG}" "${CMAKE_CXX_FLAGS_${UCFG}}")
    endforeach()

    # Warnings requiring a recent compiler. Do a single test, it's not critical
    set(SCUMMTR_MODERN_WARNING_FLAGS "-Wformat=2;-Winit-self;-Wtype-limits;-Wignored-qualifiers") # -Wzero-as-null-pointer-constant for C++11
    check_cxx_compiler_flag("${SCUMMTR_MODERN_WARNING_FLAGS}" COMPILER_SUPPORTS_MODERN_WARNING_FLAGS)
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
        check_cxx_compiler_flag("${SCUMMTR_HARDENING_FLAGS}" COMPILER_SUPPORTS_HARDENING_FLAGS)
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
