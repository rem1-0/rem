#!/bin/sh

# contrary to dll_independent.sh

for i in i686 x86_64
do
    cd /usr/lib/gcc/$i-w64-mingw32/* 2>/dev/null \
        || { echo "$i\t: not installed" && continue; }
    res=0
    ## teste si libgcc_s.a existe et est un lien symbolique
    if [ -h libgcc_s.a ]
    then
        rm libgcc_s.a
        res=1
    fi
    if [ -d old ]
    then
        mv old/* .
        rmdir old
        res=1
    fi
    
    cd /usr/$i-w64-mingw32/lib || continue
    if [ -d old ]
    then
        mv old/libpthread.dll.a .
        rmdir old
        res=1
    fi
    if [ $res -eq 1 ]
    then
        echo "$i\t: done"
    else
        echo "$i\t: this script seems to have already be called"
    fi
done
    
