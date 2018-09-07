#!/bin/sh

set -e
set -o pipefail

rm -rf *.cache \
    aclocal.m4 \
    autoscan.log \
    compile \
    config.log \
    config.status \
    configure \
    depcomp \
    install-sh \
    Makefile \
    Makefile.in \
    missing

rm -rf src/.deps \
    src/gateway* \
    src/Makefile \
    src/Makefile.in    