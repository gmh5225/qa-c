#!/bin/bash


cmake -S . -B build
cmake --build build


# Check for build success
if [ $? -eq 0 ]; then
    echo "Build succeeded."
else
    echo "Build failed."
    exit 1
fi
