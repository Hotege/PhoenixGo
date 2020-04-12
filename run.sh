#!/bin/bash

showHelp() {
    echo "Usage: $0 [yao-win] [debug|release] (gdb)"
    # echo "Usage: $0 [test] [any] [debug|release] (gdb)"
}

if [[ ! -n "$1" ]] || [[ ! -n "$2" ]]; then
    showHelp
else
    if [[ "$1" == "yao-win" ]]; then
        models=(debug release)
        for m in ${models[*]}; do
            if [[ "$m" == "$2" ]]; then
                pushd "build-$2/install/bin"
                if [[ ! -n "$3" ]]; then
                    ./yao-win.exe
                else
                    gdb ./yao-win.exe
                fi
                popd
                exit
            fi
        done
        echo "Unknown building type: $3"
        showHelp
        exit
    fi
    # if [[ "$1" == "test" ]]; then
    #     if [[ ! -n "$3" ]]; then
    #         showHelp
    #         exit
    #     fi
    #     types=(any gnugo)
    #     models=(debug release)
    #     for e in ${types[*]}; do
    #         if [[ "$e" == "$2" ]]; then
    #             for m in ${models[*]}; do
    #                 if [[ "$m" == "$3" ]]; then
    #                     pushd "build-$3/install/bin/test"
    #                     if [[ ! -n "$4" ]]; then
    #                         ./test_$2.exe
    #                     else
    #                         gdb ./test_$2.exe
    #                     fi
    #                     popd
    #                     exit
    #                 fi
    #             done
    #             echo "Unknown building type: $3"
    #             showHelp
    #             exit
    #         fi
    #     done
    #     echo "Unknown executable: $2"
    #     showHelp
    #     exit
    # fi
    echo "Unknown type: $1"
    showHelp
    exit
fi
