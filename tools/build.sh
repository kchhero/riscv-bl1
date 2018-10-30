#!/bin/bash

CURRENT_PATH=`dirname $0`
TOOLS_PATH=`readlink -ev $CURRENT_PATH`
ROOT_PATH=`readlink -ev ${TOOLS_PATH}/..`

argc=$#

BUILD_PATH=

TOOLCHAIN_PATH="${HOME}/riscv-toolchain"

FIRST_BUILD=false
CLEAN_BUILD=false
SDK_BUILD=false
CONVERT_ONLY=false

BUILD_ALL=true
BUILD_BL1=false
BUILD_PK=false
BUILD_KERNEL=false
BUILD_DTB=false
BUILD_QEMU=false
BUILD_YOCTO=false
BUILD_ROOTFS=false

BL1_PATH=`readlink -ev ${ROOT_PATH}/riscv-bl1/`

PK_PATH=`readlink -ev ${ROOT_PATH}/riscv-pk/`
PK_PAYLOAD_ENABLE=true

KERNEL_PATH=`readlink -ev ${ROOT_PATH}/riscv-linux/`
KERNEL_ARCH=riscv

DTB_PATH=`readlink -ev ${ROOT_PATH}/riscv-linux/arch/riscv/boot/dts/`

BUILDROOT_CONF_PATH=`readlink -ev ${ROOT_PATH}/tools/conf/`

YOCTO_PATH=`readlink -ev ${ROOT_PATH}/yocto/`
YOCTO_POKY_PATH=`readlink -ev ${ROOT_PATH}/yocto/riscv-poky/`

ROOTFS_PATH=
DEFAULT_ROOTFS_PATH=`readlink -ev ${ROOT_PATH}/tools/rootfs/`

RAMDISK_FILE=ramdisk.cpio.gz

BOARD_NAME=

set -e

function parse_args()
{
    ARGS=$(getopt -o "b:cfsrt:h" -- "$@");
    eval set -- "$ARGS";

    while true; do
        case "$1" in
	    -b ) BOARD_NAME=$2; shift 2 ;;
            -c ) CLEAN_BUILD=true; shift 1 ;;
	    -f ) FIRST_BUILD=true; shift 1 ;;
            -s ) SDK_BUILD=true; shift 1 ;;
            -r ) CONVERT_ONLY=true; shift 1 ;;
	    -t ) case "$2" in
                     bl1    ) BUILD_ALL=false; BUILD_BL1=true ;;
                     pk     ) BUILD_ALL=false; BUILD_PK=true ;;
                     kernel ) BUILD_ALL=false; BUILD_KERNEL=true ;;
                     dtb    ) BUILD_ALL=false; BUILD_DTB=true ;;
                     qemu   ) BUILD_ALL=false; BUILD_QEMU=true ;;
                     yocto  ) BUILD_ALL=false; BUILD_YOCTO=true ;;
                     rootfs ) BUILD_ALL=false; BUILD_ROOTFS=true ;;
                     *      ) usage; exit 1 ;;
		 esac
		 shift 2 ;;
	    -h ) usage; exit 1 ;;
	    -- ) break ;;
            *  ) echo "invalid option $1"; usage; exit 1;;
        esac
    done
}

function usage()
{
    echo -e "\nUsage: $0 [-b <boardname> -c -f -t bl1 -t pk -t kernel -t dtb -t qemu -t yocto] \n"
    echo -e " : default, total build, need boardname argument"
    echo -e " -b <boardname> : target board name (available boards: qemu drone)"
    echo -e " -c : cleanbuild"
    echo -e " -f : first build"
    echo -e " -t bl1    : if you want to build only bl1, specify this, default no"
    echo -e " -t pk     : if you want to build only pk, specify this, default no"
    echo -e " -t kernel : if you want to build only kernel, specify this, default no"
    echo -e " -t dtb    : if you want to build only dtb, specify this, default no"
    echo -e " -t qemu   : if you want to build only qemu, specify this, default no"
    echo -e " -t yocto  : if you want to build only yocto, specify this, default no"
    echo -e " -t rootfs : if you want to build only rootfs, specify this, default no"
    echo " ex) $0 "
    echo " ex) $0 -b qemu"
    echo " ex) $0 -f"
    echo " ex) $0 -t pk"
    echo " ex) $0 -t bl1 -t pk"
    echo " ex) $0 -t bl1 -t pk -t kernel"
    echo " ex) $0 -t bl1 -t pk -t kernel -t dtb"
    echo " ex) $0 -t qemu"
    echo " ex) $0 -t yocto"
    echo ""
}

function environment_check()
{
    if [ -d ${TOOLCHAIN_PATH} ]; then
        echo "Toolchain exist"
        TOOLCHAIN_PATH=`readlink -ev ${HOME}/riscv-toolchain`
        export RISCV=${TOOLCHAIN_PATH}
        export PATH=$PATH:$RISCV/bin
    else
        echo "toolchain does not installed, Plesae check toolchain or have to first build"
        usage
        exit 1
    fi
}

