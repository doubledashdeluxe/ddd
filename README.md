# Double Dash Deluxe

Double Dash Deluxe is a work-in-progress open-source (Expat License), cross-platform (Wii, vWii, Dolphin Emulator) mod for MKDD aiming to provide a variety of features and improvements over the base game, including online gameplay.

## Disclaimers

This is in an early state and not yet useful for regular players. As such, binary releases are currently not provided.

Only RVL-001 consoles are currently supported.

## Building

You need:

- Python 3.10 or later
- pyelftools
- Ninja
- lld and clang-format
- GCC and binutils

The used compiler is [NXP 'CodeWarrior Special Edition' for MPC55xx/MPC56xx v2.10](https://www.nxp.com/lgfiles/devsuites/PowerPC/CW55xx_v2_10_SE.exe). You can run `tools/prepare.sh` which will extract the relevant files from the installer with 7-Zip, or do the same manually. To use the compiler on non-Windows platforms, a Wine installation is needed.

You can then simply run `build.py` to build the project. The `out` directory will contain the generated binary.

## Running tests

You can launch `test.py` to build and run all tests.

## Contributing

If you are working on something, please comment on the relevant issue (or open a new one if necessary).

For new code running on the Wii, C++ and the `REPLACE`/`REPLACED` patching system should generally be used. Standard library and SDK patches can optionally be written in C, and for low-level code, assembly can sometimes be used.

The codebase is automatically formatted using `clang-format` (15), this will be checked by CI and must be run before merge.

## Links and resources

- [Discord guild](https://discord.gg/Ay3qffjcsE)

- Ghidra project: ask stebler on Discord for access

- [MKDD decompilation](https://github.com/SwareJonge/mkdd)

- [MKDD Wiki](https://mkdd.org/wiki/Main_Page)

## Credits

Double Dash Deluxe is based on [MKW-SP](https://github.com/mkw-sp/mkw-sp), see the list of contributors [here](https://github.com/mkw-sp/mkw-sp/graphs/contributors).
