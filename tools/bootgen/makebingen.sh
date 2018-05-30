#!/bin/bash

argc=$#
BINTYPE=$1

if [ $argc -lt 1 ]
then
    echo "Invalid argument check usage please"
    exit
fi

#nsih.txt modified for bl1.bin size compatible
python2.7 nsihtxtmod.py

#create sdboot.bin, select type gpt or dos
python2.7 bootbingen.py ${BINTYPE}

#create sdboot.bin 
#dd if=/dev/zero of=sdboot.bin bs=512 count=1

#Add nsih-bl1.bin to sdboot.bin
dd if=nsih-bl1.bin bs=512 >> sdboot.bin

#Add bl1.bin to sdboot.bin
dd if=bl1.bin >> sdboot.bin

#Add vector.bin to sdboot.bin
dd if=vector.bin bs=4K >> sdboot.bin

#Add nsih-bbl.bin to sdboot.bin
dd if=nsih-bbl.bin bs=512 >> sdboot.bin

#Add bbl binary to sdboot.bin
dd if=bbl.bin >> sdboot.bin

#Convert from ascii to hex
python2.7 converthex.py

mv sdboot.hex sdboot_${BINTYPE}.hex

cp sdboot_dos.hex ~/RISC-V/nexell/soc-reference/SOC/board/drone_soc/chip/drone_soc/evt0/design/RISCV/sim/l4_rtl/bootrom_test/compile/build/
cp sdboot_gpt.hex ~/RISC-V/nexell/soc-reference/SOC/board/drone_soc/chip/drone_soc/evt0/design/RISCV/sim/l4_rtl/bootrom_test/compile/build/

cp sdboot_dos.hex ~/RISC-V/nexell/soc-reference/SOC/board/drone_soc/chip/drone_soc/evt0/common/common_verif/common_code/l4_sim/bootcodegen/
cp sdboot_gpt.hex ~/RISC-V/nexell/soc-reference/SOC/board/drone_soc/chip/drone_soc/evt0/common/common_verif/common_code/l4_sim/bootcodegen/

