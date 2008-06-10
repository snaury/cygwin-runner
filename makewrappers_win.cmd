@echo off
set cygdir=C:/cygwin
set gitdir=C:/cygwin/home/aborzenkov/git-1.5.5.3
sh-msys ./makewrapper.sh bin-win/git.exe %gitdir%/bin/git.exe myvars_win.h
sh-msys ./makewrapper.sh bin-win/gitk.exe %gitdir%/bin/gitk.exe myvars_win.h
sh-msys ./makewrapper.sh bin-win/cygwin-ls.exe %cygdir%/bin/ls.exe myvars_win.h
sh-msys ./makewrapper.sh bin-win/cygwin-bash.exe %cygdir%/bin/bash.exe myvars_win.h
sh-msys ./makewrapper.sh bin-win/cygwin-chmod.exe %cygdir%/bin/chmod.exe myvars_win.h
