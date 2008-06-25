#!/bin/sh
echo "Building $1 -> $2 wrapper..."
if [ -n "$3" ]; then
    g++ -static -DTARGET_VARS="\"$3\"" -DTARGET="\"$2\"" wrapper.cpp -o $1
else
    g++ -static -DTARGET="\"$2\"" wrapper.cpp -o $1
fi
[[ -f $1 ]] && strip -s $1
