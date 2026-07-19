/* $Id: zynq_qspi.h,v 1.5 2018/07/23 07:02:22 easochen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/zynq_qspi.h,v $
 *
 * zynq_qspi.h - definitions for qspi driver
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */
/* Flash Type */
#define S25FL129P	1
#define S25FL128S	2
#define MT25QL128	3

/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO     dmb()

/* #define QSPI_DEBUG         1 */
#define ZYNQ_QSPI0_BASE          0xE000D000
#define QSPI_MMAP_LEN            0x1000

/* 0-3, 154-255 */
#define QSPI_PROTECT_FW_ADDR             0x000000
#define QSPI_PROTECT_FW_SEC_START        0
#define QSPI_PROTECT_FW_SECNUM           4
#ifdef PLUG_SER
#define QSPI_PROTECT_GOLDENIMG_ADDR      0x680000
#define QSPI_PROTECT_GOLDENIMG_SEC_START 104
#define QSPI_PROTECT_GOLDENIMG_SECNUM    100
#define QSPI_PROTECT_SECNUM              100
#elif REVA
#define QSPI_PROTECT_GOLDENIMG_ADDR      0x680000
#define QSPI_PROTECT_GOLDENIMG_SEC_START 104
#define QSPI_PROTECT_GOLDENIMG_SECNUM     98
#define QSPI_PROTECT_SECNUM               98
#else
#define QSPI_PROTECT_GOLDENIMG_ADDR      0x9A0000
#define QSPI_PROTECT_GOLDENIMG_SEC_START 154
#define QSPI_PROTECT_GOLDENIMG_SECNUM    102
#define QSPI_PROTECT_SECNUM              106
#endif
/* QSPI size infomation */
#define SECTOR_SIZE		0x10000
#define SECTOR_SIZE_SMALL     0x01000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		256
#define QSPI_SIZE               0x1000000
#define READ_SIZE               128

/* Register offset definitions */

/* Configuration  Register, RW */
#define QSPI_CONFIG_OFFSET        0x00
/* Interrupt Status Register, RO, WTC */
#define QSPI_STATUS_OFFSET        0x04
/* Interrupt Enable Register, WO */
#define QSPI_IEN_OFFSET           0x08
/* Interrupt Disable Reg, WO */
#define QSPI_IDIS_OFFSET          0x0C
/* Interrupt Enabled Mask Reg, RO */
#define QSPI_IMASK_OFFSET         0x10
/* Enable/Disable Register, RW */
#define QSPI_ENABLE_OFFSET        0x14
/* Delay Register, RW */
#define QSPI_DELAY_OFFSET         0x18
/* Transmit 4-byte inst/data, WO */
#define QSPI_TXD_00_OFFSET        0x1C
/* Transmit 1-byte inst, WO */
#define QSPI_TXD_01_OFFSET        0x80
/* Transmit 2-byte inst, WO */
#define QSPI_TXD_10_OFFSET        0x84
/* Transmit 3-byte inst, WO */
#define QSPI_TXD_11_OFFSET        0x88
/* Data Receive Register, RO */
#define QSPI_RXD_OFFSET           0x20
/* Slave Idle Count Register, RW */
#define QSPI_SIC_OFFSET           0x24
/* TX FIFO Watermark Reg, RW */
#define QSPI_TX_THRESH_OFFSET     0x28
/* RX FIFO Watermark Reg, RW */
#define QSPI_RX_THRESH_OFFSET     0x2C
/* LPBK Delay Adjustment Register, RW */
#define QSPI_LPBK_DLYADJ_OFFSET   0x38
/* Linear Adapter Config Ref, RW */
#define QSPI_LINEAR_CFG_OFFSET    0xA0
/* Linear QSPI status register, RW*/
#define QSPI_LINEAR_SR_OFFSET     0xA4
/* Module ID Register, RO */
#define QSPI_MOD_ID_OFFSET        0xFC

/* Config Register */
#define QSPI_CONFIG_IFMODE_MASK    0x80000000    /* Flash mem interface mode */
#define QSPI_CONFIG_ENDIAN_MASK    0x04000000    /* Tx/Rx FIFO endianness */
#define QSPI_CONFIG_MANSTRT_MASK   0x00010000    /* Manual Transmission Start */
#define QSPI_CONFIG_MANSTRTEN_MASK 0x00008000    /* Manual Transmission Start Enable */
#define QSPI_CONFIG_HOLD_B_MASK    0x00080000
/* Force Slave Select after per transfer so that more than 
   FIFO size can be transfered via a single command in manual mode */
