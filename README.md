-----------------------------------

_DISCLAIMER:_

Welcome to the **raylib gamejam template**!

This template provides a base structure to start developing a small raylib game in plain C for any of the proposed **raylib gamejams**!

Please, considering the following usual gamejam restrictions: 

 - Game must be made with raylib
 - Game must be compiled for web
 - _Specific gamejam restrictions if defined_
 
NOTE: Several GitHub Actions workflows have been preconfigured to automatically build your game for Windows, Linux and WebAssembly on each commit. Those workflows automatically sync with latest version of raylib available to build.

The repo is also pre-configured with a default `LICENSE` (zlib/libpng) and a `README.md` (this one) to be properly filled by users. Feel free to change the LICENSE as required.

All the sections defined by `$(Data to Fill)` are expected to be edited and filled properly. It's recommended to delete this disclaimer message after editing this `README.md` file.

_GETTING STARTED:_

First press the 'Use this template' button to clone the repo.

This template does not include a copy of raylib itself. By default it expects to find raylib in the same folder as the one to which you've cloned this template. To start using this template with raylib 5.0, you can do the following:

```sh
mkdir ~/raylib-gamejam && cd ~/raylib-gamejam
git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib
make -C raylib/src
git clone https://github.com/$(User Name)/$(Repo Name).git
cd $(Repo Name)
make -C src
src/raylib_game
```

This template has been created to be used with raylib (www.raylib.com) and it's licensed under an unmodified zlib/libpng license.

_Copyright (c) 2022-2024 Ramon Santamaria ([@raysan5](https://twitter.com/raysan5))_

-----------------------------------

## NeuroCircuit

![NeuroCircuit](img/banner.png "NeuroCircuit")

_Entry for raylib NEXT gamejam, theme: **connections**_

### Description

**NeuroCircuit** is a simple puzzle-game, where you need to connect Nodes.

Link controls and actions so that and use your connected controls to move the character and exit the level.

### Features

 - customize your key-binds in a new way
 - chain actions
 - move the character with your customized key-binds
 - ~~collect key to open door~~
 - ~~walls and obstetrical in the node selection~~
 - 4+ Levels
 - **Rules**
   - [x] Only connect Action with Keys or Action with other Actions
   - [x] Crossing (connection) lines with other lines not allowed
   - [x] Crossing (connection) lines with nodes not allowed
   - [x] Action can only connect with one Key
   - [x] Action can only have one direct connection with one Key
 - show more help in-game
   - L-Button: show preview lines
   - ?-Button: show Nodes help
 - ~~timer and score~~
 - ~~animation, jump animation~~

I didn't finish all the features in time, but at least it works :)

### Controls

Keyboard/Mouse:
 - click on one Node to select an Action/Key and click on another Node (Action) to connect it
   - left-click to connect nodes
   - right-click to remove connections
 - use the connected key-binds to move the character, possible keys:
   - H
   - J
   - K
   - L
   - B
   - G

#### How to play

1. link Nodes for key binding
2. press GO when ready
3. move Character
4. reach door without falling into the void
   1. press RESET to reconnect Nodes

_You can't connect Nodes while moving your character_

### Screenshots

![start-screen](screenshots/screenshot000.png "start screen")
![level-2](screenshots/screenshot001.png "level 2")
![level-3](screenshots/screenshot002.png "level 3")
![level-1](screenshots/screenrec001.gif "level 1")

### Developers

 - [@furudbat](https://twitter.com/furudbat) - Developer
 - [@blacktiger5](https://twitter.com/blacktiger5_) - Art, Level Design

### Links

 - itch.io Release: $(itch.io Game Page)

### Build

Made for Desktop and Web.

#### Configure

C++-Compiler (min. C++20) and cmake required.

```bash
cmake -G Ninja -S . -B build
```

#### Configure for Web

emscripten and cmake needed, see [Install emscripten toolchain](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)#1-install-emscripten-toolchain).

```bash
cmake -DCMAKE_C_COMPILER=emcc -DCMAKE_CXX_COMPILER=em++ -DPLATFORM=Web -DCMAKE_BUILD_TYPE=MinSizeRel -G Ninja -S build-web
```
_(tested on arch linux with [emscripten](https://archlinux.org/packages/extra/x86_64/emscripten/) installed)_

#### Build

```bash
cmake --build build-web --target raylib_game
```

### License

This project sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

*Copyright (c) 2024 furudbat ([@furudbat](https://twitter.com/furudbat))*

#### Links

- https://github.com/githubnext/monaspace
- https://itch.io/jam/raylib-next-gamejam
