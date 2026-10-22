cmake_minimum_required(VERSION 3.22.1)

project("metalmax")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

file(GLOB_RECURSE MAIN_SRC CONFIGURE_DEPENDS "*.cpp" "*.c")

add_library(metalmax
        SHARED
        ${MAIN_SRC})
target_link_libraries(metalmax
        log
        android
        EGL
        GLESv2
        OpenSLES
        jnigraphics)
