# Chip-8 Emulator

Play online at http://ajor.co.uk/chip8

## Native Build

This code has only been tested on Linux, although it may work on other platforms provided the required dependencies are available.

### Requirements
- A C++11 compiler
- [GLFW3](http://www.glfw.org/)
- GLEW (OpenGL Extension Wrangler)

### Compilation
Then compile using CMake:
```
mkdir build
cd build
cmake ../
make
```

or just run:
```
g++ main.cpp chip8.cpp font_loader.cpp -std=c++11 -lglfw -lGLEW -lGL -lGLU -pthread -O3 -Wall -pedantic
```

## Emscripten/asm.js Build

### Requirements
- [Emscripten SDK](https://kripken.github.io/emscripten-site/)
- Node.js

### Compilation
Compile using CMake, substituting the path to the Emscripten.cmake toolchain file on your system:
```
mkdir build
cd build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=/usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release
make
```

## Usage

### Native

    ./chip8 [options] rom
    Options:
      -i  Instructions per step (default: 10)
      -s  Screen scale factor (default: 20)

### Emscripten/asm.js

Place chip8.html and the generated chip8.js and chip8.js.mem files in the same directory and open in a web browser.

### General

You can adjust the speed of the emulator by changing `instructions_per_step` (defaults to 10). This parameter represents the number of Chip-8 instructions executed per frame.

Assuming a frame rate of 60fps, an `instructions_per_step` value of 10 (or a little higher) works well for most Chip-8 games tested, but Connect4 needs `instructions_per_step=1` to be playable. Super-Chip games generally need to be run a bit faster - somewhere in the 20-60 range seems to work well.

### Keyboard map
    Chip-8:    QWERTY keyboard:

    1 2 3 C        1 2 3 4
    4 5 6 D        Q W E R
    7 8 9 E        A S D F
    A 0 B F        Z X C V

Pressing Enter resets the emulator.

## Compatibility

- All Chip-8 and Super-Chip games tested appear to work correctly

## TODO
- Stop using abort() everywhere
- Fix threading
