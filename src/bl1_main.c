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
#include <nx_cpuif_regmap.h>
#include <nx_bootheader.h>
#include <iSDBOOT.h>
#ifdef MEMTEST
#include <memtester.h>
#endif
#ifdef SOC_SIM
#include <nx_qemu_sim_printf.h>
#else
#include <nx_swallow_printf.h>
#include <serial.h>
#endif

#include <nx_gpio.h>
#include <nx_chip_iomux.h>

unsigned int* bl1main()
{
    int result = 0;
    int nDDRC_init = 0;
#ifdef SOC_SIM
    _dprintf("bl1 enter---\n");
#endif

#if defined(MEMTEST) && defined(DEBUG) //early printf
   serial_init(0); //channel = 0
#endif

    //DDR1 memory initialize   
    nx_cpuif_reg_write_one(DDRC_REG_4, 0xC8A); // address : {bank, row, column} || OK
    udelay(500);
    //    nx_cpuif_reg_write_one(	DDRC_REG_8   , 0xC0C06   ); // reset default
    nx_cpuif_reg_write_one(DDRC_REG_8, 0x181414); // for 200MHz operation //B270
    udelay(500);
    //nx_cpuif_reg_write_one(	DDRC_REG_8   , 0x000B270); // for 200MHz operation //B270
    nx_cpuif_reg_write_one(DDRC_REG_C, 0x13443);
    udelay(500);
    
    nx_cpuif_reg_write_one(DDRC_REG_10, 0x618);
    udelay(500);
    //nx_cpuif_reg_write_one(DDRC_REG_0, 0x2); //clear
    nx_cpuif_reg_write_one(DDRC_REG_0, 0x9E81); //start
    udelay(500);

    nDDRC_init = nx_cpuif_reg_read_one(DDRC_REG_0, 0);
    while(0 == nDDRC_init);

    if (nDDRC_init == 0x10) {
        _dprintf("==> DDRC init complete !! \r\n");
        _dprintf("==> read one = 0x%x \r\n",mmio_read_32((unsigned int*)PHY_BASEADDR_DDRC0_MODULE));
    }
    
#ifdef MEMTEST
#ifdef DEBUG
   _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
   _dprintf("DDRC_REG_50 = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_50, 0));
   _dprintf("DDRC_REG_54 = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_54, 0));
   _dprintf("DDRC_REG_58 = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_58, 0));
   _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");   
   _dprintf("simple_memtest run\r\n");
#endif
   udelay(2000);
   simple_memtest2();
#else

#ifdef DEBUG
   serial_init(0); //channel = 0

   _dprintf("bootloader~start\r\n");
   _dprintf("Bl1 Start \r\n");
#endif

   //Boot mode check and BBL+linux loading
   result = iSDBOOT();

   //reset vector test
#ifdef VECTOR_TEST
   volatile unsigned int* pCLINT1Reg = (unsigned int*)(0x02000004);
   _dprintf("CPU1 wake-up\n");
   *pCLINT1Reg = 0x1;
   return 0;
#else
#ifdef DEBUG
    _dprintf(">> bl1 boot result = 0x%x <<\n",result);
#endif
    
    if (result) {
#ifdef DEBUG
        _dprintf(">> Launch to 0x%x\n", BASEADDR_DRAM);
#endif
        return (unsigned int*)(BASEADDR_DRAM);
    }
#endif //VECTOR_TEST
#endif //MEMTEST

    while(1);
    return 0;
}
