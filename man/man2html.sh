#!/bin/sh
#
# Convert man pages to HTML, for systems where man pages are not
# convenient.
#
# Requires: https://mandoc.bsd.lv/
#
set -eu

if ! command -v mandoc >/dev/null 2>&1 ; then
	echo "ERROR: The mandoc utility must be installed" >&2
	exit 1
fi

mkdir -p html
for manpage in *.1 ; do
	mandoc -Thtml -I os=mandoc "$manpage" > "html/${manpage%.1}.html"
done
