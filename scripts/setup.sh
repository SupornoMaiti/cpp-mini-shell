#!/bin/bash
set -e

echo "Installing dependencies..."

if command -v apt &> /dev/null; then
    sudo apt install -y build-essential cmake libreadline-dev pkg-config
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm base-devel cmake readline pkg-config
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc-c++ cmake readline-devel pkgconfig
else
    echo "Unknown package manager. Please install manually."
    exit 1
fi

echo "Building..."
rm -rf build
sudo mkdir -p build
cd build
cmake ..
make

echo ""
echo "Done! Run ./build/Shell_Project"
#!/bin/bash
set -e

echo "Installing dependencies..."

if command -v apt &> /dev/null; then
    sudo apt install -y build-essential cmake libreadline-dev pkg-config
elif command -v pacman &> /dev/null; then
    sudo pacman -S --noconfirm base-devel cmake readline pkg-config
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc-c++ cmake readline-devel pkgconfig
else
    echo "Unknown package manager. Please install manually."
    exit 1
fi

echo "Building..."
sudo rm -rf build
mkdir -p build
cd build
cmake ..
make

echo ""
echo "Done! Run ./build/Shell_Project"
