#!/bin/bash

showHelp() {
    echo "Usage: $0 [debug|release]"
}

cmakeProc() {
    if [[ -n "$1" ]]; then
        params=
        id=0
        for e in $*; do
            if [[ "$id" -ne 0 ]]; then
                params[$(($id-1))]=$e
            fi
            let 'id+=1'
        done
        rm -rf "build-$1"
        mkdir "build-$1"
        pushd "build-$1"
        cmake .. -G "Unix Makefiles" \
        -DCMAKE_MAKE_PROGRAM=mingw32-make \
        -DCMAKE_INSTALL_PREFIX=install \
        ${params[*]}
        popd
    fi
}

if [[ ! -n "$1" ]]; then
    showHelp
else
    case "$1" in
    debug )
        cmakeProc debug -DDEBUG=ON
        ;;
    release )
        cmakeProc release -DRELEASE=ON
        ;;
    * )
        echo "Unknown building type: $1."
        showHelp
        ;;
    esac
fi
