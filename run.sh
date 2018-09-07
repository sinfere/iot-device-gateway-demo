#!/bin/sh

set -e
set -o pipefail

make
./src/gateway