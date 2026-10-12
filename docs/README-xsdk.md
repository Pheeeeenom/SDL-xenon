# Xbox 360 (xsdk)

SDL port for the Xbox 360 through [xsdk](https://github.com/xenosworks/xsdk), a homebrew SDK that builds titles for
the console's retail kernel. This is an altered version of SDL 3.4.10; the changes are on the `xenon` branch.

## Building

Build xsdk first (`make` in its checkout builds `libxsdk.a`), then:

```bash
cmake -S. -Bbuild-xsdk -GNinja -DCMAKE_TOOLCHAIN_FILE=<xsdk>/cmake/xsdk.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-xsdk
```

SDL builds as a static library. A title links it through `find_package(SDL3)` and turns its ELF into a `.xex` with
xsdk's `xsdk_add_xex()`; `test/xsdk` is such a title.

## Notes

-   Audio plays through the kernel's render driver: 6 channels of big-endian floats at 48 kHz, 256 frames at a time,
    which SDL converts to. There's no recording.
-   Up to four controllers, read with `XamInputGetState` and rumbled with `XamInputSetState`, show up as Xbox 360
    controllers.
-   `SDL_GetBasePath` returns `game:\`, the folder the title was launched from, and `SDL_GetPrefPath` makes its folders
    beside it.
-   Threads, timers and time come from xsdk's pthreads and POSIX clocks.
-   There's no video, render or GPU driver of the console's own; only SDL's dummy and offscreen video drivers. Titles
    draw with xsdk's GPU layer or [vulkan4xenon](https://github.com/xenosworks/vulkan4xenon).
-   `test/xsdk/sdl_check.c` checks the platform's name, paths, timer, threads, controller and audio on the console.
