#!/usr/bin/env bash
# author: Lefteris Karapetsas <lefteris@refu.co>
#
# A script to run tests for refu

WORKING_DIR="./build/test/"
TEST_CMD="./test_refu CK_VERBOSE False"

if [[ ! -d $WORKING_DIR ]]; then
    echo "test.sh - ERROR: Could not find ${WORKING_DIR} directory"
    exit 1
fi

if [[ ! -x ./build/test/test_refu ]]; then
    echo "test.sh - ERROR: Could not find test_refu. Have you already built the tests?"
    exit 1
fi

# Execute the tests
cd $WORKING_DIR
$TEST_CMD
if [[ $? -ne 0 ]]; then
    echo "test.sh - ERROR: tests failed"
    exit 1
fi

echo "test.sh - INFO: tests passed"

command -v valgrind >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
    echo "test.sh - INFO: Valgrind found. Running tests under valgrind"
    valgrind --tool=memcheck --leak-check=yes --track-origins=yes --show-reachable=yes --num-callers=20 --track-fds=yes $TEST_CMD
else
    echo "test.sh - INFO: Valgrind not found."
fi
