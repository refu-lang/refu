#!/usr/bin/env bash
# author: Lefteris Karapetsas <lefteris@refu.co>
#
# A script to run tests for refu

# Get SCRIPT_DIR, the directory the script is located even if there are symlinks involved
FILE_SOURCE="${BASH_SOURCE[0]}"
# resolve $FILE_SOURCE until the file is no longer a symlink
while [ -h "$FILE_SOURCE" ]; do
    SCRIPT_DIR="$( cd -P "$( dirname "$FILE_SOURCE" )" && pwd )"
    FILE_SOURCE="$(readlink "$FILE_SOURCE")"
    # if $FILE_SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
    [[ $FILE_SOURCE != /* ]] && FILE_SOURCE="$SCRIPT_DIR/$FILE_SOURCE"
done
SCRIPT_DIR="$( cd -P "$( dirname "$FILE_SOURCE" )" && pwd )"
WORKING_DIR="./build/test"
TEST_CMD=" ./test_refu CK_VERBOSE False"
TEST_RFBASE=0

function print_help {
    echo "Usage: test.sh [extra-options]"
    echo "Arguments:"
    echo "    --help                  Print this help message."
    echo "    --rfbase                If given then also rfbase submodule tests are ran"
}

function run_tests {
    PREVIOUS_DIR=`pwd`
    cd ${SCRIPT_DIR}
    if [[ ! -d $WORKING_DIR ]]; then
    echo "test.sh - ERROR: Could not find ${WORKING_DIR} directory"
    exit 1
    fi

    TEST_EXEC=${WORKING_DIR}/${1}
    if [[ ! -x $TEST_EXEC ]]; then
    echo "test.sh - ERROR: Could not find ${TEST_EXEC}. Have you already built the tests?"
    exit 1
    fi

    # Execute tests
    cd $WORKING_DIR
    $TEST_CMD
    if [[ $? -ne 0 ]]; then
    echo "test.sh - ERROR: ${1} tests failed"
    exit 1
    fi

    echo "test.sh - INFO: ${1} tests passed"

    command -v valgrind >/dev/null 2>&1
    if [[ $? -eq 0 ]]; then
    echo "test.sh - INFO: Valgrind found. Running tests under valgrind"
    valgrind --tool=memcheck --leak-check=yes --track-origins=yes --show-reachable=yes --num-callers=20 --track-fds=yes $TEST_CMD
    echo "test.sh - INFO: ${1} VALGRIND tests passed"
    else
    echo "test.sh - INFO: Valgrind not found."
    fi

    cd ${PREVIOUS_DIR}
}

for arg in ${@:1}
do
    if [[ $arg == "--help" ]]; then
        print_help
        exit 1
    fi

    if [[ $arg == "--rfbase" ]]; then
        TEST_RFBASE=1
        continue
    fi

    # if we get here the argument is not recognized
    echo "test.sh: Unrecognized argument ${arg}."
    print_help
    exit 1
done


run_tests test_refu

if [[ $TEST_RFBASE -eq 1 ]]; then
    WORKING_DIR="./build/rfbase/test"
    TEST_CMD="./test_rfbase CK_VERBOSE False"
    run_tests test_rfbase
fi
