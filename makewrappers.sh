#!/bin/sh
./makewrapper.sh $HOME/shell-runners/git.exe ${DFGITROOT}/bin/git myvars.h
./makewrapper.sh $HOME/shell-runners/gitk.exe ${DFGITROOT}/bin/gitk myvars.h
./makewrapper.sh $HOME/shell-runners/cygwin-ls.exe /bin/ls myvars.h
./makewrapper.sh $HOME/shell-runners/cygwin-bash.exe /bin/bash myvars.h
./makewrapper.sh $HOME/shell-runners/cygwin-chmod.exe /bin/chmod myvars.h
