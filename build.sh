#!/bin/bash
buildDir='build'

if [ ! -d $buildDir ]; then
	mkdir $buildDir
fi

cd ./build
cmake ../
make

