/* $Id: patriot_main.h,v 1.1 2014/03/25 02:12:34 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_main.h
 *
 * Description: This file is for constant definitions and function prototypes
 *              for Patriot
 *      
 *
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

#define TEST_OK                                  0x40
#define TEST_ACK                                 0x80
#define TEST_FAIL                                0xC0  


#define FROM_HOST_CPU_ALIVE_TEST                 0x01
#define FROM_HOST_READ_FPGA_VERSION              0x02
#define FROM_HOST_FPGA_DOWNLOAD_TO_FPGA          0x03
#define FROM_HOST_FPGA_REG_TEST                  0x04
#define FROM_HOST_MEMORY_TEST                    0x05
#define FROM_HOST_SPI_PROM_TEST                  0x06
#define FROM_HOST_DS3170_REG_TEST                0x07
#define FROM_HOST_E3_AIS_TEST                    0x08
#define FROM_HOST_CLR_T3_INTR_TEST               0x09
#define FROM_HOST_CLR_T3_BERT_TEST               0x0A
#define FROM_HOST_FREESCALE_LPBK_TEST            0x0B
#define FROM_HOST_CLR_T3_LPBK_TEST               0x0C
#define FROM_HOST_CLR_T3_EX_LPBK_TEST            0x0D
#define FROM_HOST_SUB_T3_LPBK_TEST               0x0E
#define FROM_HOST_SUB_T3_EX_LPBK_TEST            0x0F
#define FROM_HOST_CLR_E3_LPBK_TEST               0x10
#define FROM_HOST_CLR_E3_EX_LPBK_TEST            0x11
#define FROM_HOST_SUB_E3_LPBK_TEST               0x12
#define FROM_HOST_SUB_E3_EX_LPBK_TEST            0x13
#define FROM_HOST_FREESCALE_MAC_LPBK_TEST        0x14
#define FROM_HOST_FPGA_LPBK_TEST                 0x15
#define FROM_HOST_FPGA_RESET                     0x16
#define FROM_HOST_DS3170_RESET                   0x17
#define FROM_HOST_LED_TEST                       0x18
#define FROM_HOST_LED_DISPLAY                    0x19
#define FROM_HOST_FW_DOWNLOAD_TO_DDR             0x1A
#define FROM_HOST_FW_DOWNLOAD_TO_SPI_PROM        0x1B
#define FROM_HOST_SWITCH_CONSOLE                 0x1C
#define FROM_HOST_FREESCALE_UCC_LPBK_TEST        0x1D
#define FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST     0x1E
#define FROM_HOST_WRITE_MAC_ADDR                 0x1F
#define FROM_HOST_CPU_GPIO_TEST_IO1_W1           0x20
#define FROM_HOST_CPU_GPIO_TEST_IO1_W0           0x21
#define FROM_HOST_CPU_GPIO_TEST_IO4_W1           0x22
#define FROM_HOST_CPU_GPIO_TEST_IO4_W0           0x23
#define FROM_HOST_CPU_GPIO_TEST_IO3_R1           0x24
#define FROM_HOST_CPU_GPIO_TEST_IO3_R0           0x25
#define FROM_HOST_FPGA_INTR_TEST                 0x26
#define FROM_HOST_SUB_T3_IND_LPBK_TEST           0x27
#define FROM_HOST_SUB_T3_IND_EX_LPBK_TEST        0x28
#define FROM_HOST_SUB_E3_IND_LPBK_TEST           0x29
#define FROM_HOST_SUB_E3_IND_EX_LPBK_TEST        0x2A
#define FROM_HOST_ECC_TEST                       0x2B
#define FROM_HOST_UART_TEST                      0x2C
#define FROM_HOST_GE0_LPBK_TEST                  0x2D
#define FROM_HOST_POWER_ALTER_NO_MARGIN          0x2E
#define FROM_HOST_POWER_ALTER_LOW_MARGIN         0x2F
#define FROM_HOST_POWER_ALTER_HIGH_MARGIN        0x30
#define FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM  0x31
#define FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM  0x32
#define FROM_HOST_FPGA_READ_INFO                 0x33


#define TO_HOST_CPU_ALIVE_TEST_ACK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_ACK)
#define TO_HOST_READ_FPGA_VERSION_ACK \
        (FROM_HOST_READ_FPGA_VERSION + TEST_ACK)
#define TO_HOST_FPGA_DOWNLOAD_TO_FPGA_ACK \
        (FROM_HOST_FPGA_DOWNLOAD_TO_FPGA + TEST_ACK)
#define TO_HOST_FPGA_REG_TEST_ACK \
        (FROM_HOST_FPGA_REG_TEST + TEST_ACK)
#define TO_HOST_MEMORY_TEST_ACK \
        (FROM_HOST_MEMORY_TEST + TEST_ACK)
#define TO_HOST_SPI_PROM_TEST_ACK \
        (FROM_HOST_SPI_PROM_TEST + TEST_ACK)
#define TO_HOST_DS3170_REG_TEST_ACK \
        (FROM_HOST_DS3170_REG_TEST + TEST_ACK)
#define TO_HOST_E3_AIS_TEST_ACK \
        (FROM_HOST_E3_AIS_TEST + TEST_ACK)
#define TO_HOST_CLR_T3_INTR_TEST_ACK \
        (FROM_HOST_CLR_T3_INTR_TEST + TEST_ACK)
#define TO_HOST_CLR_T3_BERT_TEST_ACK \
        (FROM_HOST_CLR_T3_BERT_TEST + TEST_ACK)
#define TO_HOST_FREESCALE_LPBK_TEST_ACK \
        (FROM_HOST_FREESCALE_LPBK_TEST + TEST_ACK)
#define TO_HOST_CLR_T3_LPBK_TEST_ACK \
        (FROM_HOST_CLR_T3_LPBK_TEST + TEST_ACK)
#define TO_HOST_CLR_T3_EX_LPBK_TEST_ACK \
        (FROM_HOST_CLR_T3_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_T3_LPBK_TEST_ACK \
        (FROM_HOST_SUB_T3_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_T3_EX_LPBK_TEST_ACK \
        (FROM_HOST_SUB_T3_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_CLR_E3_LPBK_TEST_ACK \
        (FROM_HOST_CLR_E3_LPBK_TEST + TEST_ACK)
#define TO_HOST_CLR_E3_EX_LPBK_TEST_ACK \
        (FROM_HOST_CLR_E3_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_E3_LPBK_TEST_ACK \
        (FROM_HOST_SUB_E3_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_E3_EX_LPBK_TEST_ACK \
        (FROM_HOST_SUB_E3_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_FREESCALE_MAC_LPBK_TEST_ACK \
        (FROM_HOST_FREESCALE_MAC_LPBK_TEST + TEST_ACK)
#define TO_HOST_FPGA_LPBK_TEST_ACK \
        (FROM_HOST_FPGA_LPBK_TEST + TEST_ACK)
#define TO_HOST_FPGA_RESET_ACK \
        (FROM_HOST_FPGA_RESET + TEST_ACK)
#define TO_HOST_DS3170_RESET_ACK \
        (FROM_HOST_DS3170_RESET + TEST_ACK)
#define TO_HOST_LED_TEST_ACK \
        (FROM_HOST_LED_TEST + TEST_ACK)
#define TO_HOST_LED_DISPLAY_ACK \
        (FROM_HOST_LED_DISPLAY + TEST_ACK)
#define TO_HOST_FW_DOWNLOAD_TO_DDR_ACK \
        (FROM_HOST_FW_DOWNLOAD_TO_DDR + TEST_ACK)
#define TO_HOST_FW_DOWNLOAD_TO_SPI_PROM_ACK \
        (FROM_HOST_FW_DOWNLOAD_TO_SPI_PROM + TEST_ACK)
#define TO_HOST_SWITCH_CONSOLE_ACK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_ACK)
#define TO_HOST_FREESCALE_UCC_LPBK_TEST_ACK \
        (FROM_HOST_FREESCALE_UCC_LPBK_TEST + TEST_ACK)
#define TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_ACK \
        (FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST + TEST_ACK)
#define TO_HOST_WRITE_MAC_ADDR_ACK \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W1_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W1 + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W0_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W0 + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W1_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W1 + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W0_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W0 + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R1_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R1 + TEST_ACK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R0_ACK \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R0 + TEST_ACK)
#define TO_HOST_FPGA_INTR_TEST_ACK \
        (FROM_HOST_FPGA_INTR_TEST + TEST_ACK)
#define TO_HOST_SUB_T3_IND_LPBK_TEST_ACK \
		(FROM_HOST_SUB_T3_IND_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_T3_IND_EX_LPBK_TEST_ACK \
		(FROM_HOST_SUB_T3_IND_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_E3_IND_LPBK_TEST_ACK  \
		(FROM_HOST_SUB_E3_IND_LPBK_TEST + TEST_ACK)
#define TO_HOST_SUB_E3_IND_EX_LPBK_TEST_ACK  \
		(FROM_HOST_SUB_E3_IND_EX_LPBK_TEST + TEST_ACK)
#define TO_HOST_ECC_TEST_ACK \
	    (FROM_HOST_ECC_TEST + TEST_ACK)
#define TO_HOST_UART_TEST_ACK \
	    (FROM_HOST_UART_TEST + TEST_ACK)
#define TO_HOST_GE0_LPBK_TEST_ACK \
        (FROM_HOST_GE0_LPBK_TEST + TEST_ACK)
#define TO_HOST_POWER_NO_MARGIN_ACK  \
	    (FROM_HOST_POWER_ALTER_NO_MARGIN + TEST_ACK)
#define TO_HOST_POWER_LOW_MARGIN_ACK  \
	    (FROM_HOST_POWER_ALTER_LOW_MARGIN + TEST_ACK)
#define TO_HOST_POWER_HIGH_MARGIN_ACK  \
	    (FROM_HOST_POWER_ALTER_HIGH_MARGIN + TEST_ACK)
#define TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_ACK \
        (FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_ACK)
#define TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_ACK \
        (FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_ACK)
#define TO_HOST_FPGA_READ_INFO_ACK \
	    (FROM_HOST_FPGA_READ_INFO + TEST_ACK)


#define TO_HOST_CPU_ALIVE_TEST_OK \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_OK)
#define TO_HOST_READ_FPGA_VERSION_OK \
        (FROM_HOST_READ_FPGA_VERSION + TEST_OK)
#define TO_HOST_FPGA_DOWNLOAD_TO_FPGA_OK \
        (FROM_HOST_FPGA_DOWNLOAD_TO_FPGA + TEST_OK)
#define TO_HOST_FPGA_REG_TEST_OK \
        (FROM_HOST_FPGA_REG_TEST + TEST_OK)
#define TO_HOST_MEMORY_TEST_OK \
        (FROM_HOST_MEMORY_TEST + TEST_OK)
#define TO_HOST_SPI_PROM_TEST_OK \
        (FROM_HOST_SPI_PROM_TEST + TEST_OK)
#define TO_HOST_DS3170_REG_TEST_OK \
        (FROM_HOST_DS3170_REG_TEST + TEST_OK)
#define TO_HOST_E3_AIS_TEST_OK \
        (FROM_HOST_E3_AIS_TEST + TEST_OK)
#define TO_HOST_CLR_T3_INTR_TEST_OK \
        (FROM_HOST_CLR_T3_INTR_TEST + TEST_OK)
#define TO_HOST_CLR_T3_BERT_TEST_OK \
        (FROM_HOST_CLR_T3_BERT_TEST + TEST_OK)
#define TO_HOST_FREESCALE_LPBK_TEST_OK \
        (FROM_HOST_FREESCALE_LPBK_TEST + TEST_OK)
#define TO_HOST_CLR_T3_LPBK_TEST_OK \
        (FROM_HOST_CLR_T3_LPBK_TEST + TEST_OK)
#define TO_HOST_CLR_T3_EX_LPBK_TEST_OK \
        (FROM_HOST_CLR_T3_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_T3_LPBK_TEST_OK \
        (FROM_HOST_SUB_T3_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_T3_EX_LPBK_TEST_OK \
        (FROM_HOST_SUB_T3_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_CLR_E3_LPBK_TEST_OK \
        (FROM_HOST_CLR_E3_LPBK_TEST + TEST_OK)
#define TO_HOST_CLR_E3_EX_LPBK_TEST_OK \
        (FROM_HOST_CLR_E3_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_E3_LPBK_TEST_OK \
        (FROM_HOST_SUB_E3_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_E3_EX_LPBK_TEST_OK \
        (FROM_HOST_SUB_E3_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_FREESCALE_MAC_LPBK_TEST_OK \
        (FROM_HOST_FREESCALE_MAC_LPBK_TEST + TEST_OK)
#define TO_HOST_FPGA_LPBK_TEST_OK \
        (FROM_HOST_FPGA_LPBK_TEST + TEST_OK)
#define TO_HOST_FPGA_RESET_OK \
        (FROM_HOST_FPGA_RESET + TEST_OK)
#define TO_HOST_DS3170_RESET_OK \
        (FROM_HOST_DS3170_RESET + TEST_OK)
#define TO_HOST_LED_TEST_OK \
        (FROM_HOST_LED_TEST + TEST_OK)
#define TO_HOST_LED_DISPLAY_OK \
        (FROM_HOST_LED_DISPLAY + TEST_OK)
#define TO_HOST_FW_DOWNLOAD_TO_DDR_OK \
        (FROM_HOST_FW_DOWNLOAD_TO_DDR + TEST_OK)
#define TO_HOST_FW_DOWNLOAD_TO_SPI_PROM_OK \
        (FROM_HOST_FW_DOWNLOAD_TO_SPI_PROM + TEST_OK)
#define TO_HOST_SWITCH_CONSOLE_OK \
        (FROM_HOST_SWITCH_CONSOLE + TEST_OK)
#define TO_HOST_FREESCALE_UCC_LPBK_TEST_OK \
        (FROM_HOST_FREESCALE_UCC_LPBK_TEST + TEST_OK)
#define TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_OK \
        (FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST + TEST_OK)
#define TO_HOST_WRITE_MAC_ADDR_OK \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W1_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W1 + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W0_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W0 + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W1_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W1 + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W0_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W0 + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R1_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R1 + TEST_OK)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R0_OK \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R0 + TEST_OK)
#define TO_HOST_FPGA_INTR_TEST_OK \
        (FROM_HOST_FPGA_INTR_TEST + TEST_OK)
#define TO_HOST_SUB_T3_IND_LPBK_TEST_OK \
		(FROM_HOST_SUB_T3_IND_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_T3_IND_EX_LPBK_TEST_OK \
		(FROM_HOST_SUB_T3_IND_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_E3_IND_LPBK_TEST_OK  \
		(FROM_HOST_SUB_E3_IND_LPBK_TEST + TEST_OK)
#define TO_HOST_SUB_E3_IND_EX_LPBK_TEST_OK  \
		(FROM_HOST_SUB_E3_IND_EX_LPBK_TEST + TEST_OK)
#define TO_HOST_ECC_TEST_OK \
	    (FROM_HOST_ECC_TEST + TEST_OK)
#define TO_HOST_UART_TEST_OK \
	    (FROM_HOST_UART_TEST + TEST_OK)
#define TO_HOST_GE0_LPBK_TEST_OK \
        (FROM_HOST_GE0_LPBK_TEST + TEST_OK)
#define TO_HOST_POWER_NO_MARGIN_OK  \
	    (FROM_HOST_POWER_ALTER_NO_MARGIN + TEST_OK)
#define TO_HOST_POWER_LOW_MARGIN_OK  \
	    (FROM_HOST_POWER_ALTER_LOW_MARGIN + TEST_OK)
#define TO_HOST_POWER_HIGH_MARGIN_OK  \
	    (FROM_HOST_POWER_ALTER_HIGH_MARGIN + TEST_OK)
#define TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_OK		\
        (FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_OK)
#define TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_OK		\
        (FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_OK)
#define TO_HOST_FPGA_READ_INFO_OK \
	    (FROM_HOST_FPGA_READ_INFO + TEST_OK)


#define TO_HOST_CPU_ALIVE_TEST_FAIL \
        (FROM_HOST_CPU_ALIVE_TEST + TEST_FAIL)
#define TO_HOST_READ_FPGA_VERSION_FAIL \
        (FROM_HOST_READ_FPGA_VERSION + TEST_FAIL)
#define TO_HOST_FPGA_DOWNLOAD_TO_FPGA_FAIL \
        (FROM_HOST_FPGA_DOWNLOAD_TO_FPGA + TEST_FAIL)
#define TO_HOST_FPGA_REG_TEST_FAIL \
        (FROM_HOST_FPGA_REG_TEST + TEST_FAIL)
#define TO_HOST_MEMORY_TEST_FAIL \
        (FROM_HOST_MEMORY_TEST + TEST_FAIL)
#define TO_HOST_SPI_PROM_TEST_FAIL \
        (FROM_HOST_SPI_PROM_TEST + TEST_FAIL)
#define TO_HOST_DS3170_REG_TEST_FAIL \
        (FROM_HOST_DS3170_REG_TEST + TEST_FAIL)
#define TO_HOST_E3_AIS_TEST_FAIL \
        (FROM_HOST_E3_AIS_TEST + TEST_FAIL)
#define TO_HOST_CLR_T3_INTR_TEST_FAIL \
        (FROM_HOST_CLR_T3_INTR_TEST + TEST_FAIL)
#define TO_HOST_CLR_T3_BERT_TEST_FAIL \
        (FROM_HOST_CLR_T3_BERT_TEST + TEST_FAIL)
#define TO_HOST_FREESCALE_LPBK_TEST_FAIL \
        (FROM_HOST_FREESCALE_LPBK_TEST + TEST_FAIL)
#define TO_HOST_CLR_T3_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_T3_LPBK_TEST + TEST_FAIL)
#define TO_HOST_CLR_T3_EX_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_T3_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_T3_LPBK_TEST_FAIL \
        (FROM_HOST_SUB_T3_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_T3_EX_LPBK_TEST_FAIL \
        (FROM_HOST_SUB_T3_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_CLR_E3_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_E3_LPBK_TEST + TEST_FAIL)
#define TO_HOST_CLR_E3_EX_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_E3_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_E3_LPBK_TEST_FAIL \
        (FROM_HOST_SUB_E3_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_E3_EX_LPBK_TEST_FAIL \
        (FROM_HOST_SUB_E3_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_FREESCALE_MAC_LPBK_TEST_FAIL \
        (FROM_HOST_FREESCALE_MAC_LPBK_TEST + TEST_FAIL)
#define TO_HOST_FPGA_LPBK_TEST_FAIL \
        (FROM_HOST_FPGA_LPBK_TEST + TEST_FAIL)
#define TO_HOST_FPGA_RESET_FAIL \
        (FROM_HOST_FPGA_RESET + TEST_FAIL)
#define TO_HOST_DS3170_RESET_FAIL \
        (FROM_HOST_DS3170_RESET + TEST_FAIL)
#define TO_HOST_LED_TEST_FAIL \
        (FROM_HOST_LED_TEST + TEST_FAIL)
#define TO_HOST_LED_DISPLAY_FAIL \
        (FROM_HOST_LED_DISPLAY + TEST_FAIL)
#define TO_HOST_FW_DOWNLOAD_TO_DDR_FAIL \
        (FROM_HOST_FW_DOWNLOAD_TO_DDR + TEST_FAIL)
#define TO_HOST_FW_DOWNLOAD_TO_SPI_PROM_FAIL \
        (FROM_HOST_FW_DOWNLOAD_TO_SPI_PROM + TEST_FAIL)
#define TO_HOST_SWITCH_CONSOLE_FAIL \
        (FROM_HOST_SWITCH_CONSOLE + TEST_FAIL)
#define TO_HOST_FREESCALE_UCC_LPBK_TEST_FAIL \
        (FROM_HOST_FREESCALE_UCC_LPBK_TEST + TEST_FAIL)
#define TO_HOST_FPGA_GPIO_FRAMER_GPIO_TEST_FAIL \
        (FROM_HOST_FPGA_GPIO_FRAMER_GPIO_TEST + TEST_FAIL)
#define TO_HOST_WRITE_MAC_ADDR_FAIL \
        (FROM_HOST_WRITE_MAC_ADDR + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W1_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W1 + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO1_W0_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO1_W0 + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W1_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W1 + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO4_W0_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO4_W0 + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R1_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R1 + TEST_FAIL)
#define TO_FROM_HOST_CPU_GPIO_TEST_IO3_R0_FAIL \
	    (FROM_HOST_CPU_GPIO_TEST_IO3_R0 + TEST_FAIL)
#define TO_HOST_FPGA_INTR_TEST_FAIL \
        (FROM_HOST_FPGA_INTR_TEST + TEST_FAIL)
#define TO_HOST_SUB_T3_IND_LPBK_TEST_FAIL \
		(FROM_HOST_SUB_T3_IND_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_T3_IND_EX_LPBK_TEST_FAIL \
		(FROM_HOST_SUB_T3_IND_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_E3_IND_LPBK_TEST_FAIL  \
		(FROM_HOST_SUB_E3_IND_LPBK_TEST + TEST_FAIL)
#define TO_HOST_SUB_E3_IND_EX_LPBK_TEST_FAIL  \
		(FROM_HOST_SUB_E3_IND_EX_LPBK_TEST + TEST_FAIL)
#define TO_HOST_ECC_TEST_FAIL \
	    (FROM_HOST_ECC_TEST + TEST_FAIL)
#define TO_HOST_UART_TEST_FAIL \
	    (FROM_HOST_UART_TEST + TEST_FAIL)
#define TO_HOST_GE0_LPBK_TEST_FAIL \
        (FROM_HOST_GE0_LPBK_TEST + TEST_FAIL)
#define TO_HOST_POWER_NO_MARGIN_FAIL  \
	    (FROM_HOST_POWER_ALTER_NO_MARGIN + TEST_FAIL)
#define TO_HOST_POWER_LOW_MARGIN_FAIL  \
	    (FROM_HOST_POWER_ALTER_LOW_MARGIN + TEST_FAIL)
#define TO_HOST_POWER_HIGH_MARGIN_FAIL  \
	    (FROM_HOST_POWER_ALTER_HIGH_MARGIN + TEST_FAIL)
#define TO_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL \
        (FROM_HOST_UPGRADE_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_FAIL)
#define TO_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM_FAIL \
        (FROM_HOST_GOLDEN_FPGA_DOWNLOAD_TO_SPI_PROM + TEST_FAIL)
#define TO_HOST_FPGA_READ_INFO_FAIL \
	    (FROM_HOST_FPGA_READ_INFO + TEST_FAIL)


/* ioctl commands */

