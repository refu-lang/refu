# Refulang


[![Travis](http://img.shields.io/travis/refu-lang/refu.svg)](https://travis-ci.org/refu-lang/refu)
[![Coveralls](https://coveralls.io/repos/github/refu-lang/refu/badge.svg?branch=master)](https://coveralls.io/github/refu-lang/refu?branch=master)
[![Join chat in gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/refu-lang/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

This is the code repository for [refulang](http://refu.co/spec.html), a strongly typed, lazily evaluated programming language.


## Building
Building from source is the only way to obtain refu at the moment.

### Dependencies

 The required dependencies are:
 - For building we need [cmake](https://cmake.org/)
 - For the backend code generation we need [llvm](http://llvm.org/)
 - For hash table generation we use [gperf](https://www.gnu.org/software/gperf/)
 - For units tests we need [libcheck](http://check.sourceforge.net/)

The optional dependencies are:
 - For generating dependency graphs, we can use [graphviz](http://www.graphviz.org/)
 - For extra memory checks during unit testing, we can use [valgrind](http://valgrind.org/)
 - For generating a json file of the AST, we can use [json-c](https://github.com/json-c/json-c)
 - For regular expressions (not yet used anywhere), we can use [pcre2](http://www.regular-expressions.info/pcre2.html)

### How to build

```sh
git clone --recursive https://github.com/refu-lang/refu
cd refu
mkdir build
cd build
cmake ..
make
```

### Run the tests
In order to run the tests you have to also build them. This is done by providing an extra
option to cmake.

```sh
cmake -DCMAKE_BUILD_TYPE=Debug .. -DTEST=1
```

There is a convenience test script that runs all of the tests for refu and its rfbase
submodule.

```sh
./test.sh --rfbase
```

## Contributing
You are more than welcome to open a Pull Request here in Github if you would like to contribute
to the development of refu.

## Coding Style and Best Practises
TODO