#define QSPI_CONFIG_SSFORCE_MASK   0x00004000
#define QSPI_CONFIG_SSCTRL_MASK    0x00003C00    /* Slave Select Decode */
#define QSPI_CONFIG_SSCTRL_SHIFT   10            /* Slave Select Decode shift */
#define QSPI_CONFIG_SSCTRL_MAXIMUM 0x03          /* Slave Select maximum value */
#define QSPI_CONFIG_SSDECEN_MASK   0x00000200    /* Slave Select Decode Enable */

#define QSPI_CONFIG_DATA_SZ_MASK   0x000000C0    /* Size of word to be transferred */
#define QSPI_CONFIG_PRESC_MASK     0x00000038    /* Prescaler Setting */
#define QSPI_CONFIG_PRESC_SHIFT    3             /* Prescaler shift */
#define QSPI_CONFIG_PRESC_MAXIMUM  0x07          /* Prescaler maximum value */

#define QSPI_CONFIG_CPHA_MASK      0x00000004    /* Phase Configuration */
#define QSPI_CONFIG_CPOL_MASK      0x00000002    /* Polarity Configuration */

#define QSPI_CONFIG_MSTREN_MASK    0x00000001    /* Master Mode Enable */

/* Deselect all the SS lines and set the transfer size to 32 at reset */
#define QSPI_CONFIG_RESET_STATE    (QSPI_CONFIG_IFMODE_MASK | \
				    QSPI_CONFIG_DATA_SZ_MASK | \
				    QSPI_CONFIG_MSTREN_MASK)
#define QSPI_CONFIG_TEST           0x840004C1

#define QSPI_IXR_TXNFULL_MASK      0x00000004    /* QSPI Tx FIFO Not Full*/
#define QSPI_IXR_TXUF_MASK	   0x00000040    /* QSPI Tx FIFO Underflow */
#define QSPI_IXR_RXFULL_MASK       0x00000020    /* QSPI Rx FIFO Full */
#define QSPI_IXR_RXNEMPTY_MASK     0x00000010    /* QSPI Rx FIFO Not Empty */
#define QSPI_IXR_TXFULL_MASK       0x00000008    /* QSPI Tx FIFO Full */
#define QSPI_IXR_TXOW_MASK	   0x00000004    /* QSPI Tx FIFO Overwater */
#define QSPI_IXR_MODF_MASK	   0x00000002    /* QSPI Mode Fault */
#define QSPI_IXR_RXOVR_MASK	   0x00000001    /* QSPI Rx FIFO Overrun */
/* QSPI enable interrupts mask */
#define QSPI_IER_EN                0x0000007D
#define QSPI_IXR_EN_MASK           0x0000007D
/* QSPI disable interrupts mask */
#define QSPI_IDR_DIS               0x0000007F
#define QSPI_IXR_DIS_MASK          0x00000000


#define QSPI_ISR_RESET_STATE       0x04
#define QSPI_ISR_RESET             0x00000041

#define QSPI_ENABLE_MASK           0x00000001
#define QSPI_ENABLE_RESET          0x00000000

#define QSPI_DELAY_BTWN_MASK	   0x00FF0000    /*Delay Between Transfers mask */
#define QSPI_DELAY_BTWN_SHIFT	   16	         /* Delay Between Transfers shift */
#define QSPI_DELAY_AFTER_MASK	   0x0000FF00    /* Delay After Transfers mask */
#define QSPI_DR_AFTER_SHIFT	   8 	         /* Delay After Transfers shift */
#define QSPI_DR_INIT_MASK	   0x000000FF    /* Delay Initially mask */
#define QSPI_DR_NODELAY_MASK       0x00000000
#define QSPI_DR_TEST               0x55555555

/* Transmit FIFO Watermark Register
 * This register defines the watermark setting for the Transmit FIFO.
 */
#define QSPI_TXWR_MASK             0x0000003F    /* Transmit Watermark Mask */
#define QSPI_FIFO_DEPTH            63            /* Transmit FIFO depth (words) */

/* Linear QSPI Configuration Register */

