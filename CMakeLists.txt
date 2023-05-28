cmake_minimum_required(VERSION 3.12)

project(chip8 
    LANGUAGES CXX
    VERSION "0.1"
    DESCRIPTION "chip8 emulator"
    HOMEPAGE_URL "https://github.com/dm67x/chip8")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dependencies.cmake)
find_package(SDL2 REQUIRED)

add_executable(chip8 "src/chip8.cc")
target_link_libraries(chip8 SDL2::SDL2main SDL2::SDL2-static)

target_compile_options(chip8 PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
)

add_custom_command(
  TARGET chip8 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/roms/
    $<TARGET_FILE_DIR:chip8>
)