#!/bin/sh

TARGET=/bin-public/
mkdir -p ${TARGET}
cp /bin/cygwin1.dll ${TARGET}

makewrapper() {
    ./makewrapper.sh ${TARGET}$1 "$2" myvars.h
}

makewrapper cg.exe            /bin/cg
makewrapper git.exe           '${DFGITROOT}/bin/git'
makewrapper gitk.exe          '${DFGITROOT}/bin/gitk'
makewrapper cygpath.exe       /bin/cygpath
makewrapper cygwin-ls.exe     /bin/ls
makewrapper cygwin-env.exe    /bin/env
makewrapper cygwin-bash.exe   /bin/bash
makewrapper cygwin-perl.exe   /bin/perl
makewrapper cygwin-chmod.exe  /bin/chmod
makewrapper cygwin-python.exe /bin/python
