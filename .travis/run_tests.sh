#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then
    # In macosx we don't currently run coverage
    mkdir build &&
        cd build &&
        cmake -DTEST=1 ..  &&
        make -j4 && cd .. &&
        ./test.sh --rfbase --in-travis --travis-job-id ${TRAVIS_JOB_ID}
else
    docker run -v `pwd`:/build refu sh -c "mkdir build && cd build && cmake -DCOVERAGE=1 -DTEST=1 ..  && make -j4 && cd .. && ./test.sh --rfbase --coverage --in-travis --travis-job-id ${TRAVIS_JOB_ID}    "
fi