#define IOCTL_GET_FPGA_INTR               0
#define IOCTL_CLEAR_FPGA_INTR             1
#define IOCTL_GET_FRAMER_INTR			  2
#define IOCTL_CLEAR_FRAMER_INTR           3
#define TO_HOST_REINIT_TX_RX_FAIL            0xFE

#define  CMD_NOT_FOUND     0x11       /* command not found */

#define PASSED              0
#define FAILED              1

#define FALSE               0
#define TRUE                1

#define BIT0      0x01
#define BIT1      0x02
#define BIT2      0x04
#define BIT3      0x08
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80

#define POLL_MODE           0
#define INTR_MODE           1

#define EQUAL      1
#define NOT_EQUAL  0

#define PATRIOT_CMD        0
#define PATRIOT_DATA       1
#define PATRIOT_ACK        2
#define PATRIOT_RESULT     4
#define PATRIOT_MENU       5

#define PATRIOT_CPU_PASS          0
#define PATRIOT_CPU_LPBK          1

#define PATRIOT_TDM_PASS          2
#define PATRIOT_TDM_LPBK          3

#define PATRIOT_UCC_PASS          2
#define PATRIOT_UCC_LPBK          3

#define MAC_ADDR_SIZE             6

#define BW_32BITS    0x04

#define TWO_MEG          0x00200000

