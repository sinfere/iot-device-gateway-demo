#!/bin/sh

set -e
set -o pipefail

make

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

./src/gateway