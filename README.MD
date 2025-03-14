# Command & Conquer Generals NextGen Fork

## About this specific fork
This fork is an effort to replace GameSpy with modern multiplayer game services provided by Steam and Epic Games.

- Current status: Heavily WIP. Once the golden path works, a more fine grain list of what is implemented / not implemented will be provided.

- Known restrictions:
  Only the Steam version of the game is supported

- Build status:
[![Build All](https://github.com/x64-dev/CnC_Generals_Zero_Hour_NextGen_Multiplayer/actions/workflows/build_all.yml/badge.svg)](https://github.com/x64-dev/CnC_Generals_Zero_Hour_NextGen_Multiplayer/actions/workflows/build_all.yml)

## Contributors
- jmarshall23
- Andrew-2E128
- Disarray(x64 support great work!).

## New Features
- Quake style console(open/close via the ` key)
- D3D12 support
- MSAA 4x support
- Widescreen support
- Visual Studio 2022 support
- Memory crash fixes
- ImGui support
- x64 support

---

## Compiling with Visual Studio 2022
1. Make sure **MFC** is installed.
2. Either build the **RTS** or **WorldBuild** project – doing so will build everything you need.

---

## (Optional) Compiling the 3DS Max Plugin
- Requires a copy of the **3DS Max 4.x SDK**, placed in `Code\Libraries\max4sdk`.
- The SDK is available from `maxsdk.zip` on the 3DS Max CD.

*(Note: The Max plugin is not required to run the game.)*

---

## Running the Game
- The game now supports loading **loose files** for easier modding.
- To distribute or ship your mod, package your loose files into a `.big` file.
- If you'd like to unpack existing `.big` files, download a **big extractor** and extract all `.big` files into the game folder *(extract patch files last)*.
- After extraction, you can safely delete the original `.big` files.

Enjoy modding and playing Command & Conquer Generals NextGen Fork!
