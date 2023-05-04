cmake_minimum_required(VERSION 3.22.1)

project("metalmax")

set(ANDROID_STL "none")
enable_language(ASM)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s)

file(GLOB_RECURSE MAP_DATA_SRC "maps/map_data/*.cpp" "maps/map_data/*.c")
file(GLOB_RECURSE SOUND_DATA_SRC "sound/*.cpp" "sound/*.c")
file(GLOB_RECURSE MAIN_SRC "*.cpp" "*.c")
file(GLOB_RECURSE NSF_PLAYER_SRC
        "xgm/devices/Audio/*.cpp" "xgm/devices/CPU/*.cpp" "xgm/devices/Memory/*.cpp"
        "xgm/devices/Misc/*.cpp" "xgm/devices/Sound/*.cpp" "xgm/player/nsf/*.cpp"
        "xgm/player/nsf/pls/*.cpp" "vcm/*.cpp")

message(STATUS "MAP_DATA_SRC = ${MAP_DATA_SRC}")

add_library( metalmax
             SHARED
             metalmax.cpp native_gl.cpp
            opt/matrix.cpp opt/matrix_mul_neo.s opt/memcpy-advsimd.S opt/memset.S
            ${MAIN_SRC}
            ${MAP_DATA_SRC}
            ${SOUND_DATA_SRC}
            ${NSF_PLAYER_SRC}
            maps/tile_bmp.cpp
            charsets.cpp
            maps/map.cpp)
target_link_libraries(metalmax
        log android EGL GLESv2 OpenSLES jnigraphics)
