#! /bin/sh

cmake -DCMAKE_BUILD_TYPE=Release build
cd build
make -j
