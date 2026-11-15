# Kapitel 20: Toolchain-Datei für den Raspberry Pi (64 Bit, Raspberry Pi OS / Ubuntu arm64)
#   sudo apt install gcc-aarch64-linux-gnu        (Compiler)
#   sudo dpkg --add-architecture arm64 && sudo apt update
#   sudo apt install libcurl4-openssl-dev:arm64 libncurses-dev:arm64   (Bibliotheken)
#   cmake -S . -B build-pi -DCMAKE_TOOLCHAIN_FILE=kapitel20_projekte/aarch64-linux-gnu.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)     # /usr/lib/aarch64-linux-gnu
# Bibliotheken und Header nur für das Ziel suchen, Programme (cmake, git) auf dem Host
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu / $ENV{PI_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
