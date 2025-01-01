#!/usr/bin/env bash

sh configure.sh
cd bin
if make; then
    cd ../run
    ../bin/bin/vulkraft
fi