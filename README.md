# Cubic

[![](https://i.imgur.com/5tS5i3M.png)](https://cubic.vldr.org/)

---

A multiplayer WebGL voxel sandbox game written in C++, inspired by the classic version of Minecraft.

## [Try it out](https://cubic.vldr.org/)
You can play around with the game by clicking the above link.

## Controls

`W` and `S` to move forward and backward.   
`A` and `D` to move left and right.  
`V` toggles fly-mode.  
`E` or `B` opens the build menu.  

## Building

### Web (Emscripten)
> [!NOTE]  
> The following instructions assume that you are in a terminal and are using a Unix based machine (Windows Subsystem, Linux, etc).

1. Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and [Make](https://www.gnu.org/software/make/manual/make.html).
2. Navigate to the `build/web/` directory.
3. Run `make -j`

After the build process completes, the output HTML, JS, and WASM files will be located in the `build/web/output/` directory.

### Windows
> [!NOTE]  
> The included compiled SDL2 and GLEW dynamic link binaries are built for x86_64 machines only.

1. Install [Visual Studio](https://visualstudio.microsoft.com/#vs-section).
2. Open `build/windows/Cubic.sln` in Visual Studio.
3. Optionally, if needed, upgrade the project's platform toolset to your Visual Studio's available platform toolset.
4. Select either `Debug` or `Release` from the build dropdown menu (top-center).
5. Press `F7` or click `Build > Build Solution` to build the project.

After the build process completes, the output executable will be located either in `build/windows/x64/Debug` or `build/windows/x64/Release` depending if you've compiled a debug or release build.

### Linux
> [!NOTE]  
> The following instructions assume that you are in a terminal.

1. Install [clang](https://clang.llvm.org/) and [Make](https://www.gnu.org/software/make/manual/make.html).
* On Ubuntu/Debian, run `sudo apt install clang build-essential`
2. Install [SDL2](https://wiki.libsdl.org/SDL2/Installation#linuxunix) and [GLEW](https://glew.sourceforge.net/install.html) 
* On Ubuntu/Debian, run `sudo apt install libgl-dev libglew-dev libsdl2-dev`
2. Navigate to the `build/linux/` directory.
3. Run `make -j`

After the build process completes, the output executable will be located in the `build/linux/output/` directory.

### MacOS
> [!NOTE]  
> The included compiled SDL2 and GLEW dynamic link binaries are built for x86_64 and arm64 machines only.

1. Install [Xcode Command Line Tools](https://mac.install.guide/commandlinetools/4.html).
2. Open Terminal. 
3. Navigate to the `build/macos/` directory.
4. Run `make -j`

After the build process completes, the output executable will be located in the `build/macos/output/` directory.