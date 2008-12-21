#!/bin/sh

TARGET=/bin-public/
mkdir -p ${TARGET}
cp /bin/cygwin1.dll ${TARGET}

makewrapper() {
    ./makewrapper.sh ${TARGET}$1 "$2" myvars.h
}

makewrapper_t() {
  if [ -e $2 ] ; then
    makewrapper "$@"
  fi
}

makewrapper cg.exe            /bin/cg
makewrapper git.exe           '${DFGITROOT}/bin/git'
makewrapper gitk.exe          '${DFGITROOT}/bin/gitk'
makewrapper ssh.exe           /bin/ssh
makewrapper cygwin.exe        /bin/env
makewrapper cygpath.exe       /bin/cygpath
makewrapper_t ruby.exe          /bin/ruby
makewrapper_t cvsps.exe         /usr/local/bin/cvsps
