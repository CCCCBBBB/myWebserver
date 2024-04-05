#! /bin/bash


set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. && 
    make

cd ..

if [ ! -d /usr/include/myWebserver ]; then
    mkdir /usr/include/myWebserver
fi

for header in `ls *.h`
do
    cp $header /usr/include/myWebserver
done

cp `pwd`/lib/libmyWebserver.so /usr/lib

ldconfig
