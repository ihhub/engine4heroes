# Nintendo Switch Port of [engine4heroes](README.md) Project

It's a homebrew app, so you need to be running custom firmware for it to work.

## Building

You will need to install the [devkitPro](https://devkitpro.org/) toolchain. Make sure to include the following package:

```text
libnx switch-sdl2_mixer
```

After installation run `make -f Makefile.switch -j 2` command to build the package.

## Setup

You will need a copy of the official game to run this port.

engine4heroes root directory is hardcoded as `/switch/engine4heroes`. Put the game files there (specifically `DATA` and `MAPS`
folders), then copy over the `engine4heroes.nro`.

At the end you should have the following directory tree on your SD card:

```text
switch
 |
 +-- ...
 +-- engine4heroes  <--- this is the game directory
     |
     +--- data              <--- HoMM4 game data
     +--- maps              <--- HoMM4 game data
     +--- engine4heroes.nro <--- Part of engine4heroes release
 ```

Generally, you will need game resources from the localized version of HoMM2 in order to use translations in engine4heroes. During
the first run, the game should auto-detect the game data you have and offer to choose a language you'd like to use. English
is always available.

## Running

This build of engine4heroes was tested on 12.0.3|AMS M.19.4|S (FAT32). exFAT is not recommended.
USB mice and keyboards connected via an OTG adapter are supported.

Working controls are:

* Touchscreen - emulates mouse, including dragging
* L-stick - move mouse cursor
* R-stick/D-pad - scroll
* A - left mouse click
* B - Right mouse click
* X - Escape
* Y - Enter
* (+) - Cast spell
* (-) - End turn
* R - Cycle through towns
* L - Cycle through heroes
