/*
 * Copyright (C) 2012 Nexell Co., All Rights Reserved
 * Nexell Co. Proprietary & Confidential
 *
 * NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 * AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Module	: uart
 * File		: nx_uart.h
 * Description	: uart register struct
 * Author	: Hans
 * History	: 2018.01.06 First draft
 */
#ifndef __NX_UART_H__
#define __NX_UART_H__

#define BIT(nr)				(1 << (nr))
#define BIT_N(val, nr)			(val << (nr))

#define RBR                     0x0             // Receiver Buffer Register
#define THR                     0x0             // Transmitter Buffer Register
#define DLL                     0x0             // Divisor Latch Low
#define THR                     0x0             // Transmit Holdng Register (Write-Only)
#define DLH                     0x4             // Divisor Latch High
#define IER                     0x4             // Interrupt Enable Reigster
#define FCR                     0x8             // FIFO Control Register
#define IIR                     0x8             // Interrupt Identification Register

#define LCR                     0xC             // Line Control Register
#define MCR                     0x10            // Modem Control Register
#define LSR                     0x14            // Line Status Register
#define MSR                     0x18            // Model Status Regsiter
#define SCR                     0x1C            // Scratch Register
#define LPDLL                   0x20            // Low Power Divisor Latch Low
#define LPDLH                   0x24            // Low Power Divisor Latch High
#define SRBR0                   0x30            // Shadow Receiver Buffer Register 0
#define SRBR1                   0x34

#define STHR0                   0x30            // Shaow Transmit Holding Register 0
#define STHR1                   0x34

#define FAR                     0x70            // FIFO Access Register

#define TFR                     0x74            // Transmit FIFO Read Register
#define RFW                     0x78            // Receiver FIFO Write Register
#define USR                     0x7C            // UART Statsu Register
#define TFL                     0x80            // Transmit FIFO Level
#define RFL                     0x84            // Receiver FIFO Level
#define SRR                     0x88            // Software Reset Register

#define SRTS                    0x8C            // Shadow Request to Send
#define SBCR                    0x90
#define SDMAM                   0x94
#define SFE                     0x98
#define SRT                     0x9C
#define STET                    0xA0

#define HTX                     0xA4            // Halt Tx
#define DMASA                   0xA8            // DMA Software Acknowledge Register
#define TCR                     0xAC            // Transceiver Control Register
#define DE_EN                   0xB0            // Driver Output Enable Register
#define RE_EN                   0xB4            // Receiver Output Enable Register
#define DET                     0xB8            // Driver Output Enable Timing Register
#define TAT                     0xBC            // Transmit Address Register
#define DLF                     0xC0            // Divisor Latch Fraction Register
#define RAR                     0xC4            // Receive Address Register
#define TAR                     0xC8            // Transmit Address Register
#define LCR_EXT                 0xCC            // Line Extended Control Register
#define CPR                     0xF4            // Component Parameter Register
#define UCV                     0xF8            // UART Component Version
#define CTR                     0xFC            // Component Type Register

/* Define Register Field */

// Interrupt Enable Register
#define PTIME                   BIT(7)          // Programmable THRE Interrupt Enable
#define ELCOLR                  BIT(4)          // Interrupt Enable Register : Enable ALC
#define EDSSI                   BIT(3)          // Enable Modem Status Interrupt
#define ELSI                    BIT(2)          // Enable Receiver Line Status Interrupt
#define ETBEI                   BIT(1)          // Enable Transmit Holding Register Empty Interrupt.
#define ERBFI                   BIT(0)          // Enable Received Data Available Interrupt.

// FIFO Control Register
#define RT                      BIT_N(0x3, 6)   // RCVR Trigger
#define TET                     BIT_N(0x3, 4)   // TX Empty Trigger
#define DMAM                    BIT_N(0x3, 3)   // DMA Mode
#define XFIFOR                  BIT(2)          // XMIT FIFO Reset
#define RFIFOR                  BIT(1)          // RCVR FIFO Reset
#define FIFO_ENB                BIT(0)          // FIFO Enable

