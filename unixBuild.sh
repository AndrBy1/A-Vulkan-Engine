#!/bin/bash
mkdir -p build_Unix
cd build_Unix
cmake -S ../ -B .
make && make Shaders && ./AVKEngine
cd ..