cmake_minimum_required(VERSION 3.20)

project(dotf
VERSION 0.1.0
LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
FTXUI
GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
GIT_TAG v6.1.9
)

FetchContent_MakeAvailable(FTXUI)

add_executable(dotf
    src/main.cpp
    src/config.cpp
    src/file.cpp
    src/git.cpp
    src/tui.cpp
    src/operations.cpp
)

target_include_directories(dotf
PRIVATE include
)

target_compile_options(dotf
PRIVATE -Wall -Wextra -Wpedantic
)

target_link_libraries(dotf
PRIVATE
ftxui::component
ftxui::dom
ftxui::screen
)
