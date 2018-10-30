#!/bin/bash

TOOLS_PATH=`dirname $0`
ROOT_PATH=`readlink -ev ${TOOLS_PATH}/..`

RISCV_TOOLS_PATH=`readlink -ev ${ROOT_PATH}/riscv-tools`
RISCV_GDB_PATH=`readlink -ev ${RISCV_TOOLS_PATH}/riscv-gnu-toolchain/riscv-binutils-gdb`

mkdir -p ${HOME}/riscv-toolchain
TARGET_TOOLCHAIN_PATH=`readlink -ev ${HOME}/riscv-toolchain`


cd ${RISCV_TOOLS_PATH}
git submodule update --init --recursive

export RISCV=${TARGET_TOOLCHAIN_PATH}
export PATH=$PATH:$RISCV/bin

function patch_apply()
{
    #riscv-gnu-toolchain/riscv-binutils-gdb
    cp ${RISCV_TOOLS_PATH}/patchfiles/gdb/* ${RISCV_GDB_PATH}/
    pushd ${RISCV_GDB_PATH}
    git checkout riscv-next
    patch -p1 < *.patch
    popd
}

patch_apply

./build.sh
