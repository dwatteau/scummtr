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

for builder in linux86 win32 ; do
	if [ "$builder" = "linux86" ] && [ "$(uname -s)" = "Linux" ] && ! grep -q vsyscall /proc/self/maps ; then
		echo "WARNING: Dockerfile.$builder requires a Linux kernel with vsyscall=emulate" >&2
		echo "Without this, your build will likely fail with \"non-zero code: 139\" errors!" >&2
		sleep 5
	fi

	docker build --tag "scummtr-$builder:$VERSION" -f "Dockerfile.$builder" .
	docker run -v"$(pwd)/..:/scummtr/project" -v"$(pwd)/output:/scummtr/output" --rm "scummtr-$builder:$VERSION"
done
