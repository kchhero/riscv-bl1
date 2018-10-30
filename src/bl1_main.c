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
#elif MEMTEST2
#include <memtester2.h>
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
    unsigned int ddr_phy_config = 0x00000000;
    unsigned int ddr_addr_size = 0x00000001;
    unsigned int ddr_timing_0 = 0x000C0C06;
    unsigned int ddr_timing_1 = 0x00623233;
    unsigned int ddr_timing_2 = 0x753008B3;
    unsigned int ddr_lmr_ext_std = 0x00000000;
    unsigned int ddr_cfg_rdq_latency = 0x00000000;
    unsigned int ddr_cfg_rdq_dlyval = 0x00000000;

#ifdef SOC_SIM
    _dprintf("bl1 enter---\n");
#endif

#if defined(MEMTEST) && defined(DEBUG) //early printf
   serial_init(0); //channel = 0
   _dprintf("\r\n\r\n\r\n\r\n\r\nbl1 enter---\r\n");
#endif

    //**********************************************************************
    // Clock Change 200MHz -> 100MHz
    //**********************************************************************
    //1. OSC_MUXSEL to 0
    udelay(1000);
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");

#ifdef DEBUG
    _dprintf("[Before]PLL0 reg = 0x%08x\r\n", mmio_read_32((unsigned int*)(0x20010000)));
    _dprintf("[Before]FBDIV val = 0x%08x\r\n", mmio_read_32((unsigned int*)(0x20010030)));
#endif
    udelay(2000000); //2s
    mmio_write_32((volatile unsigned int)(0x20010000), mmio_read_32((unsigned int*)(0x20010000)) & 0xFFF7);
    //2-1. FBDIV -> 8
    mmio_write_32((volatile unsigned int)(0x20010030), (mmio_read_32((unsigned int*)(0x20010030)) & 0xFFFF00FF) | 0x800);
    //2-2. GV -> 2
    mmio_write_32((volatile unsigned int)(0x20010030), (mmio_read_32((unsigned int*)(0x20010030)) & 0xFFFFFFF0) | 2);
    //3. wait 1ms
    udelay(1000);
    //4. OSC_MUXSEL to 1
    mmio_write_32((volatile unsigned int)(0x20010000), mmio_read_32((unsigned int*)(0x20010000)) | 0x8);
#ifdef DEBUG
    _dprintf("[After]PLL0 reg = 0x%08x\r\n", mmio_read_32((unsigned int*)(0x20010000)));
    _dprintf("[After]FBDIV val = 0x%08x\r\n", mmio_read_32((unsigned int*)(0x20010030)));
#endif
 
    //**********************************************************************
    //-------------------------------------------------------------
    //DDR1 memory initialize
    //-------------------------------------------------------------
    // 1. DDR_ADDR_SIZE set
    //-------------------------------------------------------------
    /* ddr_addr_size = (DDR_ADDR_SIZE_COL_SIZE    | \ */
    /*                  DDR_ADDR_SIZE_ROW_SIZE    | \ */
    /*                  DDR_ADDR_SIZE_BANK_SIZE   | \ */
    /*                  DDR_ADDR_SIZE_ADDR_MAP    | \ */
    /*                  DDR_ADDR_SIZE_SEL_MDDR    | \ */
    /*                  DDR_ADDR_SIZE_IO_STRENGTH | \ */
    /*                  DDR_ADDR_SIZE_IO_DRIVESEL | \ */
    /*                  DDR_ADDR_SIZE_IO_ODTSEL   | \ */
    /*                  DDR_ADDR_SIZE_IO_ODTEN      \ */
    /*                  ); */
    ddr_addr_size = DDR_ADDR_SIZE_VAL;
    mmio_write_32((volatile unsigned int)DDRC_REG_ADDR_SIZE_ADDR, ddr_addr_size);
    //nx_cpuif_reg_write_one(DDRC_REG_ADDR_SIZE, DDR_ADDR_SIZE_VAL);
#ifdef DEBUG
    _dprintf("DDR_ADDR_SIZE = 0x%08x\r\n", mmio_read_32((unsigned int*)DDRC_REG_ADDR_SIZE_ADDR));
#endif

    //-------------------------------------------------------------
    // 2. DDR_TIMING_0 set
    //-------------------------------------------------------------
    ddr_timing_0 = (DDR_TIMING_0_TRAS | \
                    DDR_TIMING_0_TRFC | \
                    DDR_TIMING_0_TRC \
                    );
    mmio_write_32((volatile unsigned int)DDRC_REG_TIMING0_ADDR, ddr_timing_0);
    //nx_cpuif_reg_write_one(DDRC_REG_TIMING0, ddr_timing_0);
