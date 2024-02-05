#!/bin/bash
ninja -C ./build/ || exit 1;
./build/wordle
