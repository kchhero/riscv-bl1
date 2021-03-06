////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Nexell Co. All Rights Reserved
// Nexell Co. Proprietary & Confidential
//
// Nexell informs that this code and information is provided "as is" base
// and without warranty of any kind, either expressed or implied, including
// but not limited to the implied warranties of merchantability and/or fitness
// for a particular puporse.
//
//
// Module	:
// File		:
// Description	:
// Author	: Hans
// History	: 2013.02.06 First implementation
//		2013.08.31 rev1 (port 0, 1, 2 selectable)
//		2017.10.09 rev0 for nxp3220
//              2018.05.10 compatible RISC-V, rocket-core by suker
////////////////////////////////////////////////////////////////////////////////
#include <nx_swallow.h>
#include <nx_bootheader.h>

#if defined(QEMU_RISCV) || defined(SOC_SIM)
#include <nx_qemu_sim_printf.h>
#else
#include <nx_swallow_printf.h>
#endif

#ifdef QEMU_RISCV
#include <test.h>
#endif

#include <nx_sdmmc.h>
#include <nx_chip_iomux.h>
#include <nx_gpio.h>
#include <nx_clock.h>
#include <nx_lib.h>
#include <iSDBOOT.h>

#define NX_ASSERT(x)

/* NX_SDMMC_RegisterSet * const pgSDXCReg[2] = */
/*     { */
/*         (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE, */
/*         (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC1_MODULE, */
/*     }; */

/* const union nxpad sdmmcpad[2][6] = { */
/*     { */
/*         {PADINDEX_OF_OSDMMC0_CDATA_0_}, */
/*         {PADINDEX_OF_OSDMMC0_CDATA_1_}, */
/*         {PADINDEX_OF_OSDMMC0_CDATA_2_}, */
/*         {PADINDEX_OF_OSDMMC0_CDATA_3_}, */
/*         {PADINDEX_OF_OSDMMC0_CCLK}, */
/*         {PADINDEX_OF_OSDMMC0_CMD}, */
/*     }, */
/*     { */
/*         {PADINDEX_OF_OSDMMC1_CDATA_0_}, */
/*         {PADINDEX_OF_OSDMMC1_CDATA_1_}, */
/*         {PADINDEX_OF_OSDMMC1_CDATA_2_}, */
/*         {PADINDEX_OF_OSDMMC1_CDATA_3_}, */
/*         {PADINDEX_OF_OSDMMC1_CCLK}, */
/*         {PADINDEX_OF_OSDMMC1_CMD}, */
/*     }}; */

//------------------------------------------------------------------------------
int NX_SDMMC_SetClock(SDBOOTSTATUS *pSDXCBootStatus, int enb, int divider)
{
    //QEMU does not necessary clock setting
#ifdef QEMU_RISCV
    _dprintf("[BL1-DEBUG]%s : nxSetDeviceClock cannot setting on QEMU mode\r\n", __func__);
    return 1;
#else
    
    volatile unsigned int timeout;
    unsigned int i = pSDXCBootStatus->SDPort;
        
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[i];

    //--------------------------------------------------------------------------
    // 1. Confirm that no card is engaged in any transaction.
    //	If there's a transaction, wait until it has been finished.

#if defined(DEBUG)
    _dprintf("[BL1-DEBUG] %s start\r\n",__func__);
    if (pSDXCReg->STATUS & (NX_SDXC_STATUS_DATABUSY | NX_SDXC_STATUS_FSMBUSY))
        {
            if (pSDXCReg->STATUS & NX_SDXC_STATUS_DATABUSY)
                _dprintf("[BL1-DEBUG]%s : ERROR - Data is busy\r\n", __func__);

            if (pSDXCReg->STATUS & NX_SDXC_STATUS_FSMBUSY)
                _dprintf("[BL1-DEBUG]%s : ERROR - Data Transfer is busy\r\n", __func__);

            INFINTE_LOOP();
        }
#endif
    
    //--------------------------------------------------------------------------
    // 2. Disable the output clock.
    // low power mode & clock disable
    pSDXCReg->CLKENA |= NX_SDXC_CLKENA_LOWPWR;
    pSDXCReg->CLKENA &= ~NX_SDXC_CLKENA_CLKENB;
    //    nxSetDeviceClock(&sdmmcclk[i][1], 1, 1);

    if (divider == SDCLK_DIVIDER_400KHZ) {
        pSDXCReg->CLKDIV = (256 >> 1); //400KHZ
    }
    else {
        pSDXCReg->CLKDIV = 2; //25MHZ
    }

    pSDXCReg->CLKENA &= ~NX_SDXC_CLKENA_LOWPWR;	// low power mode & clock disable

    //--------------------------------------------------------------------------
    // 4. Start a command with NX_SDXC_CMDFLAG_UPDATECLKONLY flag.
 repeat_4 :
    pSDXCReg->CMD = 0 | NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_UPDATECLKONLY |
        NX_SDXC_CMDFLAG_STOPABORT;

    //--------------------------------------------------------------------------
    // 5. Wait until a update clock command is taken by the SDXC module.
    //	If a HLE is occurred, repeat 4.
    timeout = 0;
    while (pSDXCReg->CMD & NX_SDXC_CMDFLAG_STARTCMD) {
        if (++timeout > NX_SDMMC_TIMEOUT) {
            INFINTE_LOOP();
            return 0;
        }
    }
    if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_HLE) {
        INFINTE_LOOP();
        pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_HLE;
        goto repeat_4;
    }
    if (0 == enb)
        return 1;

    //--------------------------------------------------------------------------
    // 6. Enable the output clock.
    pSDXCReg->CLKENA |= NX_SDXC_CLKENA_CLKENB;

    //--------------------------------------------------------------------------
    // 7. Start a command with NX_SDXC_CMDFLAG_UPDATECLKONLY flag.
 repeat_7 :
    pSDXCReg->CMD = 0 | NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_UPDATECLKONLY |
        NX_SDXC_CMDFLAG_STOPABORT;

    //--------------------------------------------------------------------------
    // 8. Wait until a update clock command is taken by the SDXC module.
    //	If a HLE is occurred, repeat 7.
    timeout = 0;
    while (pSDXCReg->CMD & NX_SDXC_CMDFLAG_STARTCMD) {
        if (++timeout > NX_SDMMC_TIMEOUT) {
            INFINTE_LOOP();
            return 0;
        }
    }
    if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_HLE) {
        INFINTE_LOOP();
        pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_HLE;
        goto repeat_7;
    }

    return 1;
#endif //QEMU_RISCV        
}

//------------------------------------------------------------------------------
unsigned int NX_SDMMC_SendCommandInternal(
                                          SDBOOTSTATUS *pSDXCBootStatus,
                                          NX_SDMMC_COMMAND *pCommand)
{
    unsigned int		cmd, flag;
    unsigned int		status = 0;
#ifndef QEMU_RISCV    
    volatile unsigned int	timeout;
#endif    
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] SendCommandInternal enter\r\n");
#endif

    NX_ASSERT(CNULL != pCommand);

    cmd	= pCommand->cmdidx & 0xFF;
    flag	= pCommand->flag;

    pSDXCReg->RINTSTS = 0xFFFFFFFF;

    //--------------------------------------------------------------------------
    // Send Command