#ifdef DEBUG
    _dprintf("DDR_TIMING_0 = 0x%08x\r\n",mmio_read_32((unsigned int*)DDRC_REG_TIMING0_ADDR));
#endif

    //-------------------------------------------------------------
    // 3. DDR_TIMING_1 set
    //-------------------------------------------------------------
    ddr_timing_1 = (DDR_TIMING_1_TRCD | \
                    DDR_TIMING_1_TRP | \
                    DDR_TIMING_1_TRRD | \
                    DDR_TIMING_1_TWR | \
                    DDR_TIMING_1_TWTR | \
                    DDR_TIMING_1_TMRD | \
                    DDR_TIMING_1_TDQSS \
                    );
    mmio_write_32((volatile unsigned int)DDRC_REG_TIMING1_ADDR, ddr_timing_1);
    //nx_cpuif_reg_write_one(DDRC_REG_TIMING1, ddr_timing_1);
#ifdef DEBUG
    _dprintf("DDR_TIMING_1 = 0x%08x\r\n",mmio_read_32((unsigned int*)DDRC_REG_TIMING1_ADDR));
#endif

    //-------------------------------------------------------------
    // 4. DDR_TIMING_2 set
    //-------------------------------------------------------------
    ddr_timing_2 = (DDR_TIMING_2_TREFR | \
                    DDR_TIMING_2_INIT_TIME \
                    );
    mmio_write_32((volatile unsigned int)DDRC_REG_TIMING2_ADDR, ddr_timing_2);
    //nx_cpuif_reg_write_one(DDRC_REG_TIMING2, ddr_timing_2); //0x618
#ifdef DEBUG
    _dprintf("DDR_TIMING_2 = 0x%08x\r\n",mmio_read_32((unsigned int*)DDRC_REG_TIMING2_ADDR));
#endif

    //-------------------------------------------------------------
    // 5. DDR_LMR_EXT_STD Mode Register
    //-------------------------------------------------------------
    ddr_lmr_ext_std = (DDR_LMR_EXT_STD_SLMR | \
		       DDR_LMR_EXT_STD_ELMR \
                      );
 
    mmio_write_32((volatile unsigned int)DDRC_REG_LMR_EXT_STD_ADDR, ddr_lmr_ext_std);
    //nx_cpuif_reg_write_one(DDRC_REG_LMR_EXT_STD, ddr_lmr_ext_std);
#ifdef DEBUG
    _dprintf("DDR_LMR_EXT_STD = 0x%08x\r\n", mmio_read_32((unsigned int*)DDRC_REG_LMR_EXT_STD_ADDR));
#endif

    /* __asm__ volatile ("nop"); */
    /* __asm__ volatile ("nop"); */
    /* __asm__ volatile ("nop"); */
    /* udelay(200); */

    //-------------------------------------------------------------
    // 7. DDR_CFG_RDQ_LATENCY set
    //-------------------------------------------------------------
    ddr_cfg_rdq_latency = (DDR_CFG_RDQ_LATENCY_LATENCY | \
                           DDR_CFG_RDQ_LATENCY_ALIGN \
                           );
    //mmio_write_32((volatile unsigned int)(0x20050000)+0x1C, ddr_cfg_rdq_latency);
    nx_cpuif_reg_write_one(DDRC_REG_CFG_RDQ_LATENCY, ddr_cfg_rdq_latency);
#ifdef DEBUG
    _dprintf("DDR_CFG_RDQ_LATENCY val = 0x%08x\r\n",nx_cpuif_reg_read_one(DDRC_REG_CFG_RDQ_LATENCY, 0));
#endif

/*     //------------------------------------------------------------- */
/*     // 7. DDR_CFG_RDQ_DLYVAL set */
/*     //------------------------------------------------------------- */
    ddr_cfg_rdq_dlyval = (DDR_CFG_RDQ_DLYVAL_0 | \
 		          DDR_CFG_RDQ_DLYVAL_1 | \
 		          DDR_CFG_RDQ_DLYVAL_2 | \
 		          DDR_CFG_RDQ_DLYVAL_3 \
                          );
    //mmio_write_32((volatile unsigned int)(0x20050000)+0x20, ddr_cfg_rdq_dlyval);
    nx_cpuif_reg_write_one(DDRC_REG_CFG_RDQ_DLYVAL, ddr_cfg_rdq_dlyval);
#ifdef DEBUG
    _dprintf("DDR_CFG_RDQ_DLYVAL val = 0x%08x\r\n",nx_cpuif_reg_read_one(DDRC_REG_CFG_RDQ_DLYVAL, 0));
