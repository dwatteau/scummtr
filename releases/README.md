# Building ScummTR releases

Here are some reproducible and portable-enough environments to build ScummTR releases.

This lets you do local release builds, with no dependency on Github Actions (which is currently just used for quick building tests).

## Supported environments

### Windows x86-32 (Win32)

`Dockerfile.win32` builds x86-32 static binaries compatible with Windows XP and above.

Out of simplicity for users, no 64-bit binaries are built, since the benefit would be low, and Win32 binaries are more portable, especially since Windows is really good at backward compatibility.

Although the ScummTR tools can be built with a modern version of Microsoft Visual C++ (MSVC), we use an older release of [Mingw-W64](http://mingw-w64.org) for the official releases, in order to make sure Windows XP support remains OK. For the same reason, we don't enable too “modern” compiler settings on this environment.

Live debug is not expected to happen with Windows release builds, so the binaries are built in `Release` mode.

### MS-DOS (DJGPP)

`Dockerfile.msdos` builds MS-DOS binaries thanks to [DJGPP](http://www.delorie.com/djgpp/).

Live debug is not expected to happen here either, so `Release` mode is used too.

Note that these binaries require a DOS Protected Mode Interface (DPMI) to properly run.

### Linux x86-32/x86-64

`Dockerfile.linux86` builds x86-32 and x86-64 dynamic binaries for Linux.

We compile against an older CentOS release, with an older version of Glibc, to provide reasonable support for most Linux distributions released since 2012.

Since Linux is a good environment for live debugging, some hardening features are enabled, mostly to help catching potential bugs. For the same reason, our default Linux binaries are built in `ReleaseWithDebug` mode.

### macOS

Portable macOS build environements are harder to provide, so you need a local macOS setup.

macOS 10.9+ builds for x86\_64 and arm64 require `cmake` (for example with [MacPorts](https://www.macports.org/install.php) or [Homebrew](https://brew.sh)), macOS 10.15.4+ and Xcode 12.2+.

```sh
rm -rf build && mkdir build && cd build
cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9 -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
make -j2
```

Mac OS X 10.4–10.8 builds for ppc32 and i386 are done on a Mac OS X 10.5 system (Xcode 3.1.4 and [CMake 3.3.2](https://cmake.org/files/v3.3/cmake-3.3.2-Darwin-universal.dmg) must be installed), although building on Mac OS X 10.4 should work, too:

```sh
rm -rf build && mkdir build && cd build
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_CXX_FLAGS="-Wa,-force_cpusubtype_ALL" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.4 -DCMAKE_OSX_ARCHITECTURES="ppc;i386" -DCOMPILER_SUPPORTS_HARDENING_FLAGS=0 ..
make
```

### Other environements

On other environments, you are currently expected to try building ScummTR by yourself. For speed reasons, GCC is currently recommended over Clang or MSVC ([issue #4](https://github.com/dwatteau/scummtr/issues/4)).

Older compiler versions should be pretty well supported if necessary, since ScummTR was written in C++98. For example, g++ 3.3 (on OpenBSD) and g++ 2.95.3 (on Haiku) have been tested.

MSVC builds require MSVC 2015 or later.

## Building

First, you need [to install Docker](https://docs.docker.com/get-docker/). Linux users will also probably need a kernel with a `vsyscall=emulate` boot parameter.

Then, from the root of the project, and with `0.0.1` as a version number example, run:

```sh
./releases/build.sh 0.0.1
```

Binaries will then appear in the `releases/output/` directory.
