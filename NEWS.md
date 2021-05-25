## ScummTR 0.5 (2021-XX-XX)

New features:

- The ScummTR tools are now open-source (using the MIT License), thanks to Thomas Combeleran, their original author.
- macOS and Linux binaries are now available too. The new Windows binaries are compatible with Windows XP and later systems.
- Manual pages are now available, with HTML versions for systems where the original format is impractical.
- ScummTr/ScummRP: make "XX.LFL should actually end at 0x1234" a warning which is always displayed, instead of being only printed in verbose mode.
- ScummTR: make it possible to import back a translation file created with the `-h` and/or `-I` options.
- ScummTR: add some encoding headers at the start of the translation file, so that most text editors will properly detect its encoding.
- ScummFont: when importing a font, you now don't need to replace the original file with the `-new` file anymore.
- ScummFont: provide more details if an unsupported BMP file is used.

Bugfixes:

- ScummTr/ScummRP: fix a fatal "Duplicate offset in index" error with MONKEY1-FLOPPY-VGA.
- ScummTr/ScummRP: fix a fatal "Duplicate offset in index" error with LOOM-EGA-EN.
- ScummFont: reject more cases of incompatible BMP files, instead of silently corrupting the internal SCUMM fonts.

Incompatibilities with the previous version:

- ScummTR: a translation file created by an earlier version of ScummTR will not necessarily be compatible with this new version. It is recommended to make a backup, import (`-i`) your current translation with the old ScummTR, and extract (`-o`) it again with the new ScummTR.
- ScummTR: the default value for the `-f` option is now the `scummtr.txt` file, instead of `./text`.
- ScummTR: ignore the `-h` and `-I` options, when the `-b` option is used.
- ScummRP: the default value for the `-d` option is now the `DUMP` directory, instead of the current (`.`) directory.

Known bugs:

- ScummTR: the `-b` option may not correctly work with all games.

Portability (for developers):

- Compatibility fixes for modern compilers. C++98 is still used by default.
- Big-endian systems are now completely supported.
- Fix a possible incompatibility with some strict-alignment systems.
- Fix some 64-bit errors on non-Windows systems.
- Drop support for MSVC versions lower than 2015, because of their improper support for safe constructs such as `snprintf()`. Use MinGW for older systems.
