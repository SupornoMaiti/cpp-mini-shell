cmake_minimum_required(VERSION 3.20)
project(Shell_Project)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "\n"
        "In-source builds not allowed!\n"
        "Run: mkdir build && cd build && cmake ..\n"
    )
endif()

add_executable(Shell_Project src/main.cpp)

find_library(READLINE_LIB readline)
find_path(READLINE_INCLUDE readline/readline.h)

if(NOT READLINE_LIB OR NOT READLINE_INCLUDE)
    message(FATAL_ERROR
        "\n"
        "readline library not found!\n"
        "Install it with:\n"
        "  Ubuntu/Debian: sudo apt install libreadline-dev\n"
        "  Arch:          sudo pacman -S readline\n"
        "  Fedora:        sudo dnf install readline-devel\n"
    )
endif()

target_include_directories(Shell_Project PRIVATE ${READLINE_INCLUDE})
target_link_libraries(Shell_Project PRIVATE ${READLINE_LIB})