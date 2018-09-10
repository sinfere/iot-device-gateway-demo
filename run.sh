#!/bin/sh

set -e
set -o pipefail

make
LD_LIBRARY_PATH=/usr/local/lib \
    ./src/gateway