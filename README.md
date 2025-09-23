# Double Dash Deluxe

Double Dash Deluxe is a work-in-progress open-source (Expat License), cross-platform (GameCube, Wii, vWii, Dolphin Emulator) mod for MKDD aiming to provide a variety of features and improvements over the base game, including online gameplay.

## Disclaimer

This is in an early state and not yet useful for regular players. As such, binary releases are currently not provided.

## Building

You need:

- Python 3.11 or later
- pyelftools
- Ninja
- cargo
- LLVM 20

The used compiler is [NXP 'CodeWarrior Special Edition' for MPC55xx/MPC56xx v2.10](https://www.nxp.com/lgfiles/devsuites/PowerPC/CW55xx_v2_10_SE.exe). You can run `tools/prepare.sh` which will extract the relevant files from the installer with 7-Zip, or do the same manually. To use the compiler on non-Windows platforms, a Wine installation is needed.

If you already have the compiler installed, you can find the relevant files in `C:\Program Files (x86)\Freescale\CW for MPC55xx and MPC56xx 2.10` and manually move them.

- `./license.dat` -> `tools/cw/license.dat`
- `./bin/lmgr11.dll` -> `tools/cw/lmgr11.dll`
- `./PowerPC_EABI_Tools/Command_Line_Tools/mwcceppc.exe` -> `tools/cw/mwcceppc.exe`

You can then simply run `build.py` to build the project. The `out` directory will contain the generated binary.

## Running tests

You can launch `out/tests` to run all tests.

## Contributing

If you are working on something, please comment on the relevant issue (or open a new one if necessary).

For new code running on console, C++ and the `REPLACE`/`REPLACED` patching system should generally be used. Standard library and SDK patches can optionally be written in C, and for low-level code, assembly can sometimes be used.

The codebase is automatically formatted using `clang-format` (15), this will be checked by CI and must be run before merge.

## Links and resources

- [Discord guild](https://discord.gg/Ay3qffjcsE)

- Ghidra project: ask stebler on Discord for access

- [MKDD decompilation](https://github.com/SwareJonge/mkdd)

- [MKDD Wiki](https://mkdd.org/wiki/Main_Page)

## Credits

Double Dash Deluxe is based on [MKW-SP](https://github.com/mkw-sp/mkw-sp), see the list of contributors [here](https://github.com/mkw-sp/mkw-sp/graphs/contributors).
