# Refulang


[![Travis](http://img.shields.io/travis/refu-lang/refu.svg)](https://travis-ci.org/refu-lang/refu)
[![Coveralls](https://coveralls.io/repos/github/refu-lang/refu/badge.svg?branch=master)](https://coveralls.io/github/refu-lang/refu?branch=master)
[![Join chat in gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/refu-lang/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

This is the code repository for [refu](http://refu.co/spec.html), a strongly typed, lazily evaluated programming language.



## Dependencies

 The required dependencies are:
 - For building we need [cmake](https://cmake.org/)
 - For the backend code generation we need [llvm](http://llvm.org/)
 - For hash table generation we use [gperf](https://www.gnu.org/software/gperf/)
 - For units tests we need [libcheck](http://check.sourceforge.net/)

The optional dependencies are:
 - For generating dependency graphs, we can use [graphviz](http://www.graphviz.org/)
 - For extra memory checks during unit testing, we can use [valgrind](http://valgrind.org/)
 - For generating a json file of the AST, we can use [json-c](https://github.com/json-c/json-c)
 - For regular expressions (not yet used anywhere yet), we can use [pcre2](http://www.regular-expressions.info/pcre2.html)
 - For generating code coverage reports we can use [lcov](https://linux.die.net/man/1/lcov)


### Dependencies in Linux

Use your distribution's package manager

### Dependencies in Macosx

```sh
brew install cmake
brew install pkg-config
brew install check
brew install llvm
brew install lcov
```

Installing llvm does not place it into the `PATH` and as such you will also need to put its path
there manually. In recent versions this means adding the following to `.profile`.
```sh
export PATH="$PATH:/usr/local/opt/llvm/bin/"
```

## Building

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

## Usage

You can invoke `refu --help` in order to get all the possible arguments along with
their description.

Example:

```
 --help                                                  display this help and exit
 --version                                               display version info and exit
 -v, --verbose-level=1-4                                 Set compiler verbosity level
 --backend=GCC|LLVM                                      The backend connection the refu compiler will user
 --backend-debug                                         If given then some debug information about the backend code will be printed
 --output-ast                                            If given then after analysis state the AST will be output in JSON format
 -o, --output=name                                       output file name. Defaults to input.exe if not given
 --rir                                                   Interpret the input file as a RIR file and parse it.
 -r, --print-rir                                         If given will output the intermediate representation in a file
 <file>                                                  input files
 ```

At the moment these are the important arguments.

- `--verbose-level`
Will determine how verbose the output of the compiler will be.

- `--backend-debug`
Will print out to the stdout the [LLVM IR](http://llvm.org/docs/LangRef.html) for
debugging purposes.

- `--output-ast`
Will output the Abstract Syntax Tree in a .json format

- `--print-rir`
Will print the generated Refu Intermediate Format (RIR) for deugging purposes.


## Contributing
You are more than welcome to open a Pull Request here in Github if you would like to contribute
to the development of refu.

## Coding Style and Best Practises
TODO