#ifndef QEMU_RISCV
    timeout = 0;
    do {
        pSDXCReg->RINTSTS	= NX_SDXC_RINTSTS_HLE;
        pSDXCReg->CMDARG	= pCommand->arg;
        pSDXCReg->CMD		= cmd | flag | NX_SDXC_CMDFLAG_USE_HOLD_REG;

        while (pSDXCReg->CMD & NX_SDXC_CMDFLAG_STARTCMD) {
            if (++timeout > NX_SDMMC_TIMEOUT) {
#ifdef DEBUG
                _dprintf("TO send cmd\r\n");
#endif
                status |= NX_SDMMC_STATUS_CMDBUSY;
                INFINTE_LOOP();
                goto End;
            }
        }
    } while (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_HLE);
#else
    do {
        pSDXCReg->RINTSTS	= NX_SDXC_RINTSTS_HLE;
        pSDXCReg->CMDARG	= pCommand->arg;
        pSDXCReg->CMD		= cmd | flag |  NX_SDXC_CMDFLAG_USE_HOLD_REG;
    } while (0);
#endif

    //--------------------------------------------------------------------------
    // Wait until Command sent to card and got response from card.
#ifndef QEMU_RISCV
    timeout = 0;
    while (1) {
        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_CD)
            break;

        if (++timeout > NX_SDMMC_TIMEOUT) {
#ifdef DEBUG
            _dprintf("TO cmd done\r\n");
#endif
            status |= NX_SDMMC_STATUS_CMDTOUT;
            INFINTE_LOOP();
            goto End;
        }

        if ((flag & NX_SDXC_CMDFLAG_STOPABORT) &&
            (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_HTO)) {
            // You have to clear FIFO when HTO is occurred.
            // After that, SDXC module leaves in stopped state and takes next command.
            while (0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY)) {
                pSDXCReg->DATA;
            }
        }
    }
#endif

    // Check Response Error
    if (pSDXCReg->RINTSTS & (NX_SDXC_RINTSTS_RCRC |
                             NX_SDXC_RINTSTS_RE |
                             NX_SDXC_RINTSTS_RTO)) {
        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_RCRC)
            status |= NX_SDMMC_STATUS_RESCRCFAIL;
        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_RE)
            status |= NX_SDMMC_STATUS_RESERROR;
        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_RTO)
            status |= NX_SDMMC_STATUS_RESTOUT;
    }

    if ((status == NX_SDMMC_STATUS_NOERROR) &&
        (flag & NX_SDXC_CMDFLAG_SHORTRSP)) {
        pCommand->response[0] = pSDXCReg->RESP[0];
        if ((flag & NX_SDXC_CMDFLAG_LONGRSP) ==
            NX_SDXC_CMDFLAG_LONGRSP) {
            pCommand->response[1] = pSDXCReg->RESP[1];
            pCommand->response[2] = pSDXCReg->RESP[2];
            pCommand->response[3] = pSDXCReg->RESP[3];
        }

        if (NX_SDMMC_RSPIDX_R1B == ((pCommand->cmdidx >> 8) & 0xFF)) {
#ifndef QEMU_RISCV
            timeout = 0;
            do {
                if (++timeout > NX_SDMMC_TIMEOUT) {
#ifdef DEBUG
                    _dprintf("TO card data ready\r\n");
#endif
                    status |= NX_SDMMC_STATUS_DATABUSY;
                    INFINTE_LOOP();
                    goto End;
                }
            } while (pSDXCReg->STATUS & NX_SDXC_STATUS_DATABUSY);
#endif
        }
    }

 End:
    /* #if !defined(QEMU_RISCV) && defined(DEBUG) */
    /* End: */
    /*     if (NX_SDMMC_STATUS_NOERROR != status) { */
    /*         		_dprintf("err: cmd:%x, arg:%x => sts:%x, resp:%x\r\n", */
    /*         			pCommand->cmdidx, pCommand->arg, */
    /*         			status, pCommand->response[0]); */
    /*     } */
    /* #endif */
    pCommand->status = status;

    return status;
}

//------------------------------------------------------------------------------
unsigned int NX_SDMMC_SendStatus(SDBOOTSTATUS *pSDXCBootStatus)
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;

    cmd.cmdidx	= SEND_STATUS;
    cmd.arg	= pSDXCBootStatus->rca;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] SendStatus: cmd = 0x%x\r\n",&cmd);
#endif

    status = NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd);

    if (NX_SDMMC_STATUS_NOERROR == status)
        return status;

#if defined(DEBUG) && !defined(SOC_SIM)
    _dprintf("\t\t ERROR : status = 0x%x\r\n",status);
    _dprintf("\t\t ERROR : Value = 0x%x\r\n",cmd.response[0]);

    if (cmd.response[0] & (1UL << 31))
        _dprintf("\t\t ERROR : OUT_OF_RANGE\r\n");
    if (cmd.response[0] & (1UL << 30))
        _dprintf("\t\t ERROR : ADDRESS_ERROR\r\n");
    if (cmd.response[0] & (1UL << 29))
        _dprintf("\t\t ERROR : BLOCK_LEN_ERROR\r\n");
    if (cmd.response[0] & (1UL << 28))
        _dprintf("\t\t ERROR : ERASE_SEQ_ERROR\r\n");
    if (cmd.response[0] & (1UL << 27))
        _dprintf("\t\t ERROR : ERASE_PARAM\r\n");
    if (cmd.response[0] & (1UL << 26))
        _dprintf("\t\t ERROR : WP_VIOLATION\r\n");
    if (cmd.response[0] & (1UL << 24))
        _dprintf("\t\t ERROR : LOCK_UNLOCK_FAILED\r\n");
    if (cmd.response[0] & (1UL << 23))
        _dprintf("\t\t ERROR : COM_CRC_ERROR\r\n");
    if (cmd.response[0] & (1UL << 22))
        _dprintf("\t\t ERROR : ILLEGAL_COMMAND\r\n");
    if (cmd.response[0] & (1UL << 21))
        _dprintf("\t\t ERROR : CARD_ECC_FAILED\r\n");
    if (cmd.response[0] & (1UL << 20))
        _dprintf("\t\t ERROR : Internal Card Controller ERROR\r\n");
    if (cmd.response[0] & (1UL << 19))
        _dprintf("\t\t ERROR : General Error\r\n");
    if (cmd.response[0] & (1UL << 17))
        _dprintf("\t\t ERROR : Deferred Response\r\n");
    if (cmd.response[0] & (1UL << 16))
        _dprintf("\t\t ERROR : CID/CSD_OVERWRITE_ERROR\r\n");
    if (cmd.response[0] & (1UL << 15))
        _dprintf("\t\t ERROR : WP_ERASE_SKIP\r\n");
    if (cmd.response[0] & (1UL << 3))
        _dprintf("\t\t ERROR : AKE_SEQ_ERROR\r\n");

    switch ((cmd.response[0] >> 9) & 0xF) {
    case 0: _dprintf("\t\t CURRENT_STATE : Idle\r\n");
        break;
    case 1: _dprintf("\t\t CURRENT_STATE : Ready\r\n");
        break;
    case 2: _dprintf("\t\t CURRENT_STATE : Identification\r\n");
        break;
    case 3: _dprintf("\t\t CURRENT_STATE : Standby\r\n");
        break;
    case 4: _dprintf("\t\t CURRENT_STATE : Transfer\r\n");
        break;
    case 5: _dprintf("\t\t CURRENT_STATE : Data\r\n");
        break;
    case 6: _dprintf("\t\t CURRENT_STATE : Receive\r\n");
        break;
    case 7: _dprintf("\t\t CURRENT_STATE : Programming\r\n");
        break;
    case 8: _dprintf("\t\t CURRENT_STATE : Disconnect\r\n");
        break;
    case 9: _dprintf("\t\t CURRENT_STATE : Sleep\r\n");
        break;
    default: _dprintf("\t\t CURRENT_STATE : Reserved\r\n");
        break;
    }
