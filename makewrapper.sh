#!/bin/sh
if [ -n "$3" ]; then
    echo "Building $1 -> $2 wrapper (with $3)..."
    g++ -static -O2 -DTARGET_VARS="\"$3\"" -DTARGET="\"$2\"" wrapper.cpp -o $1
else
    echo "Building $1 -> $2 wrapper..."
    g++ -static -O2 -DTARGET="\"$2\"" wrapper.cpp -o $1
fi
[[ -f $1 ]] && strip -s $1