#endif
    
    //-------------------------------------------------------------
    // 8. DDR_PHY_CONFIG set
    //-------------------------------------------------------------
    //nx_cpuif_reg_write_one(DDRC_REG_0, 0x9E81); //start

    ddr_phy_config = (DDR_PHY_CONFIG_START /* | */
                      /* DDR_PHY_CONFIG_DIS_PWRSEQ  \ */
                      /* DDR_PHY_CONFIG_CLEAR |  */                      
                      /* DDR_PHY_CONFIG_DIS_LATENCY | \ */
                      /* DDR_PHY_CONFIG_DIS_VERIFY  \ */
                      /* DDR_PHY_CONFIG_DIS_CALIB \ */
                      );
                      /* DDR_PHY_CONFIG_DIS_RDQCPT | \ */
                      /* DDR_PHY_CONFIG_DIS_ALIGN | \ */
                      /* DDR_PHY_CONFIG_ERR_IGNORE \ */

    mmio_write_32((volatile unsigned int)DDRC_REG_PHY_CONFIG_ADDR, ddr_phy_config);
    //nx_cpuif_reg_write_one(DDRC_REG_PHY_CONFIG, ddr_phy_config);
#ifdef DEBUG
    _dprintf("DDR_PHY_CONFIG val = 0x%08x\r\n",mmio_read_32((unsigned int*)DDRC_REG_PHY_CONFIG_ADDR));
