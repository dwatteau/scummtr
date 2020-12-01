# Building ScummTR for release

Here are some reproducible and portable-enough environments to build ScummTR releases.

This lets you do local release builds, with no dependency on Travis or Github Actions (although we might use them at some point).

## Supported environments

### Windows x86-32 (Win32)

`Dockerfile.win32` builds x86-32 static binaries compatible with Windows XP and above.

Out of simplicity for users, no 64-bit binaries are built, since the benefit would be low, and Win32 binaries are more portable, especially since Windows is really good at backward compatibility.

We use an older release of [Mingw-W64](http://mingw-w64.org) for this, to make sure Windows XP support remains OK. For the same reason, we don't enable too "modern" compiler settings on this environment.

Live debug is not expected to happen with Windows release builds, so our default Win32 binaries are built in `Release` mode.

### Linux x86-32/x86-64

`Dockerfile.linux86` builds x86-32 and x86-64 dynamic binaries for Linux.

We compile against an older CentOS release, with an older version of Glibc, to provide reasonable support for most Linux distributions released since 2012.

Since Linux is a good default "live debug" environment, some hardening features are enabled, mostly to help catching potential bugs. For the same reason, our default Linux binaries are built in `ReleaseWithDebug` mode.

### macOS

Nothing is available yet, since macOS build automation is a bit harder to provide.

Building it yourself from source should be fine though, as this environment is frequently tested during development.

### Other environements

On other environments, you are currently expected to build ScummTR from source. For speed reasons, GCC is currently recommended over Clang or MSVC (issue #4).

Older compiler versions should be pretty well supported, if necessary. For example, g++ 3.3 (OpenBSD) and g++ 2.95.3 (BeOS) have been tested.

Big-endian systems still have some issues at the moment (issue #3).

## Building

First, you need [Docker on your environment](https://docs.docker.com/get-docker/).

Then (with `0.0.1` as a version number example):

```sh
cd releases
./build.sh 0.0.1
```

Binaries will then appear in the `releases/output/` directory.
