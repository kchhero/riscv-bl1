#ifndef __NX_CHIP_SFR__H__
#define __NX_CHIP_SFR__H__

#include "nx_const.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#define NULL 0

#define CLINT_CTRL_ADDR _AC(0x2000000,UL)
#define CLINT_CTRL_SIZE _AC(0x10000,UL)

#define ReadIODW(Addr) (*(volatile unsigned int*)(Addr))
#define ReadIOW(Addr) (*(volatile unsigned short*)(Addr))
#define ReadIOB(Addr) (*(volatile unsigned char*)Addr)
#define WriteIODW(Addr,Data) (*(volatile unsigned int*)Addr)=((unsigned int)Data)
#define WriteIOW(Addr,Data) (*(volatile unsigned short*)Addr)=((unsigned short)Data)
#define WriteIOB(Addr,Data) (*(volatile unsigned char*)Addr)=((unsigned char)Data)

#define mmio_read_32(addr)              (*(volatile unsigned int  *)(addr))
#define mmio_read_16(addr)              (*(volatile unsigned short*)(addr))
#define mmio_read_8(addr)               (*(volatile unsigned char *)(addr))

#define mmio_write_32(addr,data)        (*(volatile unsigned int  *)(addr))  =  ((unsigned int  )(data))
#define mmio_write_16(addr,data)        (*(volatile unsigned short*)(addr))  =  ((unsigned short)(data))
#define mmio_write_8(addr,data)         (*(volatile unsigned char *)(addr))  =  ((unsigned char )(data))

#define mmio_set_32(addr,data)          (*(volatile unsigned int  *)(addr)) |=  ((unsigned int  )(data))
#define mmio_set_16(addr,data)          (*(volatile unsigned short*)(addr)) |=  ((unsigned short)(data))
#define mmio_set_8(addr,data)           (*(volatile unsigned char *)(addr)) |=  ((unsigned char )(data))

#define mmio_clear_32(addr,data)        (*(volatile unsigned int  *)(addr)) &= ~((unsigned int  )(data))
#define mmio_clear_16(addr,data)        (*(volatile unsigned short*)(addr)) &= ~((unsigned short)(data))
#define mmio_clear_8(addr,data)         (*(volatile unsigned char *)(addr)) &= ~((unsigned char )(data))
//--------------------------------------------------------------------------


#ifdef	__cplusplus
}
#endif

#endif
