OUTPUT_ARCH("riscv")

ENTRY(_start_vec)
 
SECTIONS
{
        . = 0x40007000;
        . = ALIGN(4);
        .text :
        {
                obj/head.o (.text)
                obj/main.o (.text)
                obj/nx_*.o (.text)
                *(.init.text)
                *(.init.data)
                *(.init.code)
                *(.text)
        }

        . = ALIGN(4);
        .rodata : { *(.rodata*) }

        . = ALIGN(4);
        .data : { *(.data*) }

        . = ALIGN(4);
        .got : { *(.got) }

        _end = .;

        __bss_start__ = .;
        .bss_start (OVERLAY) : {
                KEEP(*(.__bss_start));
                __bss_base = .;
        }
        .bss __bss_base (OVERLAY) : {
                *(.bss*)
                . = ALIGN(4);
                __bss_limit = .;
        }
        .bss_end __bss_limit (OVERLAY) : {
                KEEP(*(.__bss_end));
        }
        __bss_end__ = .;
}
