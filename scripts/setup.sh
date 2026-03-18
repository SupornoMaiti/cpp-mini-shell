#!/bin/bash
set -e

echo "Installing dependencies..."

if command -v apt &> /dev/null; then
    sudo apt install -y build-essential cmake libreadline-dev
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm base-devel cmake readline
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc-c++ cmake readline-devel
else
    echo "Unknown package manager. Please install manually."
    exit 1
fi

echo "Building..."
mkdir -p build && cd build
cmake ..
make

echo ""
echo "Done! Run ./build/Shell_Project"