#endif

    return status;
}

//------------------------------------------------------------------------------
unsigned int NX_SDMMC_SendCommand(SDBOOTSTATUS *pSDXCBootStatus,
                                  NX_SDMMC_COMMAND *pCommand)
{
    unsigned int status;

    status = NX_SDMMC_SendCommandInternal(pSDXCBootStatus, pCommand);
    if (NX_SDMMC_STATUS_NOERROR != status) {
        NX_SDMMC_SendStatus(pSDXCBootStatus);
    }

    return status;
}

//------------------------------------------------------------------------------
unsigned int NX_SDMMC_SendAppCommand(SDBOOTSTATUS *pSDXCBootStatus,
                                     NX_SDMMC_COMMAND *pCommand)
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;
    
    cmd.cmdidx	= APP_CMD;
    cmd.arg		= pSDXCBootStatus->rca;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] SendAppCommand\r\n");
#endif

    status = NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd);
    if (NX_SDMMC_STATUS_NOERROR == status) {
        NX_SDMMC_SendCommand(pSDXCBootStatus, pCommand);
    }
    
    return status;
}

//------------------------------------------------------------------------------
int NX_SDMMC_IdentifyCard(SDBOOTSTATUS *pSDXCBootStatus)
{
#ifndef QEMU_RISCV
    int timeout;
#endif
    unsigned int HCS, RCA;
    NX_SDMMC_CARDTYPE CardType = NX_SDMMC_CARDTYPE_UNKNOWN;
    NX_SDMMC_COMMAND cmd;
    NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];
    //    int nErrType = NX_SDMMC_STATUS_NOERROR;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] IdentifyCard enter, port=0x%x\r\n",pSDXCBootStatus->SDPort);
    _dprintf("[BL1-DEBUG] pSDXCReg=0x%x\r\n",*pSDXCReg);
#endif

    pSDXCReg->RSTn = 0;         // Hardware reset, 0:reset, 1:Active Mode
    
    if (0 == NX_SDMMC_SetClock(pSDXCBootStatus,1,SDCLK_DIVIDER_400KHZ))
        return 0;

    // Data Bus Width : 0(1-bit), 1(4-bit)
    pSDXCReg->CTYPE = 0;

    pSDXCBootStatus->rca = 0;

    //--------------------------------------------------------------------------
    //	Identify SD/MMC card
    //--------------------------------------------------------------------------
    // Go idle state
    cmd.cmdidx	= GO_IDLE_STATE;
    cmd.arg		= 0;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_SENDINIT |
        NX_SDXC_CMDFLAG_STOPABORT;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] IdentifyCard: cmd = 0x%x\r\n",&cmd);
    _dprintf("[BL1-DEBUG] IdentifyCard: cmd.flag = 0x%x\r\n",cmd.flag);
#endif
    NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);

    // Check SD Card Version
    cmd.cmdidx	= SEND_IF_COND;
    // argument = VHS : 2.7~3.6V and Check Pattern(0xAA)
    cmd.arg		= (1 << 8) | 0xAA;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

    _dprintf("[BL1-DEBUG] IdentifyCard 1\r\n");
//new------------------------------------------------------------------------------------
    /* int retrycount = 5; */
    /* do { */
    /*     nErrType = NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd); */
    /*     if (nErrType == NX_SDMMC_STATUS_NOERROR || retrycount <= 0) { */
    /*         break; */
    /*     } */
    /*     _dprintf("retry..."); */
    /*     retrycount--; */
    /* } while(1); */
//new------------------------------------------------------------------------------------
    if (NX_SDMMC_STATUS_NOERROR ==
        NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd)) {
        // Ver 2.0 or later SD Memory Card
        if (cmd.response[0] != ((1 << 8) | 0xAA))
            return 0;

        HCS = 1 << 30;
    } else {
        // voltage mismatch or Ver 1.X SD Memory Card or not SD Memory Card
        _dprintf("[BL1-DEBUG] voltage mismatch or Ver 1.X SD Memory Card or not SD Memory Card\r\n");
        HCS = 0;
    }

    _dprintf("[BL1-DEBUG] IdentifyCard 2\r\n");
    //--------------------------------------------------------------------------
    // voltage validation
#ifndef QEMU_RISCV
    timeout = NX_SDMMC_TIMEOUT_IDENTIFY;
#endif
    cmd.cmdidx	= APP_CMD;
    cmd.arg	= pSDXCBootStatus->rca;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

    if (NX_SDMMC_STATUS_NOERROR ==
        NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd)) {
        //----------------------------------------------------------------------
        // SD memory card
#define FAST_BOOT	(1<<29)

        cmd.cmdidx	= SD_SEND_OP_COND;
        cmd.arg		= (HCS | FAST_BOOT | 0x00FC0000);	// 3.0 ~ 3.6V
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_WAITPRVDAT |
            NX_SDXC_CMDFLAG_SHORTRSP;

        if (NX_SDMMC_STATUS_NOERROR !=
            NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd)) {
            _dprintf("[BL1-DEBUG] SD memory card indentify fail\r\n");
            return 0;
        }
#ifndef QEMU_RISCV
        /* Wait until card has finished the power up routine */
        while (0==(cmd.response[0] & (1UL << 31))) {
            if (NX_SDMMC_STATUS_NOERROR !=
                NX_SDMMC_SendAppCommand(
                                        pSDXCBootStatus, &cmd)) {
                return 0;
            }

            if (timeout-- <= 0) {
#ifdef DEBUG                
                _dprintf("TO pwrup SD\r\n");
#endif
                return 0;
            }
        }
#endif
#ifdef DEBUG
        _dprintf("---- SD ----\r\n");
#endif
        CardType	= NX_SDMMC_CARDTYPE_SDMEM;
        RCA		= 0;
    } else {
        _dprintf("[BL1-DEBUG] MMC memory card\r\n");
        //----------------------------------------------------------------------
        // MMC memory card
        cmd.cmdidx	= GO_IDLE_STATE;
        cmd.arg		= 0;
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_SENDINIT |
            NX_SDXC_CMDFLAG_STOPABORT;

        NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);
#ifndef QEMU_RISCV
        do {
            cmd.cmdidx	= SEND_OP_COND;
            cmd.arg		= 0x40FC0000;	// MMC High Capacity -_-???
            cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
                NX_SDXC_CMDFLAG_WAITPRVDAT |
                NX_SDXC_CMDFLAG_SHORTRSP;
            if (NX_SDMMC_STATUS_NOERROR !=
                NX_SDMMC_SendCommand(pSDXCBootStatus,
                                     &cmd)) {
                return 0;
            }

            if (timeout-- <= 0) {
#ifdef DEBUG                
                _dprintf("TO to wait pow-up for MMC\r\n");
#endif
                return 0;
            }
            /* Wait until card has finished the power up routine */
        } while (0==(cmd.response[0] & (1UL << 31)));