#ifdef  RAMMON
#define PHYSICAL_ADDR_START     0x00000000
#else
#define PHYSICAL_ADDR_START     0x00000000
#endif

#ifdef VIR_IS_PHY
#define DRAM_VIR_TO_PHY(x)	(x)
#define DRAM_PHY_TO_VIR(x)	(x)
#else
#define DRAM_VIR_TO_PHY(x) 	((ulong)(x) & (~PHYSICAL_ADDR_START))
#define DRAM_PHY_TO_VIR(x) 	((ulong)(x) | PHYSICAL_ADDR_START)
#endif /* #ifdef VIR_IS_PHY */

#define	PHY_ADDR(x)		DRAM_VIR_TO_PHY(x)
#define VIR_ADDR(x)		DRAM_PHY_TO_VIR(x)

#define ADRSPC_RAM		PHYSICAL_ADDR_START     /* start of RAM */

extern long int ADRSPC_PQUICC_IMEMB;
extern long int VIR_ADRSPC_RAM;

#define INTERNAL_IRQ_BASE       16
#define ETSEC2_GROUP0_TX_INTR   19
#define ETSEC2_GROUP0_RX_INTR   20
#define ETSEC2_GROUP0_ERR_INTR  24 
#define QE_LOW_INTR             44

/* From Bills - Sunridges */
/* PCIe side mapped to CPU CCSRBAR Address */
#define PCIE_MAP_CCSRBAR_ADDR   0x70000000

