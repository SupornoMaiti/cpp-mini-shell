cmake_minimum_required(VERSION 3.20)
project(Shell_Project)

set(CMAKE_CXX_STANDARD 17)

add_executable(Shell_Project src/main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE readline)

if(NOT READLINE_FOUND)
    message(FATAL_ERROR
            "\n"
            "readline library not found!\n"
            "Install it with:\n"
            "  Ubuntu/Debian: sudo apt install libreadline-dev\n"
            "  Arch:          sudo pacman -S readline\n"
            "  Fedora:        sudo dnf install readline-devel\n"
    )
endif()

target_include_directories(Shell_Project PRIVATE ${READLINE_INCLUDE_DIRS})
target_link_libraries(Shell_Project PRIVATE ${READLINE_LIBRARIES})