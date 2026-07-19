/* $Id: pcmap.h,v 1.2 2013/07/30 10:11:21 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/pcmap.h,v $
 *------------------------------------------------------------------
 *
 * pcmap.h - Memory map defines for Prince.
 *
 * Xiaoying Zhang -- Nov. 2012.
 *
 * Copyright (c) 2012-2013 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PCMAP_H_
#define _PCMAP_H_

#include <linux/ioctl.h>

/*
 * Macro to map DRAM virtual space to physical space and vice versa. 
 */
#define PHYSICAL_ADDR_START      0

/*
 * Physical DRAM definitions.
 */
#define ADRSPC_RAM              PHYSICAL_ADDR_START     /* start of RAM */
#define PHY_ADRSPC_RAM          PHYSICAL_ADDR_START     /* Start of RAM */
#define ADRSPC_RAM_END          (PHYSICAL_ADDR_START + 0x8000000) 
#define ADRSPC_RAM_SIZE         (ADRSPC_RAM_END - ADRSPC_RAM)

/*
 * Memory map.
 */
#define ZYNC_DDR_MEM_BASE                   0x0

#define ZYNC_SYSTEM_CSR_BASE                0x40000000
#define ZYNC_SYSTEM_CSR_LENGTH              0x30000

#define ZYNC_GE_MAC_CSR_BASE                0x40010000
#define ZYNC_GE_MAC_STAT_COUNTER            ZYNC_GE_MAC_CSR_BASE + 0x0200
#define ZYNC_GE_MAC_CFG                     ZYNC_GE_MAC_CSR_BASE + 0x0400
#define ZYNC_GE_MDIO_INTERFACE              ZYNC_GE_MAC_CSR_BASE + 0x0500
#define ZYNC_GE_INTR_CTRL                   ZYNC_GE_MAC_CSR_BASE + 0x0600
#define ZYNC_GE_FRAME_FILTER                ZYNC_GE_MAC_CSR_BASE + 0x0700

#define ZYNC_GE_DMA_CSR_BASE                0x40020000

#define ZYNC_SCC_CSR_BASE                   0x80000000
#define ZYNC_SCC_CSR_LENGTH                 0x10000
#define ZYNC_SCC_CTRL                       ZYNC_SCC_CSR_BASE + 0x1000
#define ZYNC_SCC_RX_DMA_CTRL                ZYNC_SCC_CSR_BASE + 0x2000
#define ZYNC_SCC_TX_DMA_CTRL                ZYNC_SCC_CSR_BASE + 0x2200
#define ZYNC_SCC_BISYNC_TX_CTRL             ZYNC_SCC_CSR_BASE + 0x2400
#define ZYNC_SCC_ASYNC_PPP_TX_CTRL          ZYNC_SCC_CSR_BASE + 0x2600
#define ZYNC_SCC_ASYNC_PPP_RX_CTRL          ZYNC_SCC_CSR_BASE + 0x2800
#define ZYNC_SCC_BISYNC_RX_CTRL             ZYNC_SCC_CSR_BASE + 0x2a00
#define ZYNC_SCC_IFACE_CTRL                 ZYNC_SCC_CSR_BASE + 0x3000

#define ZYNC_UART0_BASE                     0xE0000000
#define ZYNC_I2C0_BASE                      0xE0004000

#define ZYNC_PS_DDRC_BASE                   0xF8006000
#define ZYNC_PS_DDRC_LENGTH                 0x1000

#define ZYNC_PS_CPU_INTR_CTRL_BASE          0xF8F01000
#define ZYNC_PS_CPU_INTR_CTRL_LENGTH        0x1000
#define ZYNC_PS_ICDISER2_OFFSET             0x0108
#define ZYNC_PS_ICDICER2_OFFSET             0x0188
#define ZYNC_PS_SPI_STS1_OFFSET             0x0D08

#define ZYNC_OCM_BASE                       0xfffc0000

/*
 * Marcos for kernel driver
 */
#define DMA_MAGIC                           'm'
#define GET_GE_DMA_RX_BUF_PHYS              _IOR(DMA_MAGIC, 1, unsigned int)
#define GET_GE_DMA_TX_BUF_PHYS              _IOR(DMA_MAGIC, 2, unsigned int)
#define GET_GE_DMA_RXBD_PHYS                _IOR(DMA_MAGIC, 3, unsigned int)
#define GET_GE_DMA_TXBD_PHYS                _IOR(DMA_MAGIC, 4, unsigned int)
#define GET_SCC_DMA_RX_BUF_PHYS             _IOR(DMA_MAGIC, 5, unsigned int)
#define GET_SCC_DMA_TX_BUF_PHYS             _IOR(DMA_MAGIC, 6, unsigned int)
#define GET_SCC_DMA_RXBD_PHYS               _IOR(DMA_MAGIC, 7, unsigned int)
#define GET_SCC_DMA_TXBD_PHYS               _IOR(DMA_MAGIC, 8, unsigned int)
#define ENABLE_IRQ                          _IOR(DMA_MAGIC, 9, unsigned int)
#define DISABLE_IRQ                         _IOR(DMA_MAGIC, 10, unsigned int)

/* 
 * Defines for GE/SCC DMA 
 */
#define PRINCE_GE_DMA_RXBD_NUM              1024
#define PRINCE_GE_DMA_TXBD_NUM              256
#define PRINCE_GE_DMA_TXBD_TYPE             3
#define PRINCE_GE_DMA_RXBD_BUF_MAX          0x3fff
#define PRINCE_GE_DMA_RXBD_BUF_MIN          0x0040
#define PRINCE_GE_DMA_RXBD_BUF_SIZE         1600
#define PRINCE_GE_DMA_TXBD_BUF_MAX          0x3fff
#define PRINCE_GE_DMA_TXBD_BUF_MIN          0x0040

#define PRINCE_SCC_BUF_NUM                  0x10
#define PRINCE_SCC_BUF_SIZE                 0x80

#define BYTES_PER_BD                        8
#define DWDS_PER_BD                         2     /* 0- control, 1-buffer addr */


/*
 * Marcos for GIC SPI ID
 */
#define NR_IRQS                             128

#define GIC_SPI_ID_64                       64
#define MVL_PHY_INTR_ID                     84
#define XADC_INTR_ID                        85
#define SCC_ERR_INTR_ID                     86
#define SCC_MGMT_INTR_ID                    87
#define SCC_NETIO_INTR_ID                   88
#define GE_DMA_INTR_ID                      89
#define GE_DMA_ERR_INTR_ID                  90
#define GE_MAC_INTR_ID                      91
#define GIC_SPI_ID_95                       95

#define spi1_mask(id)                       (1 << ((id) - GIC_SPI_ID_64))
/*
 * External Functions
 */
extern ulong get_fpga_base(void);
extern ulong get_ge_dma_base(void);
extern ulong get_ge_mac_base(void);
extern ulong get_scc_base(void);
extern ulong get_ps_intr_ctrl_base(void);
extern ulong get_ps_ddr_ctrl_base(void);
extern ulong get_gic_spi_status1(void);

#endif /* _PCMAP_H_ */


/******** History ******** 
$Log: pcmap.h,v $
Revision 1.2  2013/07/30 10:11:21  xiaoyizh
Add macro for DDRC base address.

Revision 1.1  2013/04/19 07:17:50  xiaoyizh
Initial check in for Prince NIM.

$Endlog$
*/
