//#include "nx_qemu_sim_printf.h"

/* #define csr_read(csr)						\ */
/* ({								\ */
/* 	register unsigned long __v;				\ */
/* 	__asm__ __volatile__ ("csrr %0, " #csr			\ */
/* 			      : "=r" (__v) :			\ */
/* 			      : "memory");			\ */
/* 	__v;							\ */
/* }) */

int main()
{
    volatile unsigned char *reg = (unsigned char*)0x20A00000;//PHY_BASEADDR_DUMMY_MODULE;

    *reg = 'v';
    *reg = 'e';
    *reg = 'c';
    /* unsigned char test[32] = "vector table test\n"; */
    /* int i = 0; */
    /* while(test[i] != '\0') { */
    /*     *reg = test[i]; */
    /*     i++; */
    /* } */
    /* _dprintf("vector run\n"); */
    /* _dprintf("pc addr = 0x%x\n",csr_read(mepc)); */

    return 0;
}
