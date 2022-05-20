# ScummTR – Fan translation tools for SCUMM engine games

This project hosts the sources of Thomas Combeleran's fan translation tools for most LucasArts [SCUMM engine](https://en.wikipedia.org/wiki/SCUMM) games.

They are known as `scummtr`, `scummrp`, `scummfont`, and `FontXY`. They were originally only available as Windows programs, but they're now open-source and available for various systems.

## Download

Downloads for various systems are available [on the Releases page](https://github.com/dwatteau/scummtr/releases). It is recommended to always use the latest version.

Note that you need to know how to use [a command-line interface](https://en.wikipedia.org/wiki/Command-line_interface) in order to use ScummTR, though.

## How to use

[The FAQ](FAQ.md) is here to answer the most usual questions. Please read it first if you have any problem.

Some [manual pages](man/txt/) are also available.

If you have any problem or question, feel free to [open a discussion](https://github.com/dwatteau/scummtr/discussions/new) or [report an issue](https://github.com/dwatteau/scummtr/issues/new/choose)!

## Compiling

If you'd like to compile ScummTR yourself, you will need [CMake](https://cmake.org) and a C++98 compatible compiler. It should build on Windows and most POSIX systems.

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

See also [how releases are made](releases/README.md).

## Current goals

The current purpose of this repository is to host the original sources of the ScummTR tools, and provide a simple way of using them on most platforms.

My C++ skills are very modest, and the tools are mostly “done”, so for the moment no real evolution is planned, except for compilation fixes and portability improvements.

However, it is known that the tools currently don't support some game variants, such as Maniac Mansion NES, or Loom TurboGrafx-16. They may become compatible in a future release.

For later titles such as *Curse of Monkey Island* or the Humongous Entertainment games, have a look at [NUTCracker](https://github.com/BLooperZ/nutcracker), instead.

## License

This project has been [licensed under the MIT License](COPYING) in 2020 by its original author, Thomas Combeleran.

## Alternatives

[NUTCracker](https://github.com/BLooperZ/nutcracker) is a more general set of tools, with an emphasis on later SCUMM games (v5-v8, HE), while ScummTR mainly targets translations and earlier LucasArts SCUMM games.

[ScummSpeaks](http://www.jestarjokin.net/apps/scummspeaks/) uses ScummTR in order “to assist in adding or replacing speech” to the later SCUMM games. See also [ScummPacker](http://www.jestarjokin.net/apps/scummpacker/), [Scummbler](http://www.jestarjokin.net/apps/scummbler/) and [ScummImg](http://www.jestarjokin.net/apps/scummimg/) from the same author.

[Quickandeasysoftware](https://quickandeasysoftware.net/software) has also been providing extremely nice tools, for years.

## Disclaimer

This project is not affiliated in any way with LucasArts Entertainment Company, LucasFilm Ltd, or Humongous Entertainment.

This project is made out of love and respect for the old LucasArts adventure games. LucasArts and Humongous Entertainment games can be bought on wonderful places such as [GOG](https://www.gog.com/games?devpub=lucasfilm&page=1&sort=title).

This project doesn't contain and doesn't distribute any kind of game data. This project asks all of its users to respect the games, Intellectual Property and Copyright laws.

The purpose of this project is just to help fans make their own fan translations of the games. Fan translations **MUST NOT** be distributed as a whole game, or with official game data. Fan translations can only be shared through scripts, instructions or patches, and must respect Copyright laws.
