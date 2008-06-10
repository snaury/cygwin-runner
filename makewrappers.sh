#!/bin/sh
gitdir=/home/aborzenkov/git-1.5.5.3
./makewrapper.sh ~/shell-runners/git.exe $gitdir/bin/git myvars.h
./makewrapper.sh ~/shell-runners/gitk.exe $gitdir/bin/gitk myvars.h
./makewrapper.sh ~/shell-runners/cygwin-ls.exe /bin/ls myvars.h
./makewrapper.sh ~/shell-runners/cygwin-bash.exe /bin/bash myvars.h
./makewrapper.sh ~/shell-runners/cygwin-chmod.exe /bin/chmod myvars.h
