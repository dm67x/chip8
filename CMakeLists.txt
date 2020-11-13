cmake_minimum_required(VERSION 3.12)

project(chip8 
    LANGUAGES CXX
    VERSION "0.1"
    DESCRIPTION "chip8 emulator"
    HOMEPAGE_URL "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/SDL2)
include_directories(extern/SDL2/include)

add_executable(chip8 "src/chip8.cpp")
if (MINGW)
    target_link_libraries(chip8 mingw32 SDL2main SDL2-static)
else ()
    target_link_libraries(chip8 SDL2main SDL2-static)
endif ()

target_compile_options(chip8 PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)