#!/bin/sh
echo "Building $1 -> $2 wrapper..."
if [ -n "$3" ]; then
    gcc -DTARGET_VARS="\"$3\"" -DTARGET="\"$2\"" wrapper.c -o $1
else
    gcc -DTARGET="\"$2\"" wrapper.c -o $1
fi
strip -s $1
