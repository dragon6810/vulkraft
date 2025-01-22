#!/usr/bin/env bash

sh configure.sh
cd bin
if make VERBOSE=1; then
    cd ../run
    ../bin/bin/vulkraft
fi