// Interrupt Identification Register
#define FIFOs_ENB               BIT_N(0x3, 6)   // FIFOs Enabled
#define IID                     BIT_N(0xF, 0)   // Interrupt ID

// Line Control Register
#define DLAB                    BIT(7)          // Divisor Latch Access Bit.
#define BC                      BIT(6)          // Break Control BIt.
#define SP                      BIT(5)          // Stick Parity
#define EVEN_PARITY             BIT(4)          // Even Parity Select.
#define PARITY_ENB              BIT(3)          // Parity Enable
#define STOP_BIT                BIT(2)          // Number of stop bits.
#define DATA_LENGTH(x)          BIT_N(x, 0)     // Data Length Select

// Modem Control Register
#define SIRE                    BIT(6)          // SIR Mode Enable
#define AFCE                    BIT(5)          // Auto Flow Control Enable
#define LOOPBACK                BIT(4)          // LoopBack Bit
#define OUT2                    BIT(3)          // OUT2
#define OUT1                    BIT(2)          // OUT1
#define RTS                     BIT(1)          // Request to Send
#define DTR                     BIT(0)          // Data Terminal Reday

// Line Status Register
#define ADDR_RCVD               BIT(8)          // Address Received Bit.
#define RFE                     BIT(7)          // Receiver FIFO Error Bit.
#define TEMT                    BIT(6)          // Transmitter Empty Bit.
#define THRE                    BIT(5)          // Transmit Holding Empty Bit.
#define BI                      BIT(4)          // Break Interrupt Bit.
#define FE                      BIT(3)          // Freaming Error Bit.
#define PE                      BIT(2)          // Parity Error Bit.
#define OE                      BIT(1)          // Overrun Error Bit.
#define DR                      BIT(0)          // Data Ready Bit.

// Modem Status Register
#define DCD                     BIT(7)          // Data Carrier Detect.
#define RI                      BIT(6)          // Ring Indicatior
#define DSR                     BIT(5)          // Data Set Ready.
#define CTR_F                   BIT(4)          // Clear to Send
#define DDCD                    BIT(3)          // Delta Dat a Carrier Detect.
#define TERI                    BIT(2)          // Trailing Edge of Ring Indicator
#define DDSR                    BIT(1)          // Delta Data Set Ready.
#define DCTS                    BIT(0)          // Delta Clear to Send.

// Scratchpad Register
#define SCR_F                   BIT_N(0xFF, 0)  // Scratch Storage Space.

// Low Power Divisor Latch Low
#define LPDLL_F                 BIT_N(0xFF, 0)  // Low Power Divisor Latch Low

// Low Power Divisor Latch High
#define LPDLH_F                 BIT_N(0xFF, 0)  // Low Power Divisor Latch High

// Shadow Receive Buffer Register
#define RSVD_SRBR               BIT_N(0xFFFFFFFF, 0)    //

// Shadow Transmit Holding Register
#define RSVD_STHR               BIT_N(0xFFFFFFFF, 0)    //

// FIFO Access Register
#define FAR_F                   BIT(0)          // FIFO Access

// Transmit FIFO Read Register
#define TFR_F                   BIT_N(0xFF, 0)  // Transmit FIFO Read

// Receive FIFO Write
#define RFFE                    BIT(9)          // Receive FIFO Framing Error.
#define RFPE                    BIT(8)          // Reveive FIFO Parity Error.
#define RFWD                    BIT_N(0xFF, 0)  // Reveive FIFO Write Data.

// UART Status Register
#define RX_FIFO_FULL            BIT(4)          // Reveive FIFO Full
#define RX_FIFO_NOTEMPTY        BIT(3)          // Reveive FIFO Not Empty
#define TX_FIFO_NOTEMPTY        BIT(2)          // Transmit FIFO Empty
#define TX_FIFO_NOTFULL         BIT(1)          // Transmit FIFO Not Full
#define UART_BUSY               BIT(0)          // UART Busy

