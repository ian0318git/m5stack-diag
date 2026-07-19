/* $Id: sm_patriot.h,v 1.16 2014/06/12 19:16:47 huanngo Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/sm_patriot.h,v $
 *******************************************************************************
 * File Name: sm_patriot.h
 *
 * Description: Patriot main include file
 *
 *      
 * Author: Huan Ngo
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */
#ifndef __SM_PATRIOT__
#define __SM_PATRIOT__
#include "ngio.h"


#define PATRIOT_CMD        0
#define PATRIOT_DATA       1
#define PATRIOT_ACK        2
#define PATRIOT_RESULT     4

#define PATRIOT_CPU_PASS       0
#define PATRIOT_CPU_LPBK       1

#define PATRIOT_TDM_PASS       2
#define PATRIOT_TDM_LPBK       3

#define PATRIOT_DS3170_TDM_PASS   4
#define PATRIOT_DS3170_TDM_LPBK   5

#define PATRIOT_UCC_PASS          2
#define PATRIOT_UCC_LPBK          3


#define ETHER_PACKET_LEN_MAX        1514
#define ARP_BOOT_RETRY               500

#define P1016_PCI_VENDOR_ID      0x010A1957
#define P1016_PCI_REVISION_ID    0x0B200111

#define BOOTUP_TIME               80

/* PCA9557 commands */
#define INPUT_PORT_REG            0
#define OUTPUT_PORT_REG           1
#define POLARITY_INVERSION_REG    2
#define CONFIGURATION_REG         3

#define BIT0      0x01
#define BIT1      0x02
#define BIT2      0x04
#define BIT3      0x08
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80

#define BLOCK_256                  256
#define RUNTIME_HMAC_SIZE           32
#define ROLLOVER_KEY_SIZE          320
#define SB_CORE_VER_STR_SIZE_SIZE    1
#define SB_CORE_VER_STR_SIZE        31
#define MICRO_VER_STR_SIZE_SIZE      1
#define MICRO_VER_STR_SIZE          31
#define FIPS_GUID_PTR_SIZE           4
#define ROLLOVER_KEY_PTR_SIZE        4

#define RUNTIME_HMAC_OFFSET           BLOCK_256 + 0xC0
#define ROLLOVER_KEY_OFFSET           2 * BLOCK_256
#define SB_CORE_VER_STR_SIZE_OFFSET   ROLLOVER_KEY_OFFSET + ROLLOVER_KEY_SIZE + 96
#define SB_CORE_VER_STR_OFFSET        SB_CORE_VER_STR_SIZE_OFFSET + \
                                           SB_CORE_VER_STR_SIZE_SIZE
#define MICRO_VER_STR_SIZE_OFFSET     SB_CORE_VER_STR_OFFSET + \
                                           SB_CORE_VER_STR_SIZE
#define MICRO_VER_STR_OFFSET          MICRO_VER_STR_SIZE_OFFSET + \
                                           MICRO_VER_STR_SIZE_SIZE
#define FIPS_GUID_PTR_OFFSET          MICRO_VER_STR_OFFSET + \
                                           MICRO_VER_STR_SIZE
#define ROLLOVER_KEY_PTR_OFFSET       FIPS_GUID_PTR_OFFSET + \
                                           FIPS_GUID_PTR_SIZE


#define CSSR_DUART_OFFSET    0x4000
#define ULSR_DR                 0x1

#define PATRIOT_IP_ADDR          0x0A010101
#define ETHERNET_PACKET_TYPE         0x0806

