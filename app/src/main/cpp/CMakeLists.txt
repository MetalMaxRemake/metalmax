cmake_minimum_required(VERSION 3.22.1)

project("metalmax")

set(ANDROID_STL "none")
enable_language(ASM)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s)

file(GLOB_RECURSE MAP_DATA_SRC "maps/map_data/*.cpp" "maps/map_data/*.c" "maps/*.cpp")
file(GLOB_RECURSE SOUND_DATA_SRC "audio/*.cpp" "audio/*.c")
file(GLOB_RECURSE MAIN_SRC "*.cpp" "*.c")
file(GLOB_RECURSE LOGIC_SRC "logic/*.cpp" "logic/*.c")
file(GLOB_RECURSE CHARSET_SRC "charset/*.cpp" "charset/*.c")
file(GLOB_RECURSE GRAPHIC_SRC "graphic/*.cpp" "graphic/*.c")

file(GLOB_RECURSE NSF_PLAYER_SRC
        "audio/*.cpp" "audio/*.c"
        "audio/xgm/devices/Audio/*.cpp" "audio/xgm/devices/CPU/*.cpp" "audio/xgm/devices/Memory/*.cpp"
        "audio/xgm/devices/Misc/*.cpp" "audio/xgm/devices/Sound/*.cpp" "audio/xgm/player/nsf/*.cpp"
        "audio/xgm/player/nsf/pls/*.cpp" "audio/vcm/*.cpp")

message(STATUS "MAP_DATA_SRC = ${MAP_DATA_SRC}")

add_library( metalmax
             SHARED
            bridge.cpp perf.cpp
            opt/matrix.cpp opt/matrix_mul_neo.s opt/memcpy-advsimd.S opt/memset.S
            ${MAIN_SRC}
            ${LOGIC_SRC}
            ${CHARSET_SRC}
            ${GRAPHIC_SRC}
            ${MAP_DATA_SRC}
            ${SOUND_DATA_SRC}
            ${NSF_PLAYER_SRC})
target_link_libraries(metalmax
        log android EGL GLESv2 jnigraphics)
