#!/bin/bash

if [ ! -e sim ] ;then
    echo "simulator is not built yet. :cry:"
    exit
fi

cd `dirname $0`
for f in `ls samples/*.bin`; do
    echo "[*] trying $f ..."
    ./sim $f > result.txt
    diff result.txt "${f%%.*}.expected" > /dev/null
    ret=$?
    if [ $ret -eq 0 ] ;then
        printf "\e[32m$f: passed.\e[m\n"
        echo "got: "
        cat result.txt
        echo ""
    else 
        printf "\e[31m$f: failed.\e[m\n"
        echo "expected: "
        cat "${f%%.*}.expected"
        echo "got: "
        cat result.txt
    fi
    rm result.txt
done

