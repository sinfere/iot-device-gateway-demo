#!/bin/sh

set -e
set -o pipefail

autoreconf --install --force
# aclocal
# automake
# autoconf
./configure
make