/*
 * TSEC - TBI Physical Address Register (TBIPA)
 */
#define TSEC1_TBIPA_ADDR	0x11
#define TSEC2_TBIPA_ADDR	0x12
#define TSEC3_TBIPA_ADDR	0x13

/* TDMx Index */
#define TDMA                    0
#define TDMB                    1
#define TDMC                    2
#define TDMD                    3

/* Define offsets in MURAM for TX and RX buffer descriptors. */
#define RX_BD_OFFSET 0x0000 /* from AN4026 */
#define TX_BD_OFFSET 0x0100 /* from AN4026 */

#define TX_RX_STATE        0x10000000

#define C_MASK             0x0000F0B8
#define C_PRES             0x0000FFFF

#define CLEAR_HDLC         0x0000
#define CLR_VALUE          0xFFFFFFFF

#define SPIN_10                 10
#define SPIN_100                100
#define SPIN_200                200
#define SPIN_1000               1000
#define SPIN_10000              10000

/* Define offsets in MURAM for RX and TX internal data pointers */
#define RIPTR          0x1000 /* from AN4026 */
#define TIPTR          0x1a00 /* from AN4026 */

#define RFTHR             1
#define RFCNT             1

/* Define offsets in MURAM for VFIFOs. */
#define RX_VFIFO 0x001200
#define TX_VFIFO 0x002000
#define VFIFO_SIZE 0x800 /* HDLC up to 10Mbps 128 bytes */

#define GUMR_FAST_DIAG(y)        (((y) & 3) << 30)
#define GUMR_FAST_TCI         0x20000000
#define GUMR_FAST_TRX         0x10000000
#define GUMR_FAST_TTX         0x08000000
#define GUMR_FAST_CDP         0x04000000
#define GUMR_FAST_CTSP        0x02000000
#define GUMR_FAST_CDS         0x01000000
#define GUMR_FAST_CTSS        0x00800000
#define GUMR_FAST_FFTH(y)        (((y) & 3) << 21)
#define GUMR_FAST_TXSY        0x00020000
#define GUMR_FAST_RSYN        0x00010000
#define GUMR_FAST_SYNL(y)        (((y) & 3) << 14)
#define GUMR_FAST_RTSM        0x00002000
#define GUMR_FAST_RENC(y)        (((y) & 3) << 11)
#define GUMR_FAST_REVD        0x00000400
#define GUMR_FAST_TENC(y)        (((y) & 3) << 8)
#define GUMR_FAST_TCRC(y)        (((y) & 3) << 6)
#define GUMR_FAST_ENR         0x00000020
#define GUMR_FAST_ENT         0x00000010
#define GUMR_FAST_MODE(y)        (y & 0xF)


#define UPSMR_FAST_MFF        0x04000000
#define UPSMR_FAST_NBO_NIBBLE 0x00008000
#define UDSR_DEFAULT          0x7E7E
#define DDR_DIV(y)            (((y) & 3) << 14)                
#define DDR_MCK0_DIS          0x00000020
#define DDR_MCK1_DIS          0x00000010
#define DDR_MCK2_DIS          0x00000008
#define DDR_MCK3_DIS          0x00000004

#define DEVDISR_LBC_DIS       0x08000000
#define DEVDISR_USB_DIS       0x00800000
#define DEVDISR_ESDHC_DIS     0x00200000
#define DEVDISR_CORE1_DIS     0x00020000
#define DEVDISR_ETSEC1_DIS    0x00000080


#define PRINT_INTERVAL          0x40000

#define CPU_I2C0          0    /* P1021 PPC I2C 0 */
#define CPU_I2C1          1    /* P1021 PPC I2C 1 */
#define I2C_BUS_INVALID   2    /* P1021 PPC I2C 1 */

/* I2C Device address defines */
/*      CPU I2C Controller 0 */
#define MB_I2C_ADDR_CTRL        0x00    /* I2C Controller Slave Address */
#define MB_I2C_ADDR_FPGA        0x59    /* FPGA */
#define MB_I2C_ADDR_FPGA1       0x5D
#define MB_I2C_ADDR_FPGA2       0x5E
#define MB_I2C_ADDR_FPGA3       0x5F
#define MB_I2C_ADDR_SPROM       0xA0    /* SPROM */