// Transmit FIFO Level
#define TFL_F                   BIT_N(0xFF, 0)  // Transmit FIFO Level

// Receiver FIFO Level
#define RFL_F                   BIT_N(0xFF, 0)  // Receiver FIFO Level

// Software Reset Register
#define XMIT_FIFO_RESET         BIT(2)          // XMIT FIFO Reset
#define RCVR_FIFO_RESET         BIT(1)          // RCVR FIFO Reset
#define UART_RESET              BIT(0)          // UART Reset

// Shadow Request to Send
// Shadow Break Control Register
// Shadow DMA Mode Register
// Shadow FIFO Enable Register
// Shadow RCVR Trigger Register
// Shadow Tx Empty Trggier Register

// Halt Tx
#define HTX_F                   BIT(0)          // Halt TX

// DMA Software Acknowledge Register
#define DMASA_F                 BIT(0)          // DMA Software Acknowledge Register

// Transceiver Control Register
#define XFER_MODE               BIT_N(0x3, 3)   // Transfer Mode
#define DE_POL                  BIT(2)          // Driver Enable Polarity
#define RE_POL                  BIT(1)          // Receiver Enable Polarity
#define RS485_EN                BIT(0)          // RS485 Transfer Enable

// Driver Output Enable Register
#define DE_EN_F                 BIT(0)          // DE Enable Control

// Receiver Output Enable Register
#define RE_EN_F                 BIT(0)          // RE Enable Control

// Driver Output Enable Timing Register
#define DE_DEASSERT_TIME        BIT_N(0xFF, 16)         // Driver Enable De-Sserting time.
#define DE_ASSERT_TIME          BIT_N(0xFF, 0)          // Driver Enable Assert time.

// TurnAround Timing Register
#define RE_TO_DE                BIT_N(0xFF, 16)         // Receiver Enable to Driver TurnAround Time
#define DE_TO_RE                BIT_N(0xFF, 0)          // Driver Enable to Receiver Enable TurnAround time.

// Divisior Latch Fraction Register
#define DLF_F                   BIT_N(0xFFFF, 0)        // Fractional Part of Divisor

// Receiver Address Register
#define RAR_F                   BIT_N(0xFF, 0)          // Receiver Address

// Transmit Address Register
#define TAR_F                   BIT_N(0xFF, 0)          // Transmit Address

// Line Extended Control Register
#define TRANSMIT_MODE           BIT(3)                  // Transmit Mode Control bit.
#define SEND_ADDR               BIT(2)                  // Send Address Control bit.
#define ADDR_MATCH              BIT(1)                  // Address Match Moide.
#define DLS_E                   BIT(0)                  // Externsion for DLS.

// Component Parameter Register (Read Only)
#define FIFO_MODE               BIT_N(0xF, 16)          // Encoding FIFO_MODE.
#define DMA_EXTRA               BIT(13)                 // Enable the DMA_EXTRA
#define UART_ADD_ENCODED_PARAMS BIT(12)                 // Enable the UART_ADD_ENCODED_PARAM
#define SHADOW                  BIT(11)                 // Enable the Shadow
#define FIFO_STAT               BIT(10)                 // Enable the FIFO_STAT
#define FIFO_ACCESS             BIT(9)
#define ADDITIONAL_FEAT         BIT(8)
#define SIR_LP_MODE             BIT(7)
#define SIR_MODE                BIT(6)
#define THRE_MODE               BIT(5)
#define AFCE_MODE               BIT(4)
#define AB_DATA_WIDTH           BIT_N(0x3, 0)

// UART Component Version
#define UART_VERSION            BIT_N(0xFFFFFFFF, 0)    // Version Information (ASCII Value)

// Component Type Register
#define PERIPHERAL_ID           BIT_N(0xFFFFFFFF, 0)    // Peripheral ID


