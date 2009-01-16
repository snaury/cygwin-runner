TARGET=/bin-public/
mkdir -p ${TARGET}

makewrapper() {
    ./compilewrapper ${TARGET}$1 "$2" myvars.h
}

makewrapper_t() {
  if [ -e $2 ] ; then
    makewrapper "$@"
  fi
}

