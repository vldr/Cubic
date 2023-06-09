# Cubic

![](https://i.imgur.com/5tS5i3M.png)

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
**Note:** The following instructions assume that you are in a terminal and are using a Unix based machine (Windows Subsystem, Linux, etc).

1. Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and [Make](https://www.gnu.org/software/make/manual/make.html).
2. Navigate to the `Web/` directory.
3. Run `make`

After the build process completes, the output HTML, JS, and WASM files will be located in the `Web/Build/` directory.

### Windows

**Note:** The included compiled SDL2 and GLEW dynamic link binaries are built for x86-based machines only.

1. Install [Visual Studio](https://visualstudio.microsoft.com/#vs-section).
2. Open `Windows/Cubic.sln` in Visual Studio.
3. Optionally, if needed, upgrade the project's platform toolset to your Visual Studio's available platform toolset.
4. Select either `Debug` or `Release` from the build dropdown menu (top-center).
5. Press `F7` or click `Build > Build Solution` to build the project.

After the build process completes, the output executable will be located either in `Windows/x64/Debug` or `Windows/x64/Release` depending if you've compiled a debug or release build.