#define PQ_I2C_FDR_400M_100K    0x2F    /* CCB 400MHz   to I2C 97KHz */

/* Platform specific Freescale I2C defines */
#define PQ_I2CDFSRR             0xC     /* Digital Filter Sampling Rate */

#define N2G_I2C_BIT_DELAY       11      /* 100 KHz - 10 microseconds with one
                                         * extra microsecond for granularity */
#define I2C_DATA_BYTE_XMIT_TIME (9 * N2G_I2C_BIT_DELAY)  /* time to xmit byte.
                                                         * ACK/NACK included */

#define I2C_ACK         0x01    /* ACK expected */
#define I2C_NACK        0x02    /* NACK expected */


/* For 100KHz, bit time is 10 microseconds. Status of ACK/NACK should be
 * settled within the clock pulse. Since wastetime granularity is microsecond,
 * we will poll every 2 microseconds, for 5 times.
 */
#define I2C_STAT_WAIT_TIME      2       /* Status read polling time */
#define I2C_POLL_STAT_TIMEOUT   100     /* Status polling counter */
#define I2C_POLL_DATA_TIMEOUT   140     /* Data polling counter */
#define I2C_POST_STOP_DELAY     100     /* Delay needed after STOP bit*/

#define E_I2C_TIMEDOUT         (0x08)   /* Transmit timedout */
#define E_I2C_INV_ACK          (0x0A)   /* Unexpected ACK or NACK */

#define I2C_SLAVE_ADDR_SHIFT         1  /* slave address shift count */

#define SPI_PROM_READ_CMD_LEN        4
#define SPI_PROM_FAST_READ_CMD_LEN   5
#define SPI_PROM_WRITE_CMD_LEN       5
#define SPI_PROM_ERASE_CMD_LEN       4
#define SPI_PROM_WRITE            0x02
#define SPI_PROM_READ             0x03
#define SPI_PROM_RDSR             0x05
#define SPI_PROM_READ_ID          0x9F
#define SPI_PROM_WREN             0x06
#define SPI_PROM_WEL              0x02
#define SPI_PROM_ERASE_4K         0x20
#define SPI_PROM_ERASE_8K         0x40
#define SPI_PROM_ERASE_64K        0xD8
#define SPI_PROM_OTPP             0x42
#define SPI_PROM_OTPR             0x4B
#define SPI_PROM_WIP              0x01
#define SPI_PROM_WRITE_HEADER        4

#define ERASE_4K_BLOCK               1
#define ERASE_8K_BLOCK               2
#define ERASE_64K_BLOCK              3

#define LAST_SECTOR_ADDR           0xFF0000
#define GOLDEN_FPGA_SECTOR_ADDR    0x0
#define UPGRADE_FPGA_SECTOR_ADDR   0x100000
#define FPGA_HEADER_DATA_SECTOR    0x1F0000
#define FPGA_HEADER_DATA_ADDR      0x1FFFC0
#define FPGA_HEADER_SIZE           16
#define FPGA_GOLDEN_HMAC_HEADER_ADDR    0x0E0000
#define FPGA_UPGRADE_HMAC_HEADER_ADDR   0x1E0000
#define SECTOR_SIZE                0x10000 /* 64K */
#define BLOCK_64K_SIZE             0x10000 /* 64K */
#define BLOCK_32K_SIZE             0x8000  /* 32K */
#define SPI_PROM_PAGE_SIZE         0x100 /* 256 bytes */
#define SPI_PROM_TEST_SIZE         0x500 /* Test 5 pages */
#define SPI_PROM_SIZE              0x1000000 /* 16MB */
#define FPGA_SPI_PROM_SIZE         0x200000  /* 2M */
#define MEM_SIZE_256_MB            0x10000000 /* 256MB */
#define BLOCK_256                  256

#define MAX_PCIE_PORT_NUM        1       /* PCIE ports 0,1 */

#define P1021_PIC_INT_INTR_NO    64      /* P1021 has 64 internal interrupts */

typedef unsigned int        boolean;
typedef unsigned char       uchar;
typedef unsigned short      uint16;

/* CCSR window */
extern long int ADRSPC_PQUICC_IMEMB;
#define ADRSPC_PQUICC_REGB	ADRSPC_PQUICC_IMEMB

struct testmem {
    unsigned int *start, *rdaddr;
    int incr;
    long length;
    long passcount;
    unsigned char flag;
};

struct testdat {
    long rd_pat;
    long wr_pat;
    long flag;
};

uchar g_ds3170_int_flag;

typedef struct patriot_i2c_iface {
    uint32_t offset;    /* I2C device register or memory offset */
    char  *buf;         /* Read/write buffer pointer */
    uint8_t i2c_bus_no; /* I2C bus number */
    uint8_t i2c_dev;    /* MB_I2C_DEVICE device ID */
    uint16_t size;      /* Number of read/write bytes */
    uint8_t i2c_speed;  /* I2C bus speed used during init */
    uint32_t err_no;    /* error type */
} patriot_i2c_if_t;

typedef struct patriot_msg_
{
    int msg_type;
    char data[1000];
} patriot_msg_t;


/* FPGA Register  offset 0x00 ~ 0x0E */
/* LED Control */
#define LED_CTL_REG                0x00
#define CD_LED                     0x01  /* 0 = on , 1 = off */
#define FAR_END_LED                0x02  /* 0 = on , 1 = off */
#define AIS_LED                    0x04  /* 0 = on , 1 = off */
#define LPBK_LED                   0x08  /* 0 = on , 1 = off */
#define ALARM_LED                  0x10  /* 0 = on , 1 = off */
#define POWER_MARGIN_LOW           0x40  /* 0 = No Margin, 1 = Margin low */
#define POWER_MARGIN_HIGH          0x80  /* 0 = No Margin, 1 = Margin high */

/* PORT TYPE SELECT */
#define PORT_TYPE_SEL_REG          0x01
#define PORT_SEL_TYPE_T3           0x00
#define PORT_SEL_TYPE_E3           0x01
#define PORT_OSC_EN                0x02  /* 0 = Tristate/disable , 1 = enable */
#define PORT_CLK_SLAVE             0x04  /* 0 = Master, 1 = Loop time */
#define PORT_TX_SYNC_EN            0x08  /* 0 = Tristate/disable , 1 = enable */
#define PORT_PYLD_LP_EN            0x10  /* 0 = Disable , 1 = Enable */
#define PORT_FPGA_LPBK	           0x20  /* 0 = Disable , 1 = Enable */
#define PORT_SUBRATE_BYPASS        0x40  /* 0 = FPGA subrate used
                                            1 = FPGA subrate bypass */

/* FRAMER GPIO */
#define GPIO_FRAMER_REG            0x02
#define GPIO_ALARM_OUTPUT_0        0x01
#define GPIO_8KREFO                0x02
#define GPIO_0                     0x04
#define GPIO_8KREFI                0x08
#define GPIO_ALARM_OUTPUT_1        0x10
#define GPIO_TMEI_INPUT            0x20
#define GPIO_1                     0x40
#define GPIO_PMU_INPUT             0x80

