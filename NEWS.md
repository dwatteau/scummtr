## ScummTR 0.5.0 (2022-02-??)

### New features:

- MS-DOS binaries have been added, on top of the macOS, Linux and Windows binaries.
- Manual pages have been added (with .txt versions for systems where the original format is impractical).
- ScummTR: add a new `-r` option, which interprets the text of the game "as-is", in its original encoding. This can be useful for non-Latin languages such as Japanese.
- ScummTR: make it possible to import back a translation file created with the `-h` and/or `-I` options.
- ScummTR: add some encoding headers at the start of the translation file, so that most text editors will properly detect its encoding.
- ScummTR/ScummRP: make "XX.LFL should actually end at 0x1234" a warning which is always displayed, instead of being only printed in verbose mode.
- ScummFont: when importing a font, you now don't have to manually replace the original file with the `-new` file anymore.
- ScummFont: provide more details if an unsupported BMP file is given as an input.

### Bugfixes:

- ScummTR/ScummRP: fix a fatal "Duplicate offset in index" error with MONKEY1-FLOPPY-VGA.
- ScummTR/ScummRP: fix a fatal "Duplicate offset in index" error with LOOM-EGA-EN.
- ScummTR/ScummRP: don't fail when the DISK09.LEC file is missing for MONKEY1-EGA, since it was only available through the Roland Update.
- ScummTR/ScummRP: don't fail reporting missing files for the 4-disk floppy versions of MONKEY1, because only the 8-disk floppy versions need them.
- ScummFont: reject more incompatible BMP files, instead of trying to use them and then silently corrupting the internal SCUMM fonts.

### Incompatibilities with the previous version:

- ScummTR: a translation file created by an earlier version of ScummTR will not necessarily be compatible with this new version. It is recommended to make a backup, import (`-i`) your current translation with the old ScummTR, and extract (`-o`) it again with the new ScummTR.
- ScummTR: the default value for the `-f` option is now the `scummtr.txt` file, instead of `./text`.
- ScummTR: the `-h` and `-I` options are now ignored, when the `-b` option is used.
- ScummRP: the default value for the `-d` option is now the `DUMP` directory, instead of the current (`.`) directory.
- ScummTR/ScummRP: unrecognized options now produce an immediate fatal error, instead of being silently ignored.

### Known bugs:

- MANIAC-V2-EN is unsupported. This is because of an existing bug in the original game: any change made to the resources would amplify it and corrupt the game. Possible workaround: work from MANIAC-V1-EN or from a non-English version of Maniac Mansion V2.
- ScummTR: the `-b` option may not correctly work with all games. Possible workaround: use the `-r` option.

### Portability (for developers):

- Big-endian systems are now completely supported.
- Drop support for MSVC versions lower than 2015, because of their improper support for safe constructs such as `snprintf()`.

## ScummTR 0.4.2 (2020-11-28)

### New features:

- The original sources have been modified to be compatible with most modern systems.
- New binaries for Mac Intel systems.
- 64-bit binaries for Linux are now available too.

### Bugfixes:

- None.

### Portability (for developers):

- Compatibility fixes for modern compilers, and CMake configuration. C++98 is still used by default.
- Fix a possible incompatibility with some strict-alignment systems.
- Fix fatal 64-bit errors on non-Windows systems.

## ScummTR 0.4.1 (2020-11-08)

### New features:

- The ScummTR tools are now open-source (using the MIT License), thanks to Thomas Combeleran, their original author.
- The original sources have been recompiled "as-is" on circa-2003 Intel 32-bit systems. That's it.

### Bugfixes:

- None.

### Incompatibilities with the previous version:

- None.

## ScummTR 0.4.0 (2003-10-12)

Until 2020, the ScummTR tools were only available as win32 binaries, unmodified since their original upload.

They're still available from their original server (with the associated SHA256 sums), as of 2022:

* <http://hibernatus34.free.fr/scumm/scummtr.exe> (`0b96ae59e889908cb806988a0b8b54c6d5c98b62b86779975889cfbb7d0abe65`)
* <http://hibernatus34.free.fr/scumm/scummrp.exe> (`cc0ef22670715eaedc85682ce8641c10b761d7b3960809464a4f2a98113c2797`)
* <http://hibernatus34.free.fr/scumm/scummfont.exe> (`ef86343d0c152b19102c7defa46a44f1476639464929b3bcb2a4c1e558278d3c`)
* <http://hibernatus34.free.fr/scumm/FontXY.exe> (`5e162b3c018dad36eab799a7384869fbd9591e0db136ec1e8df90f0e39915897`)
