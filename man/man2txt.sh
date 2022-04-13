#!/bin/sh
#
# Convert man pages to simple .txt files, for systems where man pages
# are not convenient (e.g. Windows).
#
# Requires: https://mandoc.bsd.lv/
#
set -eu

if ! command -v mandoc >/dev/null 2>&1 ; then
	echo "ERROR: The mandoc utility must be installed" >&2
	exit 1
fi

mkdir -p txt
for manpage in *.1 ; do
	mandoc -Tascii -I os=ScummTR "$manpage" | sed "s/\$/$(printf '\r')/; s/.$(printf '\b')//g;" > "txt/${manpage%.1}.txt"
done
