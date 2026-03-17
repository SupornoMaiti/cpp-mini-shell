#!/bin/bash

echo "Installing dependencies..."

if command -v apt &> /dev/null; then
    sudo apt install -y libreadline-dev
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm readline
elif command -v dnf &> /dev/null; then
    sudo dnf install -y readline-devel
else
    echo "Unknown package manager. Please install readline manually."
    exit 1
fi

echo "Building..."
mkdir -p build && cd build
cmake ..
make

echo "Done! Run ./build/Shell_Project"