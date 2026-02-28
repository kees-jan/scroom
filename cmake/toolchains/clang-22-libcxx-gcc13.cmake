# Toolchain file for building with Clang 22.
#
# Usage:
#
# cmake -S . -B <build-dir> -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-22-libcxx-gcc13.cmake
#
# Notes:
#
# * Forces clang/clang++ 22
# * Uses libstdc++ (Clang default on Ubuntu) so it is ABI-compatible with distro libraries (e.g. Boost)
# * Pins --gcc-install-dir to GCC 13 to prefer a working GCC installation / libstdc++ headers

set(CMAKE_C_COMPILER clang-22)
set(CMAKE_CXX_COMPILER clang++-22)

# Make Clang prefer a specific GCC installation for its GCC toolchain bits.
set(_SCROOM_GCC_INSTALL_DIR "/usr/lib/gcc/x86_64-linux-gnu/13")

set(CMAKE_C_FLAGS_INIT "--gcc-install-dir=${_SCROOM_GCC_INSTALL_DIR}")
set(CMAKE_CXX_FLAGS_INIT "-stdlib=libstdc++ --gcc-install-dir=${_SCROOM_GCC_INSTALL_DIR}")

# Useful when driving tools like clang-tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
