#include <nx_cpuif_regmap.h>
#include <nx_swallow_platform.h>
#if defined(QEMU_RISCV) || defined(SOC_SIM)
#include <nx_qemu_sim_printf.h>
#else
#include <nx_swallow_printf.h>
#endif

void nx_cpuif_reg_write_one(__nx_cpuif_symbol__ symbol, unsigned int  regval)
{
    unsigned int *reg_addr;
    unsigned int reg_val;
    volatile unsigned int * reg;

    unsigned int reg_writeval;
    unsigned int reg_mask;
    unsigned int reg_startbit;
    unsigned int reg_bitwidth;
    unsigned int masked_writeval;

    //reg_addr     = (unsigned int*)(*symbol.baseaddr + symbol.offset);
    reg_addr     = (unsigned int*)(symbol.baseaddr + symbol.offset);    
    reg_startbit = symbol.startbit;
    reg_bitwidth = symbol.bitwidth;

    reg_writeval = regval << reg_startbit;

    reg = (unsigned int*) reg_addr;

    if( reg_bitwidth == 32 ) {
        reg_val  = 0;
    } else {
    //        reg_val  = *reg;
        reg_val  = ReadIODW(reg);
    }

    reg_mask = reg_bitwidth < 32 ? ((1<<(reg_bitwidth))-1) << reg_startbit : ~0;

    masked_writeval = (reg_bitwidth < 32) ? regval & ((1<<(reg_bitwidth))-1) : regval ;
    reg_writeval = masked_writeval << reg_startbit;

#if 0//defined(DEBUG) && !defined(SOC_SIM)
    _dprintf("<<write_reg>>[DEBUG]----------------------------------------------\r\n");
    _dprintf("<<write_reg>>[DEBUG] symbol.baseaddr = 0x%x\r\n", symbol.baseaddr );
    _dprintf("<<write_reg>>[DEBUG] symbol.baseaddr3 = 0x%x\r\n", symbol.baseaddr+symbol.offset );
    _dprintf("<<write_reg>>[DEBUG] reg_addr        = 0x%x\r\n", reg_addr     );    
    _dprintf("<<write_reg>>[DEBUG] reg_bitwidth    = 0x%x\r\n", reg_bitwidth );
    _dprintf("<<write_reg>>[DEBUG] masked_writeval = 0x%x\r\n", masked_writeval );
    _dprintf("<<write_reg>>[DEBUG]----------------------------------------------\r\n");
#endif

    reg_val = reg_val & (~reg_mask);
    reg_val = reg_val | reg_writeval;

    //*reg = (unsigned int)reg_val;
    WriteIODW(reg, reg_val);
#if 0//defined(DEBUG) && !defined(SOC_SIM)
    _dprintf("<<write_reg>>[DEBUG]----------------------------------------------\r\n");    
    _dprintf("<<write_final>>[DEBUG] reg = 0x%x, regval = 0x%x\r\n", reg, reg_val);
    _dprintf("<<write_after>>[DEBUG] reg = 0x%x, regval = 0x%x\r\n", reg, ReadIODW(reg));
    _dprintf("<<write_reg>>[DEBUG]----------------------------------------------\r\n");    
#endif
}

unsigned int nx_cpuif_reg_read_one(__nx_cpuif_symbol__ symbol, unsigned int * regval )
{
    unsigned int * reg_addr;
    unsigned int reg_val;
    volatile unsigned int * reg;

    unsigned int reg_readval;
    unsigned int reg_mask;
    unsigned int reg_startbit;
    unsigned int reg_bitwidth;

    //reg_addr     = (unsigned int*)(*symbol.baseaddr + symbol.offset);
    reg_addr     = (unsigned int*)(symbol.baseaddr + symbol.offset);
    reg_startbit = symbol.startbit;
    reg_bitwidth = symbol.bitwidth;

    reg = (unsigned int*) reg_addr;
    reg_val  = *reg;//ReadIODW(reg);

    reg_mask = reg_bitwidth < 32 ? ((1<<(reg_bitwidth))-1) << reg_startbit : ~0;

    reg_readval = (reg_val & reg_mask) >> reg_startbit;
    reg_val = reg_readval;

    if( regval != NULL ) {
        *regval = reg_val;
    }

    //NX_ASSERT( reg_bitwidth != 0 );

#if 0//defined(DEBUG) && !defined(SOC_SIM)
    _dprintf("<<read_reg>>[DEBUG]----------------------------------------------\r\n");
    _dprintf("<<read_reg>>[DEBUG] reg_addr     = 0x%x\r\n", reg_addr     );
    _dprintf("<<read_reg>>[DEBUG] reg_startbit = 0x%x\r\n", reg_startbit );
    _dprintf("<<read_reg>>[DEBUG] reg_bitwidth = 0x%x\r\n", reg_bitwidth );
    _dprintf("<<read_reg>>[DEBUG] reg_mask     = 0x%x\r\n", reg_mask     );
    _dprintf("<<read_reg>>[DEBUG] reg_readval  = 0x%x\r\n", reg_readval  );
    _dprintf("<<read_reg>>[DEBUG] reg_val      = 0x%x(%d)\r\n", reg_val, reg_val);
    _dprintf("<<read_reg>>[DEBUG]----------------------------------------------\r\n");
#endif
    return reg_readval;
}