#endif

    udelay(30000);
    do {
	    nDDRC_init = nx_cpuif_reg_read_one(DDRC_REG_PHY_CONFIG, 0);
            udelay(1000000);
#ifdef DEBUG
            _dprintf("nDDRC_init loop = 0x%08x\r\n",nDDRC_init);
            _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
            _dprintf("======================= DDRC_REG DUMP =======================\r\n");
            _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
            _dprintf("DDRC_REG_CFG_CMD_DLYVAL    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_24, 0));
            _dprintf("DDRC_REG_STA_RDQ_LATENCY   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_40, 0));
            _dprintf("DDRC_REG_STS_RDQ_DLYVAL    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_48, 0));
            _dprintf("DDRC_REG_STS_CMD_DLYVAL    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_4C, 0));
            _dprintf("DDRC_REG_STS_VERF_ERRPOS   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_50, 0));
            _dprintf("DDRC_REG_STS_VERF_ERR_L    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_54, 0));
            _dprintf("DDRC_REG_STS_VERF_ERR_H    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_58, 0));
            _dprintf("DDRC_REG_RISE_DQ_0-7       = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_60, 0));
            _dprintf("DDRC_REG_RISE_DQ_8-15      = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_64, 0));
            _dprintf("DDRC_REG_RISE_DQ_16-23     = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_68, 0));
            _dprintf("DDRC_REG_RISE_DQ_24-31     = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_6C, 0));
            _dprintf("DDRC_REG_FALL_DQ_0-7       = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_70, 0));
            _dprintf("DDRC_REG_FALL_DQ_8-15      = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_74, 0));
            _dprintf("DDRC_REG_FALL_DQ_15-23     = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_78, 0));
            _dprintf("DDRC_REG_FALL_DQ_23-31     = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_7C, 0));
            _dprintf("DDRC_REG_DQ_Beat_Sts-0-7   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_80, 0));
            _dprintf("DDRC_REG_DQ_Beat_Sts-8-15  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_84, 0));
            _dprintf("DDRC_REG_DQ_Beat_Sts-16-23 = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_88, 0));
            _dprintf("DDRC_REG_DQ_Beat_Sts-24-31 = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_8C, 0));
            _dprintf("DDRC_REG_DQ_Byte_L0-0-7    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_90, 0));
            _dprintf("DDRC_REG_DQ_Byte_L0-8-15   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_94, 0));
            _dprintf("DDRC_REG_DQ_Byte_L0-16-23  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_98, 0));
            _dprintf("DDRC_REG_DQ_Byte_L0-24-31  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_9C, 0));
            _dprintf("DDRC_REG_DQ_Byte_L1-0-7    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_A0, 0));
            _dprintf("DDRC_REG_DQ_Byte_L1-8-15   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_A4, 0));
            _dprintf("DDRC_REG_DQ_Byte_L1-16-23  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_A8, 0));
            _dprintf("DDRC_REG_DQ_Byte_L1-24-31  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_AC, 0));
            _dprintf("DDRC_REG_DQ_Byte_L2-0-7    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_B0, 0));
            _dprintf("DDRC_REG_DQ_Byte_L2-8-15   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_B4, 0));
            _dprintf("DDRC_REG_DQ_Byte_L2-16-23  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_B8, 0));
            _dprintf("DDRC_REG_DQ_Byte_L2-24-31  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_BC, 0));
            _dprintf("DDRC_REG_DQ_Byte_L3-0-7    = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_C0, 0));
            _dprintf("DDRC_REG_DQ_Byte_L3-8-15   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_C4, 0));
            _dprintf("DDRC_REG_DQ_Byte_L3-16-23  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_C8, 0));
            _dprintf("DDRC_REG_DQ_Byte_L3-24-31  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_CC, 0));
            _dprintf("DDRC_REG_D0   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_D0, 0));
            _dprintf("DDRC_REG_D4   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_D4, 0));
            _dprintf("DDRC_REG_D8   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_D8, 0));
            _dprintf("DDRC_REG_DC   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_DC, 0));
            _dprintf("DDRC_REG_E0   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_E0, 0));
            _dprintf("DDRC_REG_E4   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_E4, 0));
            _dprintf("DDRC_REG_E8   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_E8, 0));
            _dprintf("DDRC_REG_EC   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_EC, 0));
            _dprintf("DDRC_REG_F0   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_F0, 0));
            _dprintf("DDRC_REG_F4   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_F4, 0));
            _dprintf("DDRC_REG_F8   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_F8, 0));
            _dprintf("DDRC_REG_FC   = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_FC, 0));
            _dprintf("DDRC_REG_100  = 0x%x\r\n", nx_cpuif_reg_read_one(DDRC_REG_100, 0));
            _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
            _dprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
#endif
    } while(!((nDDRC_init & DDR_PHY_CONFIG_COMPLETE)>>4));

#ifdef DEBUG
    _dprintf("nDDRC_init = 0x%x \r\n",nDDRC_init);
    if ((nDDRC_init & DDR_PHY_CONFIG_COMPLETE) >> 4) {
        _dprintf("==> DDRC init complete !! \r\n");
        _dprintf("==> read one = 0x%x \r\n",mmio_read_32((unsigned int*)PHY_BASEADDR_DDRC0_MODULE));
    }
    
    nDDRC_init = nx_cpuif_reg_read_one(DDRC_REG_PHY_CONFIG, 0);
    if ((nDDRC_init & DDR_PHY_CONFIG_SUCCESS) >> 5) {
        _dprintf("==> DDRC init Success !! \r\n");
        _dprintf("==> read one = 0x%x \r\n",mmio_read_32((unsigned int*)PHY_BASEADDR_DDRC0_MODULE));
    } else {
    	_dprintf("==> DDRC init Success Fail !!! \r\n");
        _dprintf("==> read one = 0x%x \r\n",mmio_read_32((unsigned int*)PHY_BASEADDR_DDRC0_MODULE));
    }
#endif
   
#ifdef MEMTEST
   /* __asm__ volatile ("nop"); */
   udelay(100000);
   if (simple_memtest_8bit() == 0) {
        _dprintf("memtest 8b end\r\n");
        //        return ;
   }
   /* if (simple_memtest_16bit() == 0) { */
   /* 	_dprintf("memtest 16b end\r\n"); */
   /*      return ; */
   /* } */
   /* if (simple_readmem() == 0) { */
   /* 	_dprintf("read memtest end\r\n"); */
   /*      return ; */
   /* } */
   /* if (simple_memtest_32bit() == 0) { */
   /* 	_dprintf("memtest 32b end\r\n"); */
   /*      return ; */
   /* } */
   /* if (simple_memtest_burst() == 0) { */
   /* 	_dprintf("memtest burst end\r\n"); */
   /*      return ; */
   /* } */

#else //MEMTEST

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
   _dprintf("CPU1 wake-up\r\n");
   *pCLINT1Reg = 0x1;
   return 0;
#else
#ifdef DEBUG
    _dprintf(">> bl1 boot result = 0x%x <<\r\n",result);
#endif
    
    if (result) {
#ifdef DEBUG
        _dprintf(">> Launch to 0x%x\r\n", BASEADDR_DRAM);
        _dprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        _dprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        _dprintf(">> Kernel Start Ready!!!\r\n");
        _dprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        _dprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        _dprintf("\r\n\r\n\r\n");
        _dprintf("   Waiting for Dram Autocalibration...");
        _dprintf("   Waiting ...");
#endif
        return (unsigned int*)(BASEADDR_DRAM);
    }
#endif //VECTOR_TEST
#endif //MEMTEST

    while(1);
    return 0;
}
