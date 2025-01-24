#!/usr/bin/env bash

sh configure.sh || exit 1
cd bin
if make VERBOSE=1 && ../shaders/compileshaders.sh; then
    cd ../run
    ../bin/bin/vulkraft
else
    exit 1
fi

exit 0