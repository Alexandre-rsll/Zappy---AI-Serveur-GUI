#!/bin/bash

set -e

clear || true
if [[ $1 == "re" ]]
then
    echo "------------RE-BUILD------------"
    rm -rf ./build/ ./zappy_server ./zappy_gui
    mkdir ./build/ && cd ./build/
    cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Wno-dev
    cmake --build .
    cd ..
    echo "------------END------------"

elif [[ $1 == "d" ]]
then
    echo "------------DEBUG------------"
    rm -rf ./build/ ./zappy_server ./zappy_gui
    mkdir ./build/ && cd ./build/
    cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Wno-dev
    cmake --build . -v
    cd ..
    echo "------------END------------"

elif [[ $1 == "clean" ]]
then
    echo "------------CLEAN------------"
    rm -rf ./build/
    mkdir ./build/ && cd ./build/
    cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Wno-dev
    cmake --build . --target clean
    cd ..
    rm -rf ./build/
    echo "------------END------------"

else
    echo "------------BUILD------------"
    if [ ! -d "./build/" ]
    then
        mkdir ./build/ && cd ./build/
        cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Wno-dev
        cd ..
    else
        cmake --build ./build/
    fi
    echo "------------END------------"
fi
