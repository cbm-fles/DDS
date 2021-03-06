#!/bin/bash

if ! which clang-format; then
    echo "Error: cannot find clang-format in your path"
    exit 1
fi


find $1 -name \*.h -print -o -name \*.cpp -print | xargs clang-format -i
