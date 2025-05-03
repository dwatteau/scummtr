#!/bin/sh
#
# Convert man pages to simple .txt files, for systems where man pages
# are not convenient (e.g. Windows).
#
# (Also tries to warn of '$Mdocdate' fields needing an update.)
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
	mandoc -Tascii -I os=ScummTR "$manpage" | sed "s/.$(printf '\b')//g;" > "txt/${manpage%.1}.txt"
done

if command -v git >/dev/null 2>&1 ; then
	modified_files=$(git status -s -uno -- ./txt/ | awk '/\.txt$/ { print $2 }')
	if [ ! -z "$modified_files" ]; then
		current_date1=$(LC_ALL=C LC_TIME=C date '+%B %d, %Y')
		current_date2=$(LC_ALL=C LC_TIME=C date '+%B %e, %Y')
		for file in $modified_files ; do
			if ! grep -qF "$current_date1" -- "$file" &&
			   ! grep -qF "$current_date2" -- "$file" ; then
				echo "WARNING: Possible missing \$Mdocdate update for: $file" >&2
			fi
		done
	fi
fi