/* FRAMER GPIO OE */
#define GPIO_FRAMER_OE_REG         0x03
#define GPIO_OUTPUT_0_EN           0x01
#define GPIO_OUTPUT_1_EN           0x02
#define GPIO_OUTPUT_2_EN           0x04
#define GPIO_OUTPUT_3_EN           0x08
#define GPIO_OUTPUT_4_EN           0x10
#define GPIO_OUTPUT_5_EN           0x20
#define GPIO_OUTPUT_6_EN           0x40
#define GPIO_OUTPUT_7_EN           0x80

/* TE3 STATUS */
#define TE3_STATUS_REG             0x04

/* TE3 LINE CONFIG */
#define TE3_LINE_CONF_REG          0x05

/* T3 SUBRATE MODE SELECTION */
#define T3_SUBRATE_MODE_SEL_REG    0x06
#define T3_SUBRATE_VENDOR_SELECT   0x07 /* 000 - No Subrate, Clear T3 mode */
                                        /* 001 - Digital link */
                                        /* 010 - Kentrox */
                                        /* 011 - Larscom */
                                        /* 100 - Adtran */
                                        /* 101 - Verilink */
                                        /* 110 - Reserved */
                                        /* 111 - Reserved */
#define HI_MAP                     0x08 /* 0 - Time slots start with 1st bit */
                                        /* 1 - Time slots start with last bit */
#define RX_ONES                    0x10 /* 0 - Normal Operation */
                                        /* 1 - Send all 1's to CPU */
#define T3_SCRAMBLE_EN             0x20 /* 0 - Disable , 1 - Enable */
#define T3_SUBRATE_BLOCK_RST       0x80 /* 0 - Reset , 1 - Out of reset */

/* T3 SUBRATE BANDWIDTH SELECTION 1 */
#define T3_SUBRATE_BW_SEL_REG_1    0x07
#define T3_SUBRATE_BW_BIT_0        0x01
#define T3_SUBRATE_BW_BIT_1        0x02
#define T3_SUBRATE_BW_BIT_2        0x04
#define T3_SUBRATE_BW_BIT_3        0x08
#define T3_SUBRATE_BW_BIT_4        0x10
#define T3_SUBRATE_BW_BIT_5        0x20
#define T3_SUBRATE_BW_BIT_6        0x40
#define T3_SUBRATE_BW_BIT_7        0x80

/* T3 SUBRATE BANDWIDTH SELECTION 2 */
#define T3_SUBRATE_BW_SEL_REG_2    0x08
#define T3_SUBRATE_BW_BIT_8        0x01
#define T3_SUBRATE_BW_BIT_9        0x02
#define T3_SUBRATE_BW_BIT_10       0x04
#define T3_SUBRATE_BW_BIT_11       0x08
#define T3_SUBRATE_BW_BIT_12       0x10
#define T3_SUBRATE_BW_BIT_13       0x20
#define T3_SUBRATE_BW_BIT_14       0x40
#define T3_SUBRATE_BW_BIT_15       0x80

/* T3 SUBRATE BANDWIDTH SELECTION 3 */
#define T3_SUBRATE_BW_SEL_REG_3    0x09
#define T3_SUBRATE_BW_BIT_16       0x01
#define T3_SUBRATE_BW_BIT_17       0x02
#define T3_SUBRATE_BW_BIT_18       0x04
#define T3_SUBRATE_BW_BIT_19       0x08
#define T3_SUBRATE_BW_BIT_20       0x10
#define T3_SUBRATE_BW_BIT_21       0x20
#define T3_SUBRATE_BW_BIT_22       0x40

/* E3 SUBRATE MODE SELECTION */
#define E3_SUBRATE_MODE_SEL_REG    0x0A
#define E3_SUBRATE_VENDOR_SELECT   0x03 /* 00 - No Subrate, Clear E3 mode */
                                        /* 01 - Digital link */
                                        /* 10 - Kentrox */
                                        /* 11 - Unframed  */
#define E3_SCRAMBLE_EN             0x04 /* 0 - Disable, 1 - Enable */
#define E3_LOCAL_LPBK_EN           0x10 /* 0 - Disable, 1 - Enable */
#define E3_SUBRATE_BLOCK_RST       0x80 /* 0 - Reset , 1 - Out of reset */

/* E3 SUBRATE BANDWIDTH SELECTION 1 */
#define E3_SUBRATE_BW_SEL_REG_1    0x0B
#define E3_SUBRATE_BW_BIT_0        0x01
#define E3_SUBRATE_BW_BIT_1        0x02
#define E3_SUBRATE_BW_BIT_2        0x04
#define E3_SUBRATE_BW_BIT_3        0x08
#define E3_SUBRATE_BW_BIT_4        0x10
#define E3_SUBRATE_BW_BIT_5        0x20
#define E3_SUBRATE_BW_BIT_6        0x40
#define E3_SUBRATE_BW_BIT_7        0x80

/* E3 SUBRATE BANDWIDTH SELECTION 2 */
#define E3_SUBRATE_BW_SEL_REG_2    0x0C
#define E3_SUBRATE_BW_BIT_8        0x01
#define E3_SUBRATE_BW_BIT_9        0x02
#define E3_SUBRATE_BW_BIT_10       0x04
#define E3_SUBRATE_BW_BIT_11       0x08
#define E3_SUBRATE_BW_BIT_12       0x10
#define E3_SUBRATE_BW_BIT_13       0x20
#define E3_SUBRATE_BW_BIT_14       0x40
#define E3_SUBRATE_BW_BIT_15       0x80

/* E3 SUBRATE BANDWIDTH SELECTION 3 */
#define E3_SUBRATE_BW_SEL_REG_3    0x0D
#define E3_SUBRATE_BW_BIT_16       0x01
#define E3_SUBRATE_BW_BIT_17       0x02
#define E3_SUBRATE_BW_BIT_18       0x04
#define E3_SUBRATE_BW_BIT_19       0x08
#define E3_SUBRATE_BW_BIT_20       0x10
#define E3_SUBRATE_BW_BIT_21       0x20
#define E3_SUBRATE_BW_BIT_22       0x40


/* TDM FPGA REVISION */
#define TDM_FPGA_REV_REG           0x0E
#define FPGA_REV                   0x0F
#define FPGA_SIG                   0xF0

/* RECONFIGURATION CONTROL */
#define RECONFIG_CONTROL_REG       0x20
#define UPGRADE_IMAGE_HEADER_READ  0x01
#define RECONFIG_FSM_RESET         0x02

#define FPGA_GPIO                  0x00
#define FRAMER_GPIO				   0x01

#define ENABLE	   1
#define DISABLE	   0

#define DS3170_INT_ENABLE      ENABLE
#define DS3170_INT_DISABLE     DISABLE
/*
 * T3/E3 Subrate Configuration Reg 0x30100006 and 0x3010000A
 */
#define CLEAR               0x00
#define DIGITAL_LINK        0x01
#define KENTROX             0x02
#define LARSCOM             0x03
#define ADTRAN              0x04
#define VERILINK            0x05
#define UNFRM_E3            0x03
#define SUBRATE_MASK        0x07
#define DSU_RESET           0x80

#define VERILINK_HI_MAP     0x08
#define RX_ONES             0x10
#define SCRAMBLE_EN         0x20

