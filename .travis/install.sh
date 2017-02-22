#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    brew update
    brew install cmake
    brew install pkg-config
    brew install check
    brew install llvm
    brew install lcov
    export PATH="$PATH:/usr/local/opt/llvm/bin/"
else
    sudo apt-get update -y -qq
    # Use docker to get LLVM without requiring LLVM's APT server to be online
    docker build -t refu -f test/Dockerfile test/
fi
