
Native compilation under Ubuntu 22.04
=====================================

# mkdir Build
# cd Build
# cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_VCPKG_PACKAGES=OFF -G Ninja
# ninja


Linux Standard Base compilation
===============================

# ./holy-build-box-compile.sh Release


MinGW
=====

MinGW is not supported, at least not on Ubuntu <= 22.04, as AWS uses
C++17 constructions that are not available in gcc 10.x.
