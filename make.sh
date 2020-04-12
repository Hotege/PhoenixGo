#!/bin/bash

showHelp() {
    echo "Usage: $0 [debug|release]"
}

makeProc() {
    if [[ -n "$1" ]]; then
        pushd "build-$1"
        mingw32-make clean
        mingw32-make install
        popd
    fi
}

if [[ ! -n "$1" ]]; then
    showHelp
else
    case "$1" in
    debug )
        makeProc debug
        grep TODO * -rnI --exclude=*.sh --exclude=*.py --exclude=*.proto
        ;;
    release )
        makeProc release
        grep TODO * -rnI --exclude=*.sh --exclude=*.py --exclude=*.proto
        ;;
    * )
        echo "Unknown building type: $1."
        showHelp
        ;;
    esac
fi
