@echo off
gcc -DTARGET="\"%1\"" wrapper.c -o %2
