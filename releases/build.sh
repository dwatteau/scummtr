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
	docker build --tag "scummtr-$builder:$VERSION" -f "Dockerfile.$builder" .
	docker run -v"$(pwd)/..:/scummtr/project" -v"$(pwd)/output:/scummtr/output" --rm "scummtr-$builder:$VERSION"
done
