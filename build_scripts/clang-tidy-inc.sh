if [ -z "${BUILD+x}" ]; then
  BUILD=cmake-build-debug-clang
fi
CLANG_VERSION=22

find_sources_print0() {
  find libs plugins gui -name \*.cc -print0
}
