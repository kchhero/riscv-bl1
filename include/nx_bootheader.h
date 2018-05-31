/*
 * Copyright (C) 2012 Nexell Co., All Rights Reserved
 * Nexell Co. Proprietary & Confidential
 *
 * NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 * AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * Module	: bl0
 * File		: nx_antares_bootheader.h
 * Description	: This must be synchronized with NSIH.txt
 * Author	: hans
 * History	: 2017.10.09 create
 */

#ifndef __NX_BOOTHEADER_H__
#define __NX_BOOTHEADER_H__

struct nx_bootinfo {
    unsigned int vector[8];				/* 0x000 ~ 0x01F */
    unsigned int vector_rel[8];		        	/* 0x020 ~ 0x03F */

    unsigned int LoadSize;				/* 0x040 */
    unsigned int CRC32;		         		/* 0x044 */
    unsigned int LoadAddr;				/* 0x048 */
    unsigned int StartAddr;				/* 0x04C */

    unsigned char _reserved3[176];		        /* 0x050 ~ 0x0FF */
    
    unsigned int dtbSize;                               /* 0x100 */
    unsigned int *dtbLoadAddr;                          /* 0x104 */
    
    unsigned char _reserved4[236];      		/* 0x108 ~ 0x1f7 */

    /* version */
    unsigned int buildinfo;				/* 0x1f8 */

    /* "NSIH": nexell system infomation header */
    unsigned int signature;				/* 0x1fc */
} __attribute__ ((packed, aligned(16)));

struct nx_bootmm {
    struct nx_bootinfo bi;
    unsigned int image[16];
};

/* ========================= rst cfg =====================*/
#define SDBOOT			0
#define SPIBOOT			1

#define UARTBAUDRATE		3 // 0: 115200, 1:921600
#define UARTPORT		4 // 5, 6	// 0 ~ 6 port

/*
 * EXNOBOOTMSG	NOBOOTMSG
 *	0	    0		uart out
 *	0	    1		memory save
 *	1	    0		memory save
 *	1	    1		no message out
 */

#endif