#endif

#if defined(DEBUG)
        _dprintf("---- MMC ----\r\n");
        _dprintf("--> SEND_OP_COND Response = 0x%X\r\n", cmd.response[0]);
#endif

        CardType	= NX_SDMMC_CARDTYPE_MMC;
        RCA		= 2 << 16;
    }

    //	NX_ASSERT( (cmd.response[0] & 0x00FC0000) == 0x00FC0000 );
    pSDXCBootStatus->bHighCapacity =
        (cmd.response[0] & (1 << 30)) ? 1 : 0;

    /* #ifdef DEBUG */
    /*     _dprintf("%sCard.\r\n", (pSDXCBootStatus->bHighCapacity) ? "High Capacity " : ""); */
    /* #endif */

    //--------------------------------------------------------------------------
    // Get CID
    cmd.cmdidx	= ALL_SEND_CID;
    cmd.arg	= 0;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_LONGRSP;
    if (NX_SDMMC_STATUS_NOERROR !=
        NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd)) {
#ifdef DEBUG
        _dprintf("cannot read CID\r\n");
#endif
        return 0;
    }

    //--------------------------------------------------------------------------
    // Get RCA and change to Stand-by State in data transfer mode
    cmd.cmdidx	= (CardType == NX_SDMMC_CARDTYPE_SDMEM) ?
        SEND_RELATIVE_ADDR : SET_RELATIVE_ADDR;
    cmd.arg		= RCA;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;
    if (NX_SDMMC_STATUS_NOERROR !=
        NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd)) {
        //        _dprintf("cannot read RCA\r\n");
        return 0;
    }

    if (CardType == NX_SDMMC_CARDTYPE_SDMEM)
        pSDXCBootStatus->rca = cmd.response[0] & 0xFFFF0000;
    else
        pSDXCBootStatus->rca = RCA;

    pSDXCBootStatus->CardType = CardType;

#if defined(DEBUG)
    _dprintf("[BL1-DEBUG] CardType = 0x%x\r\n",CardType);
#endif
    
    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_SelectCard(SDBOOTSTATUS *pSDXCBootStatus)
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;

    cmd.cmdidx	= SELECT_CARD;
    cmd.arg		= pSDXCBootStatus->rca;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

    status = NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);

    return (NX_SDMMC_STATUS_NOERROR == status) ? 1 : 0;
}

//------------------------------------------------------------------------------
int NX_SDMMC_SetCardDetectPullUp(SDBOOTSTATUS *pSDXCBootStatus,
                                 int bEnb)
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;

    cmd.cmdidx	= SET_CLR_CARD_DETECT;
    cmd.arg		= (bEnb) ? 1 : 0;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;

    status = NX_SDMMC_SendAppCommand(pSDXCBootStatus, &cmd);
    
    return (NX_SDMMC_STATUS_NOERROR == status) ? 1 : 0;
}

//------------------------------------------------------------------------------
int NX_SDMMC_SetBusWidth(SDBOOTSTATUS *pSDXCBootStatus,
                         unsigned int buswidth )
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];

    NX_ASSERT( buswidth==1 || buswidth==4 );

    if (pSDXCBootStatus->CardType == NX_SDMMC_CARDTYPE_SDMEM) {
#ifdef DEBUG
        _dprintf("[BL1-DEBUG] BootStatus CardType NX_SDMMC_CARDTYPE_SDMEM\r\n");
#endif
        cmd.cmdidx	= SET_BUS_WIDTH;
        cmd.arg		= (buswidth >> 1);
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_WAITPRVDAT |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP;
        status = NX_SDMMC_SendAppCommand(pSDXCBootStatus, &cmd);
    } else {
#ifdef DEBUG
        _dprintf("[BL1-DEBUG] BootStatus CardType another\r\n");
#endif
        /* ExtCSD[183] : BUS_WIDTH <= 0 : 1-bit, 1 : 4-bit, 2 : 8-bit */
        cmd.cmdidx	= SWITCH_FUNC;
        cmd.arg		=		  (3 << 24) |
            (183 << 16) |
            ((buswidth >> 2) << 8) |
            (0 << 0);
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_WAITPRVDAT |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP;
        status = NX_SDMMC_SendCommand( pSDXCBootStatus, &cmd );
    }

    if (NX_SDMMC_STATUS_NOERROR != status)
        return 0;

    /* 0 : 1-bit mode, 1 : 4-bit mode */
    pSDXCReg->CTYPE = buswidth >> 2;
    pSDXCReg->TIEMODE = buswidth >> 2;

    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_SetBlockLength(SDBOOTSTATUS *pSDXCBootStatus,
                            unsigned int blocklength)
{
    unsigned int status;
    NX_SDMMC_COMMAND cmd;
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;
    //        pgSDXCReg[pSDXCBootStatus->SDPort];

    cmd.cmdidx	= SET_BLOCKLEN;
    cmd.arg		= blocklength;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_WAITPRVDAT |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;
    status = NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);

    if (NX_SDMMC_STATUS_NOERROR != status)
        return 0;

    pSDXCReg->BLKSIZ = blocklength;

    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_Init(SDBOOTSTATUS *pSDXCBootStatus)
{
    unsigned int i = pSDXCBootStatus->SDPort;
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[i];

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] Init start, port=0x%x \r\n",pSDXCBootStatus->SDPort);
#endif

    pSDXCReg->PWREN = 0 << 0;	// Set Power Disable
    pSDXCReg->CLKENA = NX_SDXC_CLKENA_LOWPWR;// low power mode & clock disable

    pSDXCReg->TIEDRVPHASE = NX_SDMMC_CLOCK_SHIFT_180 << 8;
    pSDXCReg->TIESMPPHASE = NX_SDMMC_CLOCK_SHIFT_0  << 8;

    pSDXCReg->CLKSRC = 0;	// prescaler 0
    pSDXCReg->CLKDIV = 0;//(SDCLK_DIVIDER_WORK >> 1) << 8 | (SDCLK_DIVIDER_ENUM >> 1) << 0;

    // fifo mode, not read wait(only use sdio mode)
    pSDXCReg->CTRL &= ~(NX_SDXC_CTRL_DMAMODE_EN | NX_SDXC_CTRL_READWAIT);
    // Reset the controller & DMA interface & FIFO
    pSDXCReg->CTRL = NX_SDXC_CTRL_DMARST |
        NX_SDXC_CTRL_FIFORST |
        NX_SDXC_CTRL_CTRLRST;

#ifndef QEMU_RISCV
    while (pSDXCReg->CTRL & (NX_SDXC_CTRL_DMARST |
                             NX_SDXC_CTRL_FIFORST |
                             NX_SDXC_CTRL_CTRLRST))
        pSDXCReg->CTRL;