function do_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m              ---------  Build Start ---------                      \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    if [ ! -d ${ROOT_PATH}/build ];then
        mkdir -p ${ROOT_PATH}/build
    fi
    BUILD_PATH=`readlink -ev ${ROOT_PATH}/build`

    if [ ${FIRST_BUILD} == true ];then
        echo -e "\033[45;30m First Build !\033[0m"
        toolchain_build

    	environment_check

        yocto_build
        qemu_build
        bl1_build
        kernel_build
        pk_build
	dtb_build
    else
    	environment_check
        if [ $CONVERT_ONLY == true ];then
            echo -e "No build, convert only"
        elif [ $BUILD_ALL == true ];then
            echo -e "\033[45;30m All Build !\033[0m"
#    qemu_build
            bl1_build
            # kernel_build
            # pk_build
	    # dtb_build
        else
            echo -e "\033[45;30m Partital Build !\033[0m"
            if [ $BUILD_BL1 == true ];then
                bl1_build
            fi
            if [ $BUILD_PK == true ];then
                pk_build
            fi
            if [ $BUILD_KERNEL == true ];then
                kernel_build
            fi
	    if [ $BUILD_DTB == true ];then
                dtb_build
            fi
            if [ $BUILD_QEMU == true ];then
                qemu_build
            fi
            if [ $BUILD_YOCTO == true ];then
                yocto_build
            fi
            if [ $BUILD_ROOTFS == true ];then
                kernel_build
                pk_build
            fi
        fi
    fi
}

function toolchain_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                      ToolChain Build                               \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"
    ${TOOLS_PATH}/riscv-tools-setup.sh
}

function qemu_build()
{
    echo -e "\n\033[46;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[46;30m                         QEMU Build                                 \033[0m"
    echo -e "\033[46;30m ------------------------------------------------------------------ \033[0m"
    ${TOOLS_PATH}/riscv-qemu-setup.sh
}

function bl1_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                         Bl1 Build                                  \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"
    pushd ${BL1_PATH}/bl1
    ./run.sh
    popd

    pushd ${BL1_PATH}/vector
    ./run.sh
    popd    
}

function pk_build()
{
    echo -e "\n\033[46;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[46;30m                         BBL(PK) Build                              \033[0m"
    echo -e "\033[46;30m ------------------------------------------------------------------ \033[0m"

    pushd ${PK_PATH}

    if [ $CLEAN_BUILD == true ];then
        rm -rf build
        mkdir -p build
    else
        if [ ! -d ${PK_PATH}/build ];then
	    mkdir -p build
	fi
    fi

    pushd build

    if [ $PK_PAYLOAD_ENABLE == true ]; then
        ../configure --prefix=$RISCV --host=riscv64-unknown-elf --enable-logo --with-logo=${TOOLS_PATH}/conf/nexell_logo.txt --with-payload=${KERNEL_PATH}/vmlinux
    else
        ../configure --prefix=$RISCV --host=riscv64-unknown-elf
    fi

    make clean
    make

    riscv64-unknown-elf-objcopy -O binary bbl bbl.bin
    riscv64-unknown-elf-objdump -D bbl > bbl.dump
    #    make install

    popd #build
    popd #riscv-pk
}

function kernel_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                         Kernel Build                               \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    check_ramdisk

    pushd ${KERNEL_PATH}

    if [ $CLEAN_BUILD == true ];then
        make clean
    fi

    make ARCH=${KERNEL_ARCH} swallow_${BOARD_NAME}_defconfig

    make CONFIG_INITRAMFS_SOURCE="${BUILDROOT_CONF_PATH}/initramfs.txt ${ROOTFS_PATH}" \
         CONFIG_INITRAMFS_ROOT_UID=1000 \
         CONFIG_INITRAMFS_ROOT_GID=1000 \
         ARCH=${KERNEL_ARCH} CROSS_COMPILE=${RISCV}/bin/riscv64-unknown-elf- vmlinux -j8

    popd
}

function check_ramdisk()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                        check rootfs                                \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    if [ -e ${BUILD_PATH}/${RAMDISK_FILE} ];then
	echo " Exist rootfs file by yocto build"

	if [ -d ${BUILD_PATH}/sysroot ];then
	    echo "Exist sysroot directory"
	else
	    mkdir -p ${BUILD_PATH}/sysroot
	fi
	ROOTFS_PATH=`readlink -ev ${BUILD_PATH}/sysroot/`
	cp ${BUILD_PATH}/${RAMDISK_FILE} ${ROOTFS_PATH}
	
	pushd ${ROOTFS_PATH}
	zcat ${RAMDISK_FILE} | cpio -idmv
	rm ${RAMDISK_FILE}
	popd

    else
	echo " Does not exist yocto build rootfs file "
	echo " Using default rootfs file"

	pushd ${DEFAULT_ROOTFS_PATH}

        if [ -e rootfs.tar.gz ];then
	    echo "rootfs exist"
	    if [ -e ${RAMDISK_FILE} ];then
		echo "${RAMDISK_FILE} exist"
	    else
		echo "doest not compatible file"
		tar xzf rootfs.tar.gz
	    fi

            if [ -d ${DEFAULT_ROOTFS_PATH}/sysroot ];then
	        echo "Exist sysroot directory"
	    else
	        mkdir -p ${DEFAULT_ROOTFS_PATH}/sysroot
	    fi

	    ROOTFS_PATH=`readlink -ev ${DEFAULT_ROOTFS_PATH}/sysroot/`
            cp ${DEFAULT_ROOTFS_PATH}/${RAMDISK_FILE} ${ROOTFS_PATH}
            cd ${ROOTFS_PATH}
            zcat ${RAMDISK_FILE} | cpio -idmv
            rm ${RAMDISK_FILE}
            cd ..
	else
	    echo -e "\033[42;30m       ERROR ::   rootfs.tar.gz does not exist               \033[0m"
	fi

	popd
    fi
}

