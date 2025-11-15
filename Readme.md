# Installation

## Installing libstatgrab

    cd external/libstatgrab
    ./autogen.sh
    ./configure --prefix=$HOME/.local
    make
    make install

## Build and setup

    mkdir build
    cd build 
    cmake configure ..
    cd ..
    cmake --build build
    