enum {
    KENTROX_1K, KENTROX_1_5K, KENTROX_2K, KENTROX_2_5K,
    KENTROX_3K, KENTROX_3_5K, KENTROX_4K, KENTROX_4_5K,
    KENTROX_5K, KENTROX_5_5K, KENTROX_6K, KENTROX_6_5K,
    KENTROX_7K, KENTROX_7_5K, KENTROX_8K, KENTROX_8_5K,
    KENTROX_9K, KENTROX_9_5K, KENTROX_10K, KENTROX_10_5K,
    KENTROX_11K, KENTROX_11_5K, KENTROX_12K, KENTROX_12_5K,
    KENTROX_13K, KENTROX_13_5K, KENTROX_14K, KENTROX_14_5K,
    KENTROX_15K, KENTROX_15_5K, KENTROX_16K, KENTROX_16_5K,
    KENTROX_17K, KENTROX_17_5K, KENTROX_18K, KENTROX_18_5K,
    KENTROX_19K, KENTROX_19_5K, KENTROX_20K, KENTROX_20_5K,
    KENTROX_21K, KENTROX_21_5K, KENTROX_22K, KENTROX_22_5K,
    KENTROX_23K, KENTROX_23_5K, KENTROX_24K, KENTROX_24_5K,
    KENTROX_25K, KENTROX_25_5K, KENTROX_26K, KENTROX_26_5K,
    KENTROX_27K, KENTROX_27_5K, KENTROX_28K, KENTROX_28_5K,
    KENTROX_29K, KENTROX_29_5K, KENTROX_30K, KENTROX_30_5K,
    KENTROX_31K, KENTROX_31_5K, KENTROX_32K, KENTROX_32_5K,
    KENTROX_33K, KENTROX_33_5K, KENTROX_34K, KENTROX_34_5K,
    KENTROX_35K,
};
#define DIG_LINK_MAX_TS     147    /* 147 timeslots, 0..146 */
#define LARSCOM_MAX_TS      14     /* 14 timeslots, 0..13  */
#define ADTRAN_MAX_TS       588    /* 588 timeslots, 0..587  */
#define VERILINK_MAX_TS     28     /* 28 timeslots, 0..27  */


/* ESPI */
#define MAX_SPI_SPIN    1000

/* Ported from common.h */
enum {
    MODE_T1 = 0,
    MODE_E1,
    MODE_CHAN_T1,
    MODE_CHAN_E1,
    MODE_8M,
    MODE_8M_INT,         /* 8M Interleave */
    MODE_8M_INT_T1,
    MODE_UNF_T1,
    MODE_UNF_E1,
    MODE_T3,
    MODE_E3,
    MODE_CHAN_T3,
    MODE_CHAN_E3,
};

#define EXT_LPBK  0   /* External Loopback */
#define INT_LPBK  1   /* Internal Loopback */
#define FPGA_LPBK 2   /* FPGA Loopback */
#define BYPASS_SUB 3  /* Bypass subrate */
#define USE_SUB    4  /* Use subrate */
#define NOTUSED    5  /* NOTUSED */

int etsec_recv_nframes[3];
int etsec_tx_nframes[3];

int hdlc_rx_frames;

extern int patriot_read_fpga_version(void);
extern int patriot_fpga_download_to_fpga(void);
extern int patriot_fpga_reg_test(void);
extern int patriot_cpu_alive_test(void);
extern int patriot_memory_test(void);
extern int patriot_spi_prom_test(void);
extern int patriot_ds3170_reg_test(void);
extern int patriot_clear_e3_ais_test(void);
extern int patriot_clear_t3_intr_test(void);
extern int patriot_clear_t3_bert_test(void);
extern int patriot_fs_lpbk_test(void);
extern int patriot_fs_tdm_lpbk_test(void);
extern int patriot_fs_ucc_lpbk_test(void);
extern int patriot_clear_t3_int_lpbk_test(void);
extern int patriot_fpga_lpbk_test(void);
extern int patriot_clear_t3_ext_lpbk_test(void);
extern int patriot_subrate_t3_int_lpbk_test(void);
extern int patriot_subrate_t3_ext_lpbk_test(void);
extern int patriot_subrate_t3_individual_int_lpbk_test(void);
extern int patriot_subrate_t3_individual_ext_lpbk_test(void);
extern int patriot_clear_e3_int_lpbk_test(void);
extern int patriot_clear_e3_ext_lpbk_test(void);
extern int patriot_subrate_e3_int_lpbk_test(void);
extern int patriot_subrate_e3_ext_lpbk_test(void);
extern int patriot_subrate_e3_individual_int_lpbk_test(void);
extern int patriot_subrate_e3_individual_ext_lpbk_test(void);
extern int patriot_clear_te3_test(uchar, uchar);
extern int patriot_clear_te3_subrate_test(uchar, uchar, ulong, uchar);
extern int patriot_ds3170_tdm_lpbk_test(void);
extern int patriot_test_fpga_gpio_framer(void);
extern int patriot_host_to_module_gpio1_wr1_test(void);
extern int patriot_host_to_module_gpio1_wr0_test(void);
extern int patriot_host_to_module_gpio4_wr1_test(void);
extern int patriot_host_to_module_gpio4_wr0_test(void);
extern int patriot_module_to_host_gpio3_rd1_test(void);
extern int patriot_module_to_host_gpio3_rd0_test(void);
extern void display_por_registers(void);
extern void display_laccs_registers(void);
extern void display_law_registers(void);
extern void display_ddr1_registers(void);
extern void display_ddr2_registers(void);
extern void display_i2c1_registers(void);
extern void display_i2c2_registers(void);
extern void display_lbus_registers(void);
extern void display_pic_registers(void);
extern void display_pcie_registers(int);
extern void display_gpio_registers(void);
extern void display_l2cache_registers(void);

extern void etsec2_intr_hndlr (void);
extern void hdlc_intr_hndlr (void);
extern int patriot_fw_download_to_ddr(void);
extern int patriot_fw_download_to_spi_prom(void);
extern int patriot_fpga_reg_test(void);
extern int patriot_switch_console(void);
extern int module_mem_test(void);
extern void display_mem(void);
extern void modify_mem(void);

/* Ethernet packet contents
 *  mac_addr_t  dest_addr;
 *  mac_addr_t  src_addr;
 *  ushort      pkt_len;
 *  uchar       data[1500]; Command:
 *                          ==> data[0] : PATRIOT_CMD (host)
 *                                        PATRIOT_ACK (module)
 *                                        PATRIOT_RESULT (module)
 *                              data[1] : command (host)
 *                                        command + TEST_ACK (module)
 *                                        ret_val from test (module)
 *                              data[2] : 1st byte (MSB) of param (host)
 *                              data[3] : 2nd byte of param (host)        
 *                              data[4] : 3rd byte of param (host)
 *                              data[5] : 4th byte (LSB) of param (host)
 *
 *                          Data: Same for host and module sides
 *                          ==> data[0] : PATRIOT_DATA
 *                              data[1] : Loppback option
 *                              data[2] : byte 1 [MSB] length of data packet
 *                              data[3] : byte 2 [LSB] length of data packet
 */
 
 
extern int patriot_fpga_register_read(void);
extern int patriot_fpga_register_write(void);
extern int patriot_display_led(void);
extern int patriot_clear_t3_kentrox_lpbk(uchar);
extern int patriot_clear_e3_kentrox_lpbk(uchar);
extern void platform_cpu_i2c_init(void);
extern int etsec_get_info_ptr (int tsec_num);
extern void platform_microcode_download (void);
extern int patriot_fpga_reset(void);
extern int patriot_ds3170_reset(void);
extern int patriot_mac_lpbk_test(void);
extern int patriot_write_mac_addr(void);
extern void patriot_init_eth_intr_count(void);
extern void patriot_get_eth_intr_count(void);
extern int patriot_fpga_intr_test(void);
extern int patriot_clear_t3_dig_link_lpbk(uchar);
extern int patriot_clear_e3_dig_link_lpbk(uchar);
extern int patriot_clear_t3_verilink_lpbk(uchar);
extern int patriot_clear_t3_larscom_lpbk(uchar);
extern int patriot_clear_t3_adtran_lpbk(uchar);
extern int patriot_ddr_ecc_single_bit_err_test (void);
extern int patriot_uart_test (void);
extern int patriot_ge0_loopback_test(void);
extern int patriot_power_no_margin(void);
extern int patriot_power_margin_low(void);
extern int patriot_power_margin_high(void);
extern int patriot_upgrade_fpga_download_to_spi_prom(void);
extern int patriot_golden_fpga_download_to_spi_prom(void);
extern int spi_prom_write_multi_bytes (uint addr_offset, uchar *data, int cs,
					int size);
extern void modify_cpu_regs(void); 
extern int spi_prom_read_multi_bytes (uint addr_offset, uchar *data, int cs,
				      int size);
extern int patriot_dump_fpga_info_to_host(void);
/*------------------------------------------------------------------------------
 * $Log: patriot_main.h,v $
 * Revision 1.1  2014/03/25 02:12:34  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.8  2012/10/25 07:24:10  steja
 * Remove "Clear" from the subrate test
 *
 * Revision 1.7  2012/10/16 07:42:40  steja
 * Improve the GPIO test
 *
 * Revision 1.6  2012/09/14 23:41:56  huanngo
 * Adding the utility to display FPGA secure boot registers and multiboot info table
 *
 * Revision 1.5  2012/07/18 23:51:53  huanngo
 * Adding functions to support programming FPGA to SPI PROM
 *
 * Revision 1.4  2012/06/11 07:43:28  steja
 * Add Power Margin Utilities
 *
 * Revision 1.3  2012/06/08 23:35:39  huanngo
 * Adding constant definitions for ECC memory,UART and GE 0 loopback tests
 *
 * Revision 1.2  2012/05/08 23:52:55  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.15  2012/03/27 07:50:00  steja
 * Fix Warning compilation
 *
 * Revision 1.1.4.14  2012/03/16 12:05:52  steja
 * Update the code to support Subrate individual test loopback
 *
 * Revision 1.1.4.13  2012/03/13 13:38:21  steja
 * Support Framer Interrupt
 *
 * Revision 1.1.4.12  2012/02/28 02:22:26  huanngo
 * Adding some bit definitions
 *
 * Revision 1.1.4.11  2012/01/09 23:06:18  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.4.10  2011/12/22 12:11:51  steja
 * Fix the GPIO test for Rev 1B board.
 *
 * Revision 1.1.4.9  2011/12/21 23:46:32  huanngo
 * Adding tests for FPGA interrupt and fix bug in FPGA i2c intermittent access failure
 *
 * Revision 1.1.4.8  2011/12/08 15:07:11  steja
 * Update IO Test function
 *
 * Revision 1.1.4.7  2011/12/01 18:51:05  huanngo
 * Support new command to write MAC address to EEPROM and fix bugs
 *
 * Revision 1.1.4.6  2011/11/24 09:33:34  steja
 * Update Patriot code
 *
 * Revision 1.1.4.5  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.4  2011/11/23 12:24:24  steja
 * Update the GPIO testing
 *
 * Revision 1.1.4.3  2011/10/07 01:11:45  huanngo
 * Update code to support HDLC, SPI EEPROM and FPGA
 *
 * Revision 1.1.4.2  2011/08/18 19:43:25  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.28  2011/08/06 00:17:40  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.27  2011/07/19 06:11:35  huanngo
 * Update code per code review comments
 *
 * Revision 1.1.2.26  2011/07/14 14:38:27  steja
 * Update Patriot Project Module side code
 *
 * Revision 1.1.2.25  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.24  2011/07/08 00:08:48  huanngo
 * Clean up code
 *
 * Revision 1.1.2.23  2011/07/07 16:21:54  steja
 * 1. Clean up code
 * 2. Add check statur register after loopback test for DS3170.
 *
 * Revision 1.1.2.22  2011/07/05 09:57:36  steja
 * Update Loopback pass for DS3170 code
 *
 * Revision 1.1.2.21  2011/07/04 09:54:35  steja
 * Update DS3170 code :
 * 1. Add {FROM_HOST_CLR_T3_EX_LPBK_TEST}
 * 2. FROM_HOST_CLR_T3_EX_LPBK_TEST
 *     FROM_HOST_CLR_T3_SUB_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_EX_LPBK_TEST
 *     FROM_HOST_CLR_E3_SUB_EX_LPBK_TEST
 * 3. Update return (TO_HOST_CLR_E3_EX_LPBK_TEST_FAIL) and
 *     return(TO_HOST_CLR_E3_EX_LPBK_TEST_OK)
 *
 * Revision 1.1.2.20  2011/07/03 19:04:14  huanngo
 * Remove subrate individual loopbacks from host
 *
 * Revision 1.1.2.19  2011/07/01 22:13:02  huanngo
 * Clean up and update code for Patriot
 *
 * Revision 1.1.2.18  2011/07/01 15:39:07  steja
 * 1. Update DS3170 utility test code
 * 2. Update Internal and External loopback test for DS3170
 *
 * Revision 1.1.2.17  2011/06/29 16:24:55  steja
 * Update DS3170 code.
 *
 * Revision 1.1.2.16  2011/06/28 06:27:56  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.15  2011/06/27 14:14:06  steja
 * 1. Update FPGA register test function
 * 2. Add FPGA dump register function
 * 3. Add FPGA register read / write utility function
 * 4. Add FPGA initialization function
 *
 * Revision 1.1.2.14  2011/06/22 02:37:18  steja
 * Update DS3170 code Interrupt Handler function
 *
 * Revision 1.1.2.13  2011/06/17 07:03:54  steja
 * 1. Move Patriot_fpga_test to patriot_main.c
 * 2. Remove fpga loopback test item
 *
 * Revision 1.1.2.12  2011/06/17 02:26:32  huanngo
 * Adding definitions for LED test and display
 *
 * Revision 1.1.2.11  2011/06/14 10:13:42  steja
 * Update DS3170 code and FPGA Register test
 *
 * Revision 1.1.2.10  2011/06/13 12:21:27  steja
 * Add submenu utilites for DS3170 and FPGA
 *
 * Revision 1.1.2.9  2011/06/13 09:55:16  steja
 * Remove no used variables
 *
 * Revision 1.1.2.8  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.7  2011/06/09 07:03:37  steja
 * Update the code for DS3170 and FPGA's Patriot
 *
 * Revision 1.1.2.6  2011/06/09 01:28:10  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.5  2011/05/25 16:05:06  steja
 * Update the DS3170 testing function based on specs
 *
 * Revision 1.1.2.4  2011/05/21 01:01:29  huanngo
 * Support memory test, I2C interface
 *
 * Revision 1.1.2.3  2011/05/09 21:07:23  huanngo
 * Update code for HDLC over TDM loopback
 *
 * Revision 1.1.2.2  2011/05/09 15:38:37  steja
 * Initial Check in Maxim DS3170 Framer
 *
 * Revision 1.1.2.1  2011/05/02 23:33:23  huanngo
 * Update code to support Patriot module side
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