#define QSPI_LQSPI_CR_LINEAR_MASK	 0x80000000 /* LQSPI mode enable */
#define QSPI_LQSPI_CR_TWO_MEM_MASK	 0x40000000 /* Both memories or one */
#define QSPI_LQSPI_CR_SEP_BUS_MASK	 0x20000000 /* Seperate memory bus */
#define QSPI_LQSPI_CR_U_PAGE_MASK	 0x10000000 /* Upper memory page */
#define QSPI_LQSPI_CR_CMD_MERGE_MASK     0x04000000 /* Merge back to back AXI commands */
#define QSPI_LQSPI_CR_MODE_EN_MASK	 0x02000000 /* Enable mode bits */
#define QSPI_LQSPI_CR_MODE_ON_MASK	 0x01000000 /* Mode on */
#define QSPI_LQSPI_CR_MODE_BITS_MASK     0x00FF0000 /* Mode value for dual I/O or quad I/O */
#define QSPI_LQSPI_CR_RD_ZEROS_MASK	 0x00000800 /* Zero out all read data */
/* Number of dummy bytes between addr and return read data */
#define QSPI_LQSPI_CR_DUMMY_MASK	 0x00000700
#define QSPI_LQSPI_CR_INST_MASK	         0x000000FF /* Read instr code */
#define QSPI_LQSPI_CR_RST_STATE          0x0400000B /* Non Linear mode */
#define QSPI_LQSPI_CR_MASK               0x8400016B

/* Linear QSPI Status Register */
#define QSPI_LQSPI_SR_CMD_MERGED_MASK    0x00000100 /* AXI read commands have been merged */
#define QSPI_LQSPI_SR_FB_RECVD_MASK	 0x00000004 /* AXI fixed burst command received */
#define QSPI_LQSPI_SR_WR_RECVD_MASK	 0x00000002 /* AXI write command received */
#define QSPI_LQSPI_SR_UNKN_INST_MASK     0x00000001 /* Unknown read inst code */
#define QSPI_LQSPI_SR_RST_STATE          0x00000000

#define QSPI_LINEAR_BASEADDR             0xFC000000

/* Mod ID Register reset value */
#define QSPI_MOD_ID_RESET                0x01090101
/* LPBK Delay Adjustment Register reset value */
#define QSPI_LPBK_DLYADJ_RESET           0x00000033
/* Slave Idle Count Registers reset value 
 * pclk cycles the slave waits for QSPI clock to become stable 
 * it can detect the start of the next transfer in CPHA = 1 mode.
 */
#define QSPI_SICR_RESET                  0x000000FF

/* QSPI Clock Configuration to program master mode bit rate
 * The bit rate can be programmed from pclk/2 to pclk/256.
 */
#define QSPI_CLK_PRESCALE_2		0x00 /**< PCLK/2 Prescaler */
#define QSPI_CLK_PRESCALE_4		0x01 /**< PCLK/4 Prescaler */
#define QSPI_CLK_PRESCALE_8		0x02 /**< PCLK/8 Prescaler */
#define QSPI_CLK_PRESCALE_16		0x03 /**< PCLK/16 Prescaler */
#define QSPI_CLK_PRESCALE_32		0x04 /**< PCLK/32 Prescaler */
#define QSPI_CLK_PRESCALE_64		0x05 /**< PCLK/64 Prescaler */
#define QSPI_CLK_PRESCALE_128	        0x06 /**< PCLK/128 Prescaler */
#define QSPI_CLK_PRESCALE_256	        0x07 /**< PCLK/256 Prescaler */

/* commands to be sent to the FLASH device.*/

#define WRITE_STATUS_CMD	0x01
#define WRITE_STATUS_CMD2	0x07
#define WRITE_CMD		0x02
#define QUAD_WRITE_CMD          0x32
#define READ_CMD		0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define READ_CONF_CMD		0x35
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F
#define WRITE_LOCK_CMD          0xE5
#define READ_LOCK_CMD           0xE8
#define CLEAR_STATUS_CMD        0x30
#define READ_ASP_CMD            0x2B
#define WRITE_ASP_CMD           0x2F
#define READ_PPB_CMD            0xE2
#define WRITE_PPB_CMD           0xE3
#define ERASE_PPB_CMD           0xE4
#define READ_PPBL_CMD           0xA7
#define WRITE_PPBL_CMD          0xA6
/* MT25QL128 specific commands sent to FLASH device.*/
#define MT25QL128_READ_ASPRD    0x2D
#define MT25QL128_ASPP          0x2C
#define READ_MT25QL128_CONF_CMD 0x70
#define CLEAR_MT25QL128_FSR_CMD 0x50


