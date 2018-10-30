#!/bin/bash

cd riscv-qemu

mkdir -p build
cd build

#../configure
../configure --target-list=riscv64-softmmu

make
