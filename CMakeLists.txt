cmake_minimum_required(VERSION 3.12)

project(chip8 
    LANGUAGES CXX
    VERSION "0.1"
    DESCRIPTION "chip8 emulator"
    HOMEPAGE_URL "")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/SDL2)
include_directories(extern/SDL2/include)

add_executable(chip8 "chip8.cpp")
target_link_libraries(chip8 mingw32 SDL2main SDL2)