function dtb_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                         dtb Build                               \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    pushd ${KERNEL_PATH}

    make ARCH=${KERNEL_ARCH} CROSS_COMPILE=${RISCV}/bin/riscv64-unknown-elf- dtbs -j8

    popd
}

function yocto_build()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                         yocto Build                                \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    pushd ${YOCTO_POKY_PATH}

    if [ $SDK_BUILD == true ];then
        source oe-init-build-env ${YOCTO_PATH}/build_sdk
        bitbake -c populate_sdk core-image-riscv
    else
        source oe-init-build-env ${YOCTO_PATH}/build
        bitbake core-image-riscv
    fi

    popd

    if [ ${FIRST_BUILD} == true ];then
        echo -e "\033[45;30m     Copy yocto rootfs ---->        \033[0m"
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-initramfs-riscv64.cpio.gz ${BUILD_PATH}/${RAMDISK_FILE}
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-riscv64.cpio.gz ${BUILD_PATH}/rootfs.cpio.gz
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-riscv64.ext2 ${BUILD_PATH}/rootfs.ext2
    fi
}

function move_images()
{
    echo -e "\n\033[46;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[46;30m                         Move Images                                \033[0m"
    echo -e "\033[46;30m ------------------------------------------------------------------ \033[0m"

    if [ $BUILD_ALL == true -o $BUILD_BL1 == true ];then
        echo -e "\033[45;30m     Copy bl1.bin ---->        \033[0m"
        cp ${BL1_PATH}/bl1/build/bl1.bin ${BUILD_PATH}
 
        echo -e "\033[45;30m     Copy vector.bin ---->        \033[0m"
        cp ${BL1_PATH}/vector/build/vector.bin ${BUILD_PATH}
    fi
    if [ $BUILD_ALL == true -o $BUILD_KERNEL == true -o $BUILD_ROOTFS == true ];then
        echo -e "\033[45;30m     Copy vmlinux ---->        \033[0m"
        cp ${KERNEL_PATH}/vmlinux ${BUILD_PATH}
    fi
    if [ $BUILD_ALL == true -o $BUILD_PK == true -o $BUILD_ROOTFS == true ];then
        echo -e "\033[45;30m     Copy bbl & bbl.bin ---->        \033[0m"
        cp ${PK_PATH}/build/bbl.bin ${BUILD_PATH}
        cp ${PK_PATH}/build/bbl ${BUILD_PATH}
    fi
    if [ $BUILD_ALL == true -o $BUILD_DTB == true ];then
        echo -e "\033[45;30m     Copy dtb ---->        \033[0m"
        cp ${DTB_PATH}/swallow-${BOARD_NAME}.dtb ${BUILD_PATH}
    fi
    if [ $BUILD_YOCTO == true -o $FIRST_BUILD == false ];then
        echo -e "\033[45;30m     Copy yocto rootfs ---->        \033[0m"
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-initramfs-riscv64.cpio.gz ${BUILD_PATH}/${RAMDISK_FILE}
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-riscv64.cpio.gz ${BUILD_PATH}/rootfs.cpio.gz
        cp ${YOCTO_PATH}/build/tmp/deploy/images/riscv64/core-image-riscv-riscv64.ext2 ${BUILD_PATH}/rootfs.ext2
    fi
}

function convert_images()
{
    echo -e "\n\033[45;30m ------------------------------------------------------------------ \033[0m"
    echo -e "\033[45;30m                         Convert Images                             \033[0m"
    echo -e "\033[45;30m ------------------------------------------------------------------ \033[0m"

    pushd ${TOOLS_PATH}/bootgen

    ./makebingen.sh ${BOARD_NAME} dos ${BUILD_PATH}

    popd
}

function check_board_name()
{
    local board_name=${1}

    if [ -z ${board_name} ]; then
	echo -e "\n\033[45;30m ---------------------------------- \033[0m"
	echo -e "\033[45;30m Please insert Boardname(-b option) \033[0m"
	echo -e "\033[45;30m ---------------------------------- \033[0m"
	usage; exit 1
    fi
}

parse_args $@
check_board_name ${BOARD_NAME}
do_build
move_images
convert_images
