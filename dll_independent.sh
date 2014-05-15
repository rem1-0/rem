#!/bin/sh

# This is script is an adaptation of the following one :
# http://people.videolan.org/~funman/win/howto-gcc

# This script enforces statically linking of libgcc, libstdc++-6,
# and libpthread, without needing to rebuild gcc and mingw-w64 from scratch.
# -static-libgcc -static-libstdc++ flags can not be used in a libtool
# build system, as libtool removes flags that it doesn't understand.

for i in i686 x86_64
do
    cd /usr/lib/gcc/$i-w64-mingw32/* 2>/dev/null \
        || { echo "$i\t: not installed" && continue; }
    mkdir -p old
    if [ -h libgcc_s.a ]
    then
        echo "$i\t: this script seems to have already be called"
        continue
    fi
    mv libgcc_s* libstdc++*dll* old
    ln -s libgcc_eh.a libgcc_s.a
    cd /usr/$i-w64-mingw32/lib
    mkdir -p old
    mv libpthread.dll.a old
    echo "$i\t: done"
done

