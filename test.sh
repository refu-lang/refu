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
COVERAGE=0
IN_TRAVIS=0
TRAVIS_JOB_ID=0
REQUESTED_ARG=""

function print_help {
    echo "Usage: test.sh [extra-options]"
    echo "Arguments:"
    echo "    --help                  Print this help message."
    echo "    --rfbase                If given then also rfbase submodule tests are ran"
    echo "    --coverage              If given then generate coverage report"
    echo "    --in-travis             We are running tests in a travis instance"
    echo "    --travis-job-id NUMBER   If running inside a travis instance this should contain the job ID"
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

    # zero out previous lcov data
    if [[ $COVERAGE -ne 0 ]]; then
        lcov --zerocounters  --directory .
    fi

    $TEST_CMD
    if [[ $? -ne 0 ]]; then
        echo "test.sh - ERROR: ${1} tests failed"
        exit 1
    fi

    echo "test.sh - INFO: ${1} tests passed"

    if [[ $COVERAGE -ne 0 && ${1} == "test_refu" ]]; then
        echo "Generating coverage report for ${1} ..."
        lcov --directory .  --capture --output-file ${1}.info
        lcov --remove ${1}.info \*.gperf -o ${1}.info
        genhtml --output-directory coverage \
                --demangle-cpp --num-spaces 4 --sort \
                --title "Refu Test Coverage" \
                --function-coverage --branch-coverage --legend \
                ${1}.info
        echo "Coverage report generated!"
        if [[ $IN_TRAVIS -eq 1 ]]; then
            echo "Invoking coveralls-lcov to send report to coveralls.io with Travis Job ID: ${TRAVIS_JOB_ID}"
            LCOV_OUT=`coveralls-lcov -nv ${1}.info`
            if [[ $? -ne 0 ]]; then
                echo "test.sh - ERROR: Failed to invoke coveralls-lcov"
                exit 1
            fi
            REPLACE_NEW="\"service_job_id\": \"${TRAVIS_JOB_ID}\""
            RESULT=${LCOV_OUT/\"service_job_id\":null/$REPLACE_NEW}
            echo $RESULT > coveralls.json
            curl -F 'json_file=@coveralls.json' https://coveralls.io/api/v1/jobs
        fi
    fi

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
    if [[ ${REQUESTED_ARG} != "" ]]; then
        case $REQUESTED_ARG in
            "travis-job-id")
                TRAVIS_JOB_ID=$arg
                ;;
            *)
                echo "ERROR: Unrecognized argument \"$arg\".";
                print_help
                exit 1
        esac
        REQUESTED_ARG=""
        continue
    fi

    if [[ $arg == "--help" ]]; then
        print_help
        exit 1
    fi

    if [[ $arg == "--rfbase" ]]; then
        TEST_RFBASE=1
        continue
    elif [[ $arg == "--coverage" ]]; then
        COVERAGE=1
        continue
    elif [[ $arg == "--in-travis" ]]; then
        IN_TRAVIS=1
        continue
    elif [[ $arg == "--travis-job-id" ]]; then
        REQUESTED_ARG="travis-job-id"
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
