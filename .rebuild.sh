#!/bin/bash
rm -r build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=y -DECM_ENABLE_SANITIZERS="memory,address" ..
make