#define	QSPI_IS_DATA		0x00     /* Data-only transfer */
#define	QSPI_IS_INST		0x01     /* The fist bytes in a transfer is instruction */

/*
 * Extra bytes to be sent to QSPI, not data but control information,
 * which includes the command and address
 */
#define COMMAND_OFFSET		0     /* FLASH instruction */
#define ADDRESS_1_OFFSET	1     /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2     /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3     /* LSB byte of address to read or write */
#define DATA_OFFSET		4     /* Start of Data for Read/Write */
#define S25FL_DIFF_OFFSET	0x46  /* S25FL diff byte offset of ID-CFI */
#define DUMMY_OFFSET		4     /* Dummy byte offset for fast, dual and quad reads */
#define DUMMY_SIZE		1     /* Number of dummy bytes for fast, dual and quad reads */
#define RD_ID_SIZE		4     /* Read ID command + 3 bytes ID response */
#define RD_CFI_SIZE		0x50  /* ID-CFI map bytes 10h-50h*/
#define BULK_ERASE_SIZE		1     /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4     /* Sector Erase command + Sector address */
#define WRITE_LOCK_SIZE		4     /* WRITE LOCK command + Sector address */
#define OVERHEAD_SIZE		4     /* first specified bytes are command and address */ 

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the FLASH.
 */
#define MAX_DATA		PAGE_COUNT * PAGE_SIZE

#define QSPI0_SELECT            0x00

/* Write status register */
#define	QSPI_OPCODE_WRSR	0x01
#define	QSPI_OPCODE_WRSR2	0x07
/* Page program */
#define	QSPI_OPCODE_PP		0x02
/* Quad Page program */
#define	QSPI_OPCODE_QP		0x32
/* Normal read data bytes */
#define	QSPI_OPCODE_NORM_READ	0x03
/* Write disable */
#define	QSPI_OPCODE_WRDS	0x04
/* Read status register 1 */
#define	QSPI_OPCODE_RDSR1	0x05
#define	QSPI_OPCODE_MT_RDFSR	0x70  /* Micron MT25QL128 */
/* Write enable */
#define	QSPI_OPCODE_WREN	0x06
/* Fast read data bytes */
#define	QSPI_OPCODE_FAST_READ	0x0B
/* Erase 4KiB block */
#define	QSPI_OPCODE_BE_4K       0x20
/* Read config register */
#define	QSPI_OPCODE_RCR 	0x35
/* Dual read data bytes */
#define	QSPI_OPCODE_DUAL_READ	0x3B
/* Erase 32KiB block */
#define	QSPI_OPCODE_BE_32K	0x52
/* Quad read data bytes */
#define	QSPI_OPCODE_QUAD_READ	0x6B
/* Erase suspend */
#define	QSPI_OPCODE_ERASE_SUS	0x75
/* Erase resume */
#define	QSPI_OPCODE_ERASE_RES	0x7A
/* Read JEDEC ID */
#define	QSPI_OPCODE_RDID	0x9F
/* Erase whole flash block */
#define	QSPI_OPCODE_BE          0xC7
/* Sector erase (usually 64KB)*/
#define	QSPI_OPCODE_SE	        0xD8
/* Write to Lock Register */
#define	QSPI_OPCODE_WRLR        0xE5
/* Read Lock Register */
#define	QSPI_OPCODE_RDLR        0xE8
/* Clear Status Register */
#define	QSPI_OPCODE_CLSR        0x30
#define	QSPI_OPCODE_MT_CLRFSR   0x50  /* Micron MT25QL128 */
/* Read ASP Register */
#define	QSPI_OPCODE_ASPRD       0x2B
#define	QSPI_OPCODE_MT_ASPRD    0x2C  /* Micron MT25QL128 */
/* Write ASP Register */
#define	QSPI_OPCODE_ASPP        0x2F
#define	QSPI_OPCODE_MT_ASPP     0x2D  /* Micron MT25QL128 */

/* Read PPB Register */
#define QSPI_OPCODE_PPBRD       0xE2
/* Write PPB Register */
#define QSPI_OPCODE_PPBP        0xE3
/* Erase PPB Register */
#define QSPI_OPCODE_PPBE        0xE4
/* Read PPB Lock Register */
#define QSPI_OPCODE_PLBRD       0xA7
/* Write PPB Lock Register */
#define QSPI_OPCODE_PLBWR       0xA6