#endif

    pSDXCReg->PWREN = 0x1 << 0;	// Set Power Enable

    // Data Timeout = 0xFFFFFF, Response Timeout = 0x64
    pSDXCReg->TMOUT = (0xFFFFFFU << 8) | (0x64 << 0);

    // Data Bus Width : 0(1-bit), 1(4-bit)
    pSDXCReg->CTYPE = 0;

    // Block size
    pSDXCReg->BLKSIZ = BLOCK_LENGTH;

    // Issue when RX FIFO Count >= 8 x 4 bytes & TX FIFO Count <= 8 x 4 bytes
    pSDXCReg->FIFOTH = ((8 - 1) << 16) |		// Rx threshold
        (8 << 0);		// Tx threshold

    // Mask & Clear All interrupts
    pSDXCReg->INTMASK = 0;
    pSDXCReg->RINTSTS = 0xFFFFFFFF;

    // Wake up & Power on fifo sram
    pSDXCReg->TIESRAM = 0x3;
    pSDXCReg->TIEMODE = 1;

    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_Terminate(SDBOOTSTATUS *pSDXCBootStatus)
{
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;
    //        pgSDXCReg[pSDXCBootStatus->SDPort];
    
    // Clear All interrupts
    pSDXCReg->RINTSTS = 0xFFFFFFFF;

    // Reset the controller & DMA interface & FIFO
    pSDXCReg->CTRL = NX_SDXC_CTRL_DMARST |
        NX_SDXC_CTRL_FIFORST |
        NX_SDXC_CTRL_CTRLRST;
#ifndef QEMU_RISCV        
    while (pSDXCReg->CTRL & (NX_SDXC_CTRL_DMARST |
                             NX_SDXC_CTRL_FIFORST |
                             NX_SDXC_CTRL_CTRLRST))
        {
            pSDXCReg->CTRL;
        }
    //    nxSetDeviceClock(&sdmmcclk[pSDXCBootStatus->SDPort][0], 2, 0);
#endif

    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_Open(SDBOOTSTATUS *pSDXCBootStatus)
{
#ifdef DEBUG
    _dprintf("[BL1-DEBUG] NX_SDMMC_Open step1\r\n");
#endif

#ifdef QEMU_RISCV
    return 1;
#endif

    //--------------------------------------------------------------------------
    // card identification mode : Identify & Initialize
    if (0 == NX_SDMMC_IdentifyCard(pSDXCBootStatus)) {
#ifdef DEBUG        
        _dprintf("Identify Fail\r\n");
#endif
        return 0;
    }

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] NX_SDMMC_Open step2\r\n");
#endif
    
    //--------------------------------------------------------------------------
    // data transfer mode : Stand-by state
    if (0 == NX_SDMMC_SetClock(pSDXCBootStatus, 1, SDCLK_DIVIDER_NORMAL)) {
        //_dprintf("Card Clk rst fail\r\n");
        return 0;
    }
    if (0 == NX_SDMMC_SelectCard(pSDXCBootStatus)) {
        //_dprintf("Card Sel Fail\r\n");
        return 0;
    }

    //--------------------------------------------------------------------------
    // data transfer mode : Transfer state
    if (pSDXCBootStatus->CardType == NX_SDMMC_CARDTYPE_SDMEM) {
        NX_SDMMC_SetCardDetectPullUp(pSDXCBootStatus, 0);
    }

    if (0 == NX_SDMMC_SetBlockLength(pSDXCBootStatus,
                                     BLOCK_LENGTH)) {
        //_dprintf("Set Blk Lng Fail\r\n");
        return 0;
    }

    //2018/05/08 EBE error
    NX_SDMMC_SetBusWidth(pSDXCBootStatus, 4);

    return 1;
}

//------------------------------------------------------------------------------
int NX_SDMMC_Close(SDBOOTSTATUS *pSDXCBootStatus)
{
#ifdef DEBUG
    _dprintf("[BL1-DEBUG] NX_SDMMC_Close\r\n");
#endif
    
    NX_SDMMC_SetClock(pSDXCBootStatus, 0, SDCLK_DIVIDER_400KHZ);
       
    return 1;
}

//------------------------------------------------------------------------------
#ifdef QEMU_RISCV
int NX_SDMMC_ReadSectorData(
                            SDBOOTSTATUS *pSDXCBootStatus,
                            unsigned int numberOfSector,
                            unsigned int step,
                            unsigned int *pdwBuffer)
{
    unsigned int	count;
    unsigned int i = 0;
    unsigned int *temp;
    unsigned int seek_qemu = step * 512;
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];

    if (seek_qemu > sd_body_size)
        return 0;
    
#ifdef DEBUG
    _dprintf("[BL1-DEBUG] ---- start ---- %s\r\n",__func__);
    _dprintf("[BL1-DEBUG] ---------------------------------------\r\n");
    _dprintf("[BL1-DEBUG] %s pdwBuffer start addr = 0x%x\r\n",__func__, pdwBuffer);
    _dprintf("[BL1-DEBUG] ---------------------------------------\r\n");
#endif
        
    NX_ASSERT(0 == ((unsigned int)pdwBuffer & 3));

    count = numberOfSector * BLOCK_LENGTH;
    NX_ASSERT(0 == (count % 32));

#ifdef DEBUG
    for(unsigned int j = seek_qemu; j < count; j++) {
        _dprintf("%c",(unsigned char)(sd_body[j]));
    }
    _dprintf("*****************************************\r\n\r\n");
    _dprintf("**************copy to pdwbuffer*******\r\n",__func__);
#endif

    while (0 < count) {
        *pdwBuffer++ = (unsigned int)sd_body[i+seek_qemu];
        //        _dprintf("%c",(unsigned char)*pdwBuffer);
        count -= 1;
        i++;

        NX_ASSERT(0 == (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_FRUN));
    }
    
    pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_DTO;

#ifdef DEBUG
    _dprintf("*************************************\r\n",__func__);
    _dprintf("\r\n[BL1-DEBUG] %s ---- end ----\r\n\r\n",__func__);
#endif

    return 1;
}

#else //QEMU_RISCV

int NX_SDMMC_ReadSectorData(
                            SDBOOTSTATUS *pSDXCBootStatus,
                            unsigned int numberOfSector,
                            unsigned int *pdwBuffer)
{
    unsigned int		count;
    volatile unsigned int       temp = 0;
    
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];
#ifdef DEBUG
    unsigned int                _cnt = 0;

    _dprintf("ReadSectorData : numberOfSector= 0x%x\r\n",numberOfSector);
    _dprintf("ReadSectorData : BLOCK_LENGTH = 0x%x\r\n",BLOCK_LENGTH);    
