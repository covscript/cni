#!/usr/bin/env bash
mkdir -p cmake-build/unix
cd       cmake-build/unix
cmake -G "Unix Makefiles" ../..
cmake --build . --target covscript -- -j8
cd ../..
rm -rf build
mkdir -p build/lib
mv cmake-build/unix/*.a build/lib/
rm -rf csdev
mkdir -p csdev/include/covscript
mkdir -p csdev/lib
cp -r include/covscript csdev/include/
cp -r build/lib         csdev/
