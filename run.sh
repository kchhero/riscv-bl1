make clean
make
riscv64-unknown-elf-objdump -D build/bl1.elf > build/bl1.dump
