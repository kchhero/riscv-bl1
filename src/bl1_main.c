////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
// Nexell Co. Proprietary & Confidential
//
// Nexell informs that this code and information is provided "as is" base
// and without warranty of any kind, either expressed or implied, including
// but not limited to the implied warranties of merchantability and/or
// fitness for a particular puporse.
//
// Module	:
// File		: iROMBOOT.c
// Description	:
// Author	: Hans
// History	:
//		2014-01-12	Hans create
//		2017-10-09	Hans modified for NXP3220
//		2018-01-15	Hans sim done
////////////////////////////////////////////////////////////////////////////////
#include <nx_swallow.h>
#include <nx_qemu_sim_printf.h>
#include <nx_cpuif_regmap.h>
#include <nx_bootheader.h>
//#ifdef MEMTEST
//#include "./test/memtester.h"
#include <memtester.h>
//#endif
/* void __riscv_synch_thread(void) { */
/*     __asm__ __volatile__ ("fence" : : : "memory"); */
/* } */

unsigned int* bl1main()
{
    //    volatile unsigned int* pCLINT0Reg = (unsigned int*)(0x02000000);
    int result = 0;
    //    volatile unsigned int* pCLINT1Reg = (unsigned int*)(0x02000004);
    _dprintf("bl1 enter---\n");

    /* _dprintf("CPU1 wake-up\n"); */
    /* *pCLINT1Reg = 0x1; */


    //uart init
    //debug print
    
    //DDR1 memory initialize
    _dprintf("DDR1 init start\n");
    nx_cpuif_reg_write_one(	DDRC_REG_4   , 0x8C   ); // address : {bank, row, column} || OK
    //nx_cpuif_reg_write_one(	DDRC_REG_8   , 0xC0C06   ); // reset default
    nx_cpuif_reg_write_one(	DDRC_REG_8   , 0x0B270   ); // for 200MHz operation
    nx_cpuif_reg_write_one(	DDRC_REG_0   , 0x1    );

    while(0 == nx_cpuif_reg_read_one(DDRC_REG_0, 0) );
   _dprintf("DDR1 init done\n");
    
    //Boot mode check and BBL+linux loading
   /* simple_memtest(); */
   /* while(1); */
   /* return ;    */
   result = iSDBOOT();
/*       __riscv_synch_thread(); */
    __asm__ __volatile__ ("fence.i" : : : "memory");
   
   /* volatile unsigned int* pCLINT1Reg = (unsigned int*)(0x02000004); */
   /* _dprintf("CPU1 wake-up\n"); */
   /* *pCLINT1Reg = 0x1; */

#ifdef DEBUG
    _dprintf(">> bl1 boot result = 0x%x <<\n",result);
#endif
    
    if (result) {
        //        struct nx_bootinfo *pbi = (struct nx_bootinfo *)BASEADDR_DRAM;
#ifdef DEBUG
        _dprintf(">> Launch to 0x%x\n", BASEADDR_DRAM);//pbi->StartAddr);
#endif
        //        __asm__ __volatile__ ("lifence" : : : "memory");
        return (unsigned int*)(BASEADDR_DRAM);//pbi->StartAddr; //0x40000200 */
        //        return 1;
    }

    while(1);
    return ;
}