/* struct NX_UART_RegisterSet */
/* { */
/* 	volatile unsigned int    RBR_DLL_THR;    /\* 0x00 receive buf, divisor latch, tx holding *\/ */
/* 	volatile unsigned int    DLH_IER;        /\* 0x04 divisor latch, int enb *\/ */
/* 	volatile unsigned int    FCR_IIR;        /\* 0x08 fifo control, int id *\/ */
/* 	volatile unsigned int    LCR;            /\* 0x0C line control *\/ */
/* 	volatile unsigned int    MCR;            /\* 0x10 modem control *\/ */
/* 	volatile unsigned int    LSR;            /\* 0x14 line status *\/ */
/* 	volatile unsigned int    MSR;            /\* 0x18 modem status *\/ */
/* 	volatile unsigned int    SCR;            /\* 0x1C scratch pad *\/ */
/* 	volatile unsigned int    LPDLL;          /\* 0x20 low power divisor latch low *\/ */
/* 	volatile unsigned int    LPDLH;          /\* 0x24 low power divisor latch high *\/ */
/* 	volatile unsigned int    _Rev0[2];       /\* 0x28 *\/ */
/* 	volatile unsigned int    SRBR[8];        /\* 0x30 shadow reg low *\/ */
/* 	volatile unsigned int    STHR[8];        /\* 0x50 shadow reg high *\/ */
/* 	volatile unsigned int    FAR;            /\* 0x70 fifo access for test *\/ */
/* 	volatile unsigned int    TFR;            /\* 0x74 tx fifo read for test *\/ */
/* 	volatile unsigned int    RFW;            /\* 0x78 rx fifo write for test *\/ */
/* 	volatile unsigned int    USR;            /\* 0x7C status *\/ */
/* 	volatile unsigned int    TFL;            /\* 0x80 tx fifo level *\/ */
/* 	volatile unsigned int    RFL;            /\* 0x84 rx fifo level *\/ */
/* 	volatile unsigned int    SRR;            /\* 0x88 software reset *\/ */
/* 	volatile unsigned int    SRTS;           /\* 0x8C shadow req to send *\/ */
/* 	volatile unsigned int    SBCR;           /\* 0x90 shadow brk ctrl *\/ */
/* 	volatile unsigned int    SDMAM;          /\* 0x94 shadow dma mode *\/ */
/* 	volatile unsigned int    SFE;            /\* 0x98 shadow fifo enable *\/ */
/* 	volatile unsigned int    SRT;            /\* 0x9C shadow RCVR trigger *\/ */
/* 	volatile unsigned int    STET;           /\* 0xA0 shadow TX Empty Trigger *\/ */
/* 	volatile unsigned int    HTX;            /\* 0xA4 Halt TX *\/ */
/* 	volatile unsigned int    DMASA;          /\* 0xA8 dma software ack *\/ */
/* 	volatile unsigned int    TCR;            /\* 0xAC tx ctrl *\/ */
/* 	volatile unsigned int    DEN;            /\* 0xB0 driver output enable *\/ */
/* 	volatile unsigned int    REN;            /\* 0xB4 receiver output enable *\/ */
/* 	volatile unsigned int    DET;            /\* 0xB8 driver output enable timing *\/ */
/* 	volatile unsigned int    TAT;            /\* 0xBC turnaround timing *\/ */
/* 	volatile unsigned int    DLF;            /\* 0xC0 divisor latch fraction *\/ */
/* 	volatile unsigned int    RAR;            /\* 0xC4 rx address *\/ */
/* 	volatile unsigned int    TAR;            /\* 0xC8 tx address *\/ */
/* 	volatile unsigned int    LCR_EXT;        /\* 0xCC line extended control *\/ */
/* 	volatile unsigned int    _Rev1[9];       /\* 0xD0 *\/ */
/* 	volatile unsigned int    CPR;            /\* 0xF4 component parameter *\/ */
/* 	volatile unsigned int    UCV;            /\* 0xF8 uart component version *\/ */
/* 	volatile unsigned int    CTR;            /\* 0xFC component type *\/ */
/* }; */

#endif // __NX_UART_H__
