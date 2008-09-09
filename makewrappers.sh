#!/bin/sh
./makewrapper.sh /bin-public/cg.exe /bin/cg myvars.h
./makewrapper.sh /bin-public/git.exe '${DFGITROOT}/bin/git' myvars.h
./makewrapper.sh /bin-public/gitk.exe '${DFGITROOT}/bin/gitk' myvars.h
./makewrapper.sh /bin-public/cygpath.exe /bin/cygpath myvars.h
./makewrapper.sh /bin-public/cygwin-ls.exe /bin/ls myvars.h
./makewrapper.sh /bin-public/cygwin-env.exe /bin/env myvars.h
./makewrapper.sh /bin-public/cygwin-bash.exe /bin/bash myvars.h
./makewrapper.sh /bin-public/cygwin-chmod.exe /bin/chmod myvars.h
./makewrapper.sh /bin-public/cygwin-python.exe /bin/python myvars.h