#endif
    NX_ASSERT(0 == ((unsigned int)pdwBuffer & 3));

    count = numberOfSector * BLOCK_LENGTH;
    //_dprintf("ReadSectorData : total count = 0x%x\r\n",count);
    
    NX_ASSERT(0 == (count % 32));

    while (0 < count) {
/* #ifdef DEBUG */
/*         _dprintf("ReadSectorData : Try 0x%x\r\n",_cnt++); */
/* #endif */
        if ((pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_RXDR) || (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_DTO)) {            
            unsigned int FSize, Timeout = NX_SDMMC_TIMEOUT;

            while ((pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY) && Timeout--) {
                pSDXCReg->STATUS;
            }
            if (0 == Timeout) {
                //_dprintf("ReadSectorData : Timeout 0!!\r\n");
                break;
            }
            FSize = (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOCOUNT) >> 17;
/* #ifdef DEBUG             */
/*             _dprintf("ReadSectorData : FSize = 0x%x\r\n",FSize); */
/* #endif */
            count -= (FSize * 4);
/* #ifdef DEBUG */
/*             _dprintf("ReadSectorData : left count = 0x%x\r\n",count); */
/* #endif */
            
            while (FSize) {
                *pdwBuffer++ = pSDXCReg->DATA;
                FSize--;
            }
            pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_RXDR;
        }

        temp = pSDXCReg->RINTSTS;

        // Check Errors
        if (temp & (NX_SDXC_RINTSTS_DRTO |
                    NX_SDXC_RINTSTS_EBE |
                    NX_SDXC_RINTSTS_SBE |
                    NX_SDXC_RINTSTS_DCRC)) {
#if DEBUG
            _dprintf("Read left = %x\r\n", count);

            if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_DRTO)
                _dprintf("DRTO\r\n");
            if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_EBE)
                _dprintf("EBE\r\n");
            if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_SBE)
                _dprintf("SBE\r\n");
            if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_DCRC)
                _dprintf("DCRC\r\n");
#endif
            return 0;
        }

        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_DTO) {
            if (count == 0) {
                pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_DTO;
                break;
            }
        }

        if (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_HTO) {
#ifdef DEBUG            
            _dprintf("HTO\r\n");
#endif
            pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_HTO;
        }

        NX_ASSERT(0 == (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_FRUN));
    }
    
    pSDXCReg->RINTSTS = NX_SDXC_RINTSTS_DTO;

    return 1;
}
#endif //QEMU_RISCV

//------------------------------------------------------------------------------
int NX_SDMMC_ReadSectors(SDBOOTSTATUS *pSDXCBootStatus,
                         unsigned int SectorNum, unsigned int numberOfSector, unsigned int *pdwBuffer)
{
    int	result = 0;
    unsigned int    status;
    NX_SDMMC_COMMAND cmd;
    register NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];
        
    NX_ASSERT(0 == ((unsigned int)pdwBuffer & 3));
#ifdef DEBUG
    _dprintf("[BL1-DEBUG] ReadSectors enter\r\n");
#endif
    // wait while data busy or data transfer busy
    while (pSDXCReg->STATUS & (1 << 9 | 1 << 10))
        pSDXCReg->STATUS;

    //--------------------------------------------------------------------------
    // wait until 'Ready for data' is set and card is in transfer state.
#ifndef QEMU_RISCV
    do {
        cmd.cmdidx	= SEND_STATUS;
        cmd.arg		= pSDXCBootStatus->rca;
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP;
        status = NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);
        if (NX_SDMMC_STATUS_NOERROR != status) {
            //_dprintf("[BL1-DEBUG] NX_SDMMC_STATUS_NOERROR != status\r\n");
            goto End;
        }
    } while (!((cmd.response[0] & (1 << 8)) &&
               (((cmd.response[0] >> 9) & 0xF) == 4)));
#else
    cmd.cmdidx	= SEND_STATUS;
    cmd.arg		= pSDXCBootStatus->rca;
    cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
        NX_SDXC_CMDFLAG_CHKRSPCRC |
        NX_SDXC_CMDFLAG_SHORTRSP;
    status = NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);
#endif //QEMU_RISCV
    
    NX_ASSERT(NX_SDXC_STATUS_FIFOEMPTY ==
              (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY));
    NX_ASSERT(0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FSMBUSY));

    // Set byte count
    pSDXCReg->BYTCNT = numberOfSector * BLOCK_LENGTH;

    //--------------------------------------------------------------------------
    // Send Command
    if (numberOfSector > 1) {
        cmd.cmdidx	= READ_MULTIPLE_BLOCK;
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_WAITPRVDAT |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP |
            NX_SDXC_CMDFLAG_BLOCK |
            NX_SDXC_CMDFLAG_RXDATA |
            NX_SDXC_CMDFLAG_SENDAUTOSTOP;
    } else {
        cmd.cmdidx	= READ_SINGLE_BLOCK;
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_WAITPRVDAT |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP |
            NX_SDXC_CMDFLAG_BLOCK |
            NX_SDXC_CMDFLAG_RXDATA;
    }
    cmd.arg		= (pSDXCBootStatus->bHighCapacity) ?
        SectorNum : SectorNum*BLOCK_LENGTH;

    status = NX_SDMMC_SendCommand(pSDXCBootStatus, &cmd);

#ifndef QEMU_RISCV
    if (NX_SDMMC_STATUS_NOERROR != status) {
        //_dprintf("[BL1-DEBUG] NX_SDMMC_STATUS_NOERROR != status 2\r\n");
        goto End;
    }
    //--------------------------------------------------------------------------
    // Read data
    if (1 != NX_SDMMC_ReadSectorData(pSDXCBootStatus, numberOfSector, pdwBuffer)) {
        //_dprintf("[BL1-DEBUG] ReadSectorData Fail\r\n");
        goto End;
    }
#else
    if (1 != NX_SDMMC_ReadSectorData(pSDXCBootStatus, numberOfSector, SectorNum, pdwBuffer)) {
        goto End;
    }
#endif //QEMU_RISCV

    NX_ASSERT(NX_SDXC_STATUS_FIFOEMPTY ==
              (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY));
    NX_ASSERT(0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOFULL));
    NX_ASSERT(0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOCOUNT));

#ifndef QEMU_RISCV
    if (numberOfSector > 1) {
        // Wait until the Auto-stop command has been finished.
        while (0 == (pSDXCReg->RINTSTS & NX_SDXC_RINTSTS_ACD))
            pSDXCReg->RINTSTS;

        NX_ASSERT(0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FSMBUSY));

        /* #ifdef DEBUG */
        /*         // Get Auto-stop response and then check it. */
        /*         { */
        /*             unsigned int    response; */

        /*             response = pSDXCReg->RESP[1]; */
        /*             if (response & 0xFDF98008) { */
        /*                 _dprintf("Auto Stop Resp Fail:%x\r\r\n", response); */
        /*                 //goto End; */
        /*             } */
        /*         } */
        /* #endif */
    }
#endif //QEMU_RISCV
    result = 1;

 End:
    if (0 == result) {
        //DateSheet 7.4.4.1 H/W Reset Programming Sequence
        cmd.cmdidx	= STOP_TRANSMISSION;
        cmd.arg		= 0;
        cmd.flag	= NX_SDXC_CMDFLAG_STARTCMD |
            NX_SDXC_CMDFLAG_CHKRSPCRC |
            NX_SDXC_CMDFLAG_SHORTRSP |
            NX_SDXC_CMDFLAG_STOPABORT;
        NX_SDMMC_SendCommandInternal(pSDXCBootStatus, &cmd);


        if (0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY)) {
            pSDXCReg->CTRL = NX_SDXC_CTRL_FIFORST;
#ifndef QEMU_RISCV            
            while (pSDXCReg->CTRL & NX_SDXC_CTRL_FIFORST)
                pSDXCReg->CTRL;
#endif //QEMU_RISCV
        }
    }

    return result;
}