/* DUART Registers(0x4000-0x5000) */
typedef struct ccsr_duart {
    char    res1[1280];
    uchar  urbr1_uthr1_udlb1;  /* 0x4500 - URBR1, UTHR1, UDLB1 with the */
                                /*          same address offset of 0x04500 */
    uchar  uier1_udmb1;        /* 0x4501 - UIER1, UDMB1 with the same */
                                /*          address offset of 0x04501 */
    uchar  uiir1_ufcr1_uafr1;  /* 0x4502 - UIIR1, UFCR1, UAFR1 with the */
                                /*          same address offset of 0x04502 */
    uchar  ulcr1;            /* 0x4503 - UART1 Line Control Register */
    uchar  umcr1;            /* 0x4504 - UART1 Modem Control Register */
    uchar  ulsr1;            /* 0x4505 - UART1 Line Status Register */
    uchar  umsr1;            /* 0x4506 - UART1 Modem Status Register */
    uchar  uscr1;            /* 0x4507 - UART1 Scratch Register */
    char    res2[8];
    uchar  udsr1;            /* 0x4510 - UART1 DMA Status Register */
    char    res3[239];
    volatile uchar  urbr2_uthr2_udlb2;/* 0x4600 - URBR2, UTHR2, UDLB2 with the */
                                /*          same address offset of 0x04600 */
    uchar  uier2_udmb2;        /* 0x4601 - UIER2, UDMB2 with the same */
                                /*          address offset of 0x04601 */
    uchar  uiir2_ufcr2_uafr2;  /* 0x4602 - UIIR2, UFCR2, UAFR2 with the */
                                /*          same address offset of 0x04602 */
    uchar  ulcr2;            /* 0x4603 - UART2 Line Control Register */
    uchar  umcr2;            /* 0x4604 - UART2 Modem Control Register */
    uchar  ulsr2;            /* 0x4605 - UART2 Line Status Register */
    uchar  umsr2;            /* 0x4606 - UART2 Modem Status Register */
    uchar  uscr2;            /* 0x4607 - UART2 Scratch Register */
    char    res4[8];
    uchar  udsr2;            /* 0x4610 - UART2 DMA Status Register */
    char    res5[2543];
} ccsr_duart_t;


typedef struct patriot_ds {
    ushort  board_id;
    uchar   slot;
    uchar   uart;
    ulong   host_pci_base_addr;
    ulong   nm_pci_base_addr;
    ulong   patriot_ds_addr;
    uchar   fpga_downloaded[MAX_SM+1];
    uchar   fw_downloaded[MAX_SM+1];
    char   b_name[30];
    int     ge_in_port;
    int     ge_out_port;
    uchar tx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet tx packet buffer */
    uchar rx_pak[ETHER_PACKET_LEN_MAX];     /* Ethernet rx packet buffer */
    ether_hdr_t  eth_hdr;
    struct ngio_intf_t *patriot_sm_iface;
} patriot_ds_t;

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
#define TO_HOST_CLR_E3_SUB_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_E3_SUB_LPBK_TEST + TEST_FAIL)
#define TO_HOST_CLR_E3_SUB_EX_LPBK_TEST_FAIL \
        (FROM_HOST_CLR_E3_SUB_EX_LPBK_TEST + TEST_FAIL)
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


extern ulong get_pci_device_base_offset (uint slot, uint dev_num);
extern ulong get_pci_device_base(uint, uint);
extern int setup_sw_dev(uint, uint, uint);
extern uint get_sgmii_ip_addr (int sgmii_port);


extern int patriot_cpu_fw_download_wrapper(patriot_ds_t *iface);
extern int patriot_fpga_download(patriot_ds_t *iface);
extern int patriot_display_fpga_version(patriot_ds_t *iface);
extern int patriot_fpga_download_to_fpga(patriot_ds_t *iface);
extern int patriot_fpga_regs_display(patriot_ds_t *iface);
extern int patriot_fpga_reg_show(patriot_ds_t *iface);
extern int patriot_fpga_reg_alter(patriot_ds_t *iface);
extern int patriot_diplay_led(patriot_ds_t *iface);
extern int patriot_test_led(patriot_ds_t *iface);
extern int patriot_sm_reset(patriot_ds_t *iface);
extern int patriot_cpu_reset(patriot_ds_t *iface);
extern int patriot_fpga_reset(patriot_ds_t *iface);
extern int patriot_ds3170_reset(patriot_ds_t *iface);
extern int patriot_fpga_reg_test(patriot_ds_t *iface);
extern int patriot_cpu_alive_test(patriot_ds_t *iface);
extern int patriot_memory_test(patriot_ds_t *iface);
extern int patriot_spi_prom_test_wrap(patriot_ds_t *iface);
extern int patriot_spi_prom_test(patriot_ds_t *iface, int spi_num);
extern int patriot_ds3170_reg_test(patriot_ds_t *iface);
extern int patriot_clear_e3_ais_test(patriot_ds_t *iface);
extern int patriot_framer_intr_test(patriot_ds_t *iface);
extern int patriot_clear_t3_bert_test(patriot_ds_t *iface);
extern int patriot_fs_lpbk_test(patriot_ds_t *iface);
extern int patriot_fs_tdm_lpbk_test(patriot_ds_t *iface);
extern int patriot_fs_ucc_lpbk_test(patriot_ds_t *iface);
extern int patriot_clear_t3_lpbk_test(patriot_ds_t *iface);
extern int patriot_subrate_t3_lpbk_test(patriot_ds_t *iface);
extern int patriot_subrate_t3_individual_lpbk_test(patriot_ds_t *iface);
extern int patriot_clear_e3_lpbk_test(patriot_ds_t *iface);
extern int patriot_subrate_e3_lpbk_test(patriot_ds_t *iface);
extern int patriot_subrate_e3_individual_lpbk_test(patriot_ds_t *iface);
extern int patriot_send_cmd(patriot_ds_t *iface, uchar cmd, int param);  
extern int patriot_setup_ge_env(patriot_ds_t *iface);     
extern int patriot_cleanup_ge_env(patriot_ds_t *iface);
extern void patriot_clear_rx_buf(void);    
extern int patriot_send_data_packet (patriot_ds_t *iface, uchar *data_ptr,
				     int blk_size);