/* QSPI device for each transfer */
typedef struct XQspidev {
    uchar *SendBuffer;
    uchar *RecvBuffer;
    int RequestedBytes;	     /* Number of bytes to transfer (state) */
    int RemainingBytes;	     /* Number of bytes left to transfer(state) */
    uint32_t IsReady;        /* Device is initialized and ready */
    uint32_t IsBusy;         /* A transfer is in progress (state) */
    uint32_t SlaveSelect;    /* The slave select line that needs to be asserted during a transfer */
} XQspi;

/* qspi flash instruction format */
typedef struct XQspiInstr {
    uchar OpCode;    /* Operational code of the instruction */
    uchar InstSize;  /*  Size of the instruction including address bytes */
    uchar TxOffset;  /* Register address where instruction to be written */
} XQspiInstFormat;

/* List of all the QSPI instructions and its format */
static XQspiInstFormat QspiInst[] = {
    { QSPI_OPCODE_WREN, 1, QSPI_TXD_01_OFFSET},
    { QSPI_OPCODE_WRDS, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_CLSR, 1, QSPI_TXD_01_OFFSET},
    { QSPI_OPCODE_RDSR1, 2, QSPI_TXD_10_OFFSET },
    { QSPI_OPCODE_RCR,   2, QSPI_TXD_10_OFFSET },
    { QSPI_OPCODE_WRSR, 3, QSPI_TXD_11_OFFSET },
    { QSPI_OPCODE_WRSR2, 2, QSPI_TXD_10_OFFSET },
    { QSPI_OPCODE_WRLR, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_RDLR, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_PP, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_QP, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_SE, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_BE_32K, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_BE_4K, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_BE, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_ERASE_SUS, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_ERASE_RES, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_RDID, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_NORM_READ, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_FAST_READ, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_DUAL_READ, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_QUAD_READ, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_QUAD_READ, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_ASPRD, 3, QSPI_TXD_11_OFFSET },
    { QSPI_OPCODE_ASPP,  3, QSPI_TXD_11_OFFSET },
    { QSPI_OPCODE_PPBRD, 4, QSPI_TXD_00_OFFSET },
    { QSPI_OPCODE_PPBP,  1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_PPBE,  1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_PLBRD, 2, QSPI_TXD_10_OFFSET },
    { QSPI_OPCODE_PLBWR, 1, QSPI_TXD_01_OFFSET },
    { QSPI_OPCODE_MT_ASPRD, 3, QSPI_TXD_11_OFFSET }, /* Micron MT25QL128 */
    { QSPI_OPCODE_MT_ASPP,  3, QSPI_TXD_11_OFFSET }, /* Micron MT25QL128 */
    { QSPI_OPCODE_MT_CLRFSR,1, QSPI_TXD_01_OFFSET }, /* Micron MT25QL128 */
    { QSPI_OPCODE_MT_RDFSR, 2, QSPI_TXD_10_OFFSET }, /* Micron MT25QL128 */
    /* Add all the instructions supported by the flash device */
};
/*******************************************************************************
Flag Status Register Definitions (See MT25QL128 Datasheet)
*******************************************************************************/
enum {
	SPI_FSR_PROG_ERASE_CTL		= 0x80,
	SPI_FSR_ERASE_SUSP		= 0x40,
	SPI_FSR_ERASE			= 0x20,
	SPI_FSR_PROGRAM			= 0x10,
	SPI_FSR_VPP			= 0x08,
	SPI_FSR_PROG_SUSP		= 0x04,
	SPI_FSR_PROT			= 0x02,
	SPI_FSR_ADDR_MODE		= 0x01
};

#define QspiInstSize    sizeof(QspiInst)/sizeof(XQspiInstFormat)

/******** History ******** 
$Log: zynq_qspi.h,v $
Revision 1.5  2018/07/23 07:02:22  easochen
Support golden image protection with Micron flash

Revision 1.4  2013/12/20 08:10:25  xiaoyizh
Updated golden image location according to the latest PCAMAP.

Revision 1.3  2013/09/03 07:04:58  liwwang
add Spansion S25FL128S QSPI ASP support and image protection utility support

Revision 1.2  2013/07/16 04:15:50  liwwang
add support for sector lock and new type flash

Revision 1.1  2013/04/19 07:17:54  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