//------------------------------------------------------------------------------
int SDMMCBOOT(SDBOOTSTATUS *pSDXCBootStatus)
{
    int	result = 0;
    //    int ret = 0;
    struct nx_bootmm *bl1BootMem = (struct nx_bootmm *)BASEADDR_DRAM;
    struct nx_bootmm *bblBootMem = (struct nx_bootmm *)BASEADDR_DRAM;
    unsigned int bl1BinSize = 0;
    unsigned int bblBinSize = 0;
    unsigned int dtbBinSize = 0;
    NX_SDMMC_RegisterSet * const pSDXCReg = (NX_SDMMC_RegisterSet *)PHY_BASEADDR_SDMMC0_MODULE;//pgSDXCReg[pSDXCBootStatus->SDPort];

    unsigned int *pVectorTableSector = (unsigned int *)(VECTOR_ADDR);    
    unsigned int *pBL1Sector = (unsigned int *)&(bl1BootMem)->bi;
    unsigned int *pBBLSector = (unsigned int *)&(bblBootMem)->bi;
    unsigned int *pDTBSector = (unsigned int *)&(bblBootMem)->bi;
    unsigned int rsn = 1;

    struct nx_bootinfo *pBL1BootInfo = &(bl1BootMem)->bi;
    struct nx_bootinfo *pBBLBootInfo = &(bblBootMem)->bi;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] SDMMCBOOT start\r\n");
#endif

    if (1 != NX_SDMMC_Open(pSDXCBootStatus)) {
        //_dprintf("device open fail\r\n");
        goto error;
    }

    if (0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY)) {
#ifndef QEMU_RISCV
        volatile unsigned int tempcount = 0x100000;
#endif
        pSDXCReg->CTRL = NX_SDXC_CTRL_FIFORST;
        /* Wait until the FIFO reset is completed. */
#ifndef QEMU_RISCV                
        while ((pSDXCReg->CTRL & NX_SDXC_CTRL_FIFORST) &&
               tempcount--)
            pSDXCReg->CTRL;
#endif
    }

    //=============================================================================================================
    // BL1
    //---------------------------------------------------------------------------
    //MBR sector 0 skipped
    // BL1 NSIH sector 1 of BL1
    //---------------------------------------------------------------------------
#ifdef DEBUG
    _dprintf("\r\n[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] BL1 NSIH header reload\r\n");
    _dprintf("[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] rsn = 0x%x\r\n",rsn);
#endif
    if (NX_SDMMC_ReadSectors(pSDXCBootStatus, rsn++, 1, pBL1Sector) == 0) {
#ifdef DEBUG
        _dprintf("NSIH read fail.\r\n");
#endif
    }

/* #ifdef DEBUG //BL1 NSIH check */
/*     { */
/*         unsigned int* temp = (unsigned int*)(BASEADDR_DRAM); */
/*         _dprintf("128 byte bl1 NSIH values are below ---\r\n"); */
/*         for (unsigned int i = 0; i < 128; i++) { */
/*             _dprintf("%x ",*(temp+i)); */
/*         } */
/*     } */
/* #endif */

    //---------------------------------------------------------------------------
    // get BL1 Image Size for seek disk
    //---------------------------------------------------------------------------
    bl1BinSize = pBL1BootInfo->LoadSize;
    dtbBinSize = pBL1BootInfo->dtbSize;
    pDTBSector = pBL1BootInfo->dtbLoadAddr;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] BL1 NSIH load addr = 0x%x\r\n",pBL1BootInfo->LoadAddr);
    _dprintf("[BL1-DEBUG] DTB BIN size = 0x%x\r\n",dtbBinSize);
    _dprintf("[BL1-DEBUG] DTB BIN load addr = 0x%x\r\n",pDTBSector);
