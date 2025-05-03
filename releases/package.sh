#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu

if ! command -v zip >/dev/null 2>&1 ; then
	echo "ERROR: \"zip\" command not found" >&2
	exit 1
fi

if [ $# -ne 1 ]; then
	echo "USAGE: $0 VERSION" >&2
	exit 1
fi
VERSION="$1"

if [ ! -f releases/package.sh ]; then
	echo "ERROR: This script must be run from the root of the project" >&2
	exit 1
fi

PKGPATH=releases/output-packages

_add_readme_file()
{
	local readme_path
	local target_name
	local target_description

	if [ $# -ne 3 ]; then
		echo "ERROR: Wrong number of arguments given to _add_readme_file" >&2
		exit 1
	fi

	readme_path="$1"
	target_name="$2"
	target_description="$3"

	cat > "${readme_path}.in" << EOF
scummtr-$VERSION for $target_description
@@HEADER_LINE@@

Fan translation tools for SCUMM engine games.

<https://github.com/dwatteau/scummtr>

This software is licensed under the MIT License; see the COPYING file.

Please read the FAQ if you have any problem:
<https://github.com/dwatteau/scummtr/blob/main/FAQ.md>

Bugs may be reported here, as long as you own a LEGAL copy of the game:
<https://github.com/dwatteau/scummtr/issues>

Built on $(LC_ALL=C LC_TIME=C date '+%Y-%m-%d') with the Dockerfile.$target_name build script.
EOF

	awk \
		'NR==1{l=$0; print; next} NR==2{for(i=1;i<=length(l);i++) printf "-"; print ""; next} 1' \
		< "${readme_path}.in" \
		> "$readme_path"

	case "$target_name" in
	*msdos*|*win32*)
		perl -pi -e "s/\$/$(printf '\r')/" "$readme_path"
		;;
	macos*)
		cat >> "$readme_path" << 'EOF'
IMPORTANT: On modern versions of macOS, you may need to do this:
$ xattr -d com.apple.quarantine scummtr scummrp scummfont FontXY

so that Gatekeeper lets you run the tools. See the FAQ for more details.
EOF
		;;
	esac

	rm -f -- "${readme_path}.in"
}

_pkg_it()
{
	local dir_path
	local target_name

	if [ $# -ne 2 ]; then
		echo "ERROR: Wrong number of arguments given to _pkg_it" >&2
		exit 1
	fi

	dir_path="${1%/}"
	target_name="$2"

	echo "==> Creating an archive for '$target_name' build..."

	find "$dir_path" -name .DS_Store -o -name '._*' -o -name '*.in' -o -name '*.*.swp' -print0 | xargs -0 rm -Rf

	case "$target_name" in
	*linux*)
		rm -f -- "${dir_path}.tar.gz"
		tar cvzf "${dir_path}.tar.gz" "$dir_path"
		;;
	*macos*|*msdos*|*win32*)
		rm -f -- "${dir_path}.zip"
		zip -r "${dir_path}.zip" "$dir_path"
		;;
	*)
		echo "WARNING: Using .zip for unknown '$target_name' target..." >&2
		rm -f -- "${dir_path}.zip"
		zip -r "${dir_path}.zip" "$dir_path"
		;;
	esac
}

install_linux86()
{
	DEST="$PKGPATH/scummtr-$VERSION-linux86"

	# Note: for now, I guess if you use Linux aarch64 you know how to
	# build things yourself...
	for arch in x86 x64 ; do
		mkdir -p -- "$DEST/linux-$arch"
		cp -p "releases/output/linux-$arch/usr/local/bin/"* "$DEST/linux-$arch/"
	done

	cp -p "releases/output/linux-x86/usr/local/share/doc/scummtr/"* "$DEST/"

	mkdir -p -- "$DEST/manuals/txt" "$DEST/manuals/man"
	cp -p releases/output/linux-x86/usr/local/share/man/man1/*.1 "$DEST/manuals/man/"
	cp -p man/txt/*.txt "$DEST/manuals/txt/"

	_add_readme_file "$DEST/README.txt" "linux86" "Linux (Intel x86-32 and x86-64)"

	_pkg_it "$DEST" "linux86"
}

install_win32()
{
	DEST="$PKGPATH/scummtr-$VERSION-win32"

	# Copy as-is
	mkdir -p -- "$DEST"
	cp -pR "releases/output/win32/"* "$DEST/"

	_add_readme_file "$DEST/README.txt" "win32" "Windows (Windows XP and above)"

	_pkg_it "$DEST" "win32"
}

install_msdos()
{
	DEST="$PKGPATH/scummtr-$VERSION-msdos"

	# Copy as-is
	mkdir -p -- "$DEST"
	cp -pR "releases/output/msdos/"* "$DEST/"

	_add_readme_file "$DEST/README.txt" "msdos" "DOS systems (32-bit binaries)"

	_pkg_it "$DEST" "msdos"
}

# TODO: maybe use install(1) with explicit permissions?

install_linux86
install_win32
install_msdos

# TODO: for macOS we depend on local builds... find a way to plug them in