extern int patriot_spi_prom_write(patriot_ds_t *iface);
extern int patriot_spi_prom_read(patriot_ds_t *iface);     
extern int patriot_send_test_data_packets (patriot_ds_t *iface, int lpbk_op);
extern int patriot_upgrade_fw(patriot_ds_t *iface);
extern int patriot_switch_console(patriot_ds_t *iface);
extern int patriot_rcv_cmd_result_packet(patriot_ds_t *iface, uchar cmd);
extern int patriot_rcv_cmd_result_packet_for_ge0_lpbk(patriot_ds_t *iface,
						      uchar cmd);
extern int patriot_i2c_port_reg_read(patriot_ds_t *iface);
extern int patriot_i2c_port_reg_write(patriot_ds_t *iface);
extern int patriot_pcie_config_reg_test(patriot_ds_t *iface);
extern int patriot_fpga_lpbk_test(patriot_ds_t *iface);
extern int patriot_fs_mac_lpbk_test(patriot_ds_t *iface);
extern int get_sm_mac_addr(int slot, uchar *);
extern int patriot_write_mac_addr(patriot_ds_t *iface);
extern int patriot_test_fpga_gpio_framer(patriot_ds_t *iface);
extern int patriot_fpga_intr_test(patriot_ds_t *iface);
extern int patriot_host_to_module_gpio_test(patriot_ds_t *iface);
extern int patriot_cpu_fw_download_util(patriot_ds_t *iface);
extern int configure_ltc4215_and_io_port(patriot_ds_t *iface);
extern int patriot_sm_cleanup(patriot_ds_t *iface);
extern int patriot_subrate_t3_ind_lpbk_test(patriot_ds_t *iface);
extern int patriot_subrate_e3_ind_lpbk_test(patriot_ds_t *iface);
extern int patriot_memory_ecc_test(patriot_ds_t *iface);
extern int patriot_uart_test(patriot_ds_t *iface);
extern int patriot_ge0_loopback_test(patriot_ds_t *iface);
extern int patriot_config_power_margin(patriot_ds_t *iface);
extern int patriot_upgrade_fpga_download_spi_prom(patriot_ds_t *iface);
extern int patriot_golden_fpga_download_spi_prom(patriot_ds_t *iface);
extern int patriot_display_fpga_info(patriot_ds_t *iface);
extern int get_mac_addr_from_cookie(uchar *cookie_contents, mac_addr_t *mac_addr);
extern int set_promisc(char *device, int sock);
extern void display_err_msg(void);
#endif
/*------------------------------------------------------------------------------
$Log: sm_patriot.h,v $
Revision 1.16  2014/06/12 19:16:47  huanngo
Fix the GE0 loopback failure on the new switch from Utah

Revision 1.15  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.14  2012/12/03 12:42:24  steja
Add error message utility

Revision 1.13  2012/11/07 02:42:58  steja
Add Subrate individual test

Revision 1.12  2012/10/25 08:49:00  steja
Remove "Clear" from the subrate test

Revision 1.11  2012/10/16 12:35:22  steja
Improve the GPIO test

Revision 1.10  2012/10/15 21:21:56  huanngo
Adding function prototype for set_promisc()

Revision 1.9  2012/09/19 18:34:25  huanngo
Support new utility for secure boot and interface test

Revision 1.8  2012/07/19 17:40:04  huanngo
Support FPGA programming to SPI PROM

Revision 1.7  2012/06/30 00:14:16  huanngo
Increase BOOTUP_TIME to avoid intermittent failure

Revision 1.6  2012/06/27 06:17:03  steja
Add Power Margin Utilities

Revision 1.5  2012/06/07 21:20:07  huanngo
Adding new tests

Revision 1.4  2012/05/02 17:55:16  huanngo
Increase the boot up time due to code signing

Revision 1.3  2012/03/28 23:34:56  huanngo
Support new tests and utilities on Patriot

Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