#endif

    if (pBL1BootInfo->signature != HEADER_ID) {
#ifdef DEBUG
        _dprintf("\r\n[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
        _dprintf("[BL1-DEBUG] BL1 NSIH header check\r\n");
        _dprintf("[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
        _dprintf("[BL1-DEBUG] bl1 pbi sifnature addr = %x\r\n", &(pBL1BootInfo->signature));
        _dprintf("[BL1-DEBUG] bl1 expected HEADER_ID = %x\r\n", HEADER_ID);
#endif        
        return 0;
    }

    //---------------------------------------------------------------------------
    // Calculate BL1 Image size for seek
    //---------------------------------------------------------------------------
    //sector seek, BL1 body size + 1
    //---------------------------------------------------------------------------
    rsn += ((bl1BinSize + BLOCK_LENGTH - 1) / BLOCK_LENGTH);

    //clear DRAM base 0x80000000
    nx_memset(pBL1Sector, 0x00, BLOCK_LENGTH + bl1BinSize); //NSIH header 512byte + bl1 body size

    //=============================================================================================================
    // Vector Table
    //-------------------------------------------------------------------------------------------------------------
    // Vector Binary Loading to SRAM offset 0x7000 (4KB region) = BLOCK_LENGTH*8 = 512 * 8
    //-------------------------------------------------------------------------------------------------------------
#ifdef DEBUG
    _dprintf("\r\n[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] VECTOR loading in addr = 0x%x\r\n",pVectorTableSector);
    _dprintf("[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
#endif

    unsigned int vectorBinSectorSize = (4096 + BLOCK_LENGTH - 1) / BLOCK_LENGTH;
    
    if (NX_SDMMC_ReadSectors(pSDXCBootStatus, rsn, vectorBinSectorSize, pVectorTableSector) == 0) {
#ifdef DEBUG
        _dprintf("Vector Table read fail.\r\n");
#endif
    }
#ifdef DEBUG
    {
        unsigned int* temp = (unsigned int*)(0x40007000);
        _dprintf("128 byte vector table region values are below ---\r\n");
        for (unsigned int i = 0; i < 128; i++) {
            _dprintf("%x ",*(temp+i));
        }
        _dprintf("\r\n");
    }
#endif
    //=============================================================================================================

    //---------------------------------------------------------------------------
    //sector seek, BL1 body size + 1 + vector size + 1
    //---------------------------------------------------------------------------
#ifndef VECTOR_TEST
    rsn += vectorBinSectorSize;
    
    //=============================================================================================================
    // DTB
    //=============================================================================================================
    //---------------------------------------------------------------------------
    // DTB binary read
    //---------------------------------------------------------------------------
    unsigned int dtbBinSectorSize = (dtbBinSize + BLOCK_LENGTH - 1) / BLOCK_LENGTH;
    result = NX_SDMMC_ReadSectors(pSDXCBootStatus, rsn, dtbBinSectorSize, pDTBSector);
    rsn += dtbBinSectorSize;
#ifdef DEBUG
    {
        unsigned int* temp = (unsigned int*)pDTBSector;
        _dprintf("128 byte DTB values are below ---\r\n");
        for (unsigned int i = 0; i < 128; i++) {
            _dprintf("%x ",*(temp+i));
        }
        _dprintf("\r\n");
    }
#endif
    //=============================================================================================================
        

    //=============================================================================================================
    // BBL
    //-------------------------------------------------------------------------------------------------------------
    // BBL NSIH 512byte read
#ifdef DEBUG
    _dprintf("\r\n[BL1-DEBUG]++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] BBL NSIH header read\r\n");
    _dprintf("[BL1-DEBUG]++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] seek vector table, rsn = 0x%x\r\n",rsn);
#endif

    result = NX_SDMMC_ReadSectors(pSDXCBootStatus, rsn++, 1, pBBLSector);

#ifdef DEBUG
    {
        unsigned int* temp = (unsigned int*)(BASEADDR_DRAM);
        _dprintf("128 byte bbl NSIH values are below ---\r\n");
        for (unsigned int i = 0; i < 128; i++) {
            _dprintf("%x ",*(temp+i));
        }
        _dprintf("\r\n");
    }
#endif
    //-------------------------------------------------------------------------------------------------------------
    
#ifdef DEBUG
    _dprintf("\r\n[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] BBL BODY binary read to pData\r\n");
    _dprintf("[BL1-DEBUG]++++++++++++++++++++++++++++\r\n");
    _dprintf("[BL1-DEBUG] rsn = 0x%x\r\n",rsn);
#endif

    bblBinSize = pBBLBootInfo->LoadSize;
    if (pBBLBootInfo->signature != HEADER_ID) {
#ifdef DEBUG
        _dprintf("[BL1-DEBUG] bbl expected HEADER_ID = %x\r\n", HEADER_ID);
#endif
        return 0;
    }

    //-------------------------------------------------------------------------------------------------------------
    // BBL Binary
    //---------------------------------------------------------------------------
    // pBBLSector = (unsigned int*)(BASEADDR_DRAM);
    // clear DRAM BBL NSIH header, not neccesary anymore
    //---------------------------------------------------------------------------
    nx_memset(pBBLSector, 0x00, BLOCK_LENGTH); //NSIH header 512byte
    
    unsigned int bblBodySectorsize = (bblBinSize + BLOCK_LENGTH - 1) / BLOCK_LENGTH;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] BBL image size = 0x%x\r\n",bblBinSize);
    _dprintf("[BL1-DEBUG] pData addr = 0x%x\r\n",pBBLSector);
    _dprintf("[BL1-DEBUG] bblBodySectorsize = 0x%x\r\n",bblBodySectorsize);
#endif

    //---------------------------------------------------------------------------
    // BBL Image Loaing to DRAM 0x80000000
    //---------------------------------------------------------------------------
    result = NX_SDMMC_ReadSectors(pSDXCBootStatus, rsn, bblBodySectorsize, pBBLSector);

#ifdef DEBUG
    {
        unsigned int* temp = (unsigned int*)(pBBLSector);// + BLOCK_LENGTH);
        _dprintf("128 byte bbl BODY values are below ---\r\n");
        _dprintf("temp addr = 0x%x\r\n",temp);
        for (unsigned int i = 0; i < 128; i++) {
            _dprintf("%x ",*(temp+i));
        }
        _dprintf("\r\n");
    }
#endif
    //=============================================================================================================
    
#endif //VECTOR_TEST
    return result;

 error:
    return result;
}

void NX_SDPADSetALT(unsigned int PortNum)
{
 struct nxpadi sdmmc0_gpio_0 = { 1, 0, 0, 1 };
 struct nxpadi sdmmc0_gpio_1 = { 1, 1, 0, 1 };
 struct nxpadi sdmmc0_gpio_2 = { 1, 2, 0, 1 };
 struct nxpadi sdmmc0_gpio_3 = { 1, 3, 0, 1 };
 struct nxpadi sdmmc0_gpio_4 = { 1, 4, 0, 1 };
 struct nxpadi sdmmc0_gpio_5 = { 1, 6, 0, 1 };

 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CDATA_0_); //SDMMC_pad00_ref = 0x10001 */
 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CDATA_1_); //SDMMC_pad00_ref = 0x10009 */
 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CDATA_2_); //SDMMC_pad00_ref = 0x10011 */
 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CDATA_3_); //SDMMC_pad00_ref = 0x10019 */
 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CCLK);     //SDMMC_pad00_ref = 0x10021 */
 /* _dprintf("SDMMC_pad00_ref = 0x%x\r\n", PADINDEX_OF_OSDMMC0_CMD);      //SDMMC_pad00_ref = 0x10031 */

 setpadEx(sdmmc0_gpio_0, 1);
 setpadEx(sdmmc0_gpio_1, 1);
 setpadEx(sdmmc0_gpio_2, 1);
 setpadEx(sdmmc0_gpio_3, 1);
 setpadEx(sdmmc0_gpio_4, 1);
 setpadEx(sdmmc0_gpio_5, 1);
    /* setpad(sdmmcpad[PortNum][0].padi, 1); */
    /* setpad(sdmmcpad[PortNum][1].padi, 1); */
    /* setpad(sdmmcpad[PortNum][2].padi, 1); */
    /* setpad(sdmmcpad[PortNum][3].padi, 1); */
    /* setpad(sdmmcpad[PortNum][4].padi, 1); */
    /* setpad(sdmmcpad[PortNum][5].padi, 1); */
}

void NX_SDPADSetGPIO(unsigned int PortNum)
{
 struct nxpadi sdmmc0_gpio_0 = { 1, 0, 0, 1 };
 struct nxpadi sdmmc0_gpio_1 = { 1, 1, 0, 1 };
 struct nxpadi sdmmc0_gpio_2 = { 1, 2, 0, 1 };
 struct nxpadi sdmmc0_gpio_3 = { 1, 3, 0, 1 };
 struct nxpadi sdmmc0_gpio_4 = { 1, 4, 0, 1 };
 struct nxpadi sdmmc0_gpio_5 = { 1, 6, 0, 1 };

 setpadEx(sdmmc0_gpio_0, 0);
 setpadEx(sdmmc0_gpio_1, 0);
 setpadEx(sdmmc0_gpio_2, 0);
 setpadEx(sdmmc0_gpio_3, 0);
 setpadEx(sdmmc0_gpio_4, 0);
 setpadEx(sdmmc0_gpio_5, 0);

    /* setpad(sdmmcpad[PortNum][0].padi, 0); */
    /* setpad(sdmmcpad[PortNum][1].padi, 0); */
    /* setpad(sdmmcpad[PortNum][2].padi, 0); */
    /* setpad(sdmmcpad[PortNum][3].padi, 0); */
    /* setpad(sdmmcpad[PortNum][4].padi, 0); */
    /* setpad(sdmmcpad[PortNum][5].padi, 0); */
}

//------------------------------------------------------------------------------
unsigned int iSDBOOT(void)
{
    SDBOOTSTATUS SDXCBootStatus, *pSDXCBootStatus;
    int	result = 0;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] iSDBOOT start \r\n");
#endif
    pSDXCBootStatus = &SDXCBootStatus;

#ifdef DEBUG
    _dprintf("[BL1-DEBUG] pSDXCBootStatus = 0x%x \r\n",pSDXCBootStatus);
#endif
    pSDXCBootStatus->SDPort = 0;
    pSDXCBootStatus->bHighSpeed = 0;

    NX_SDPADSetALT(pSDXCBootStatus->SDPort);
    NX_SDMMC_Init(pSDXCBootStatus);
    
    result = SDMMCBOOT(pSDXCBootStatus);
    
    NX_SDMMC_Close(pSDXCBootStatus);
    NX_SDMMC_Terminate(pSDXCBootStatus);

    NX_SDPADSetGPIO(pSDXCBootStatus->SDPort);
    
    return result;
}
