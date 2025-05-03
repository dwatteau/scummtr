#!/bin/sh
set -eu

if [ ! -f releases/clean.sh ]; then
	echo "ERROR: This script must be run from the root of the project" >&2
	exit 1
fi

find releases/output -mindepth 2 \! -name '.gitignore' -print0 | xargs -0 rm -Rf
