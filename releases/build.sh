#!/bin/sh
set -eu

if ! command -v docker >/dev/null 2>&1 ; then
	echo "ERROR: \"docker\" command not found" >&2
	exit 1
fi

if [ $# -ne 1 ]; then
	echo "USAGE: $0 VERSION" >&2
	exit 1
fi
VERSION="$1"

export BUILDKIT_PROGRESS=plain

if [ ! -f releases/build.sh ]; then
	echo "ERROR: This script must be run from the root of the project" >&2
	exit 1
fi

for builder in linux86 msdos win32 ; do
	echo "===> Building for $builder"

	if [ "$builder" = "linux86" ] && [ "$(uname -s)" = "Linux" ] && ! grep -q vsyscall /proc/self/maps ; then
		echo "WARNING: Dockerfile.$builder requires a Linux kernel with vsyscall=emulate" >&2
		echo "Without this, your build will likely fail with \"non-zero code: 139\" errors!" >&2
		sleep 5
	fi

	docker build --tag "scummtr-$builder:$VERSION" -f "releases/Dockerfile.$builder" .
	docker run -it -v"$(pwd)/releases/output:/scummtr/output" --rm "scummtr-$builder:$VERSION"

	echo
done
