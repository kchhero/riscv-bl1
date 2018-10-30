#!/bin/bash

#-----------------------------
#   vmlinux + BBL
#-----------------------------
#   NSIH of BBL   -  512Byte
#-----------------------------
#   DTB Binary
#-----------------------------
#   Vector Binary -  4KB
#-----------------------------
#   BL1 Binary
#-----------------------------
#   NSIH of BL1   -  512Byte
#-----------------------------

argc=$#
BOARD_NAME=$1
BINTYPE=$2
BINPATH=`readlink -ev $3`

SDBOOT_BIN="${BINPATH}/sdboot.bin"

if [ $argc -lt 1 ]
then
    echo "Invalid argument check usage please"
    exit
fi

#nsih.txt modified for bl1.bin size compatible
python2.7 nsihtxtmod.py ${BOARD_NAME} ${BINPATH}

#create sdboot.bin, select type gpt or dos
python2.7 bootbingen.py ${BINTYPE} ${BINPATH}

#create sdboot.bin
#dd if=/dev/zero of=sdboot.bin bs=512 count=1

#Add nsih-bl1.bin to sdboot.bin
dd if=${BINPATH}/nsih-bl1.bin bs=512 >> ${SDBOOT_BIN}

#Add bl1.bin to sdboot.bin
dd if=${BINPATH}/bl1.bin >> ${SDBOOT_BIN}

#Add vector.bin to sdboot.bin
dd if=${BINPATH}/vector.bin bs=4K >> ${SDBOOT_BIN}

#Add DTB binary(swallow.dtb) to sdboot.bin
dd if=${BINPATH}/swallow-${BOARD_NAME}.dtb >> ${SDBOOT_BIN}

#Add nsih-bbl.bin to sdboot.bin
dd if=${BINPATH}/nsih-bbl.bin bs=512 >> ${SDBOOT_BIN}

#Add bbl binary to sdboot.bin
dd if=${BINPATH}/bbl.bin >> ${SDBOOT_BIN}

#Convert from ascii to hex
#---- Use simulation ----
# python2.7 converthex.py ${BINPATH}
# mv ${BINPATH}/sdboot.hex ${BINPATH}/sdboot_${BINTYPE}.hex
#------------------------
