# A title that checks SDL's xsdk drivers on the Xbox 360, against SDL built for xsdk:
#   cmake -S test/xsdk -B build-xsdk-check -G Ninja -DCMAKE_TOOLCHAIN_FILE=<xsdk>/cmake/xsdk.cmake -DSDL3_DIR=build-xsdk
cmake_minimum_required(VERSION 3.25)
project(sdl_check C)

find_package(SDL3 REQUIRED CONFIG)

add_executable(sdl_check sdl_check.c)
target_compile_features(sdl_check PRIVATE c_std_11)
target_compile_options(sdl_check PRIVATE -Wall -Wextra -Werror)
target_link_libraries(sdl_check PRIVATE SDL3::SDL3-static)
xsdk_add_xex(sdl_check)
