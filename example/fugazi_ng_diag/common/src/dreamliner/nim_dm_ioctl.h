/* $Id: nim_dm_ioctl.h,v 1.2 2015/02/27 10:02:21 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner/nim_dm_ioctl.h,v $
 *------------------------------------------------------------------
 * DM kernel driver related exports and defines API
 *
 * Christine Wen -- Feb. 2014
 *
 * Copyright (c) 2014-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *---------------------------------------------------------------------------
 */

#ifndef __NIM_DM_IOCTL_H__
#define __NIM_DM_IOCTL_H__

typedef struct pcie_switch_port_mapping_s {
    /*
     * secondary bus number of PCIe switch port of specific slot/bay
     */
    unsigned int sec_bus;

    /*
     * subordinate bus number of PCIe switch port of specific slot/bay
     */
    unsigned int sub_bus;

    unsigned int slot;
    unsigned int bay;
} pcie_switch_port_mapping_t;

typedef struct pcie_mapping_array_s {
    unsigned long long num;

    /*
     * mapping entry pointer for both 32bit app and 64bit kernel
     */
    unsigned long long ents;
} pcie_mapping_array_t;

typedef struct pcie_config_reg_s
{
    unsigned int bus;
    unsigned int dev;
    unsigned int func;
    unsigned int reg;
    unsigned int data;
} pcie_config_reg_t;

typedef struct pcie_find_dev_s
{
    unsigned short vendorId;
    unsigned short devId;

    /*
     * not used since one cpss lib manages one pp
     */
    unsigned int instance;

    unsigned int bus;
    unsigned int dev;
    unsigned int func;
} pcie_find_dev_t;

typedef struct intrline_to_vector_s
{
    /*
     * interrupt line number
     */
    unsigned int intrline;

    /*
     * irq number
     */
    unsigned int vector;
} intrline_to_vector_t;

typedef struct config_ppregs_size_s
{
    unsigned int config_size;
    unsigned int ppregs_size;
} config_ppregs_size_t;

#define READ_WRITE_TYPE_CONFIG    0
#define READ_WRITE_TYPE_PPREGS    1
typedef struct read_write_data_s
{
    /*
     * access type: config space or ppregs space
     */
    unsigned int type;

    unsigned int offset;

    /*
     * DMA operation burst size, not used for u64 operation
     */
    unsigned int burst;
    unsigned int length;

    /*
     * actual data for u64 op, data pointer for dma op
     */
    unsigned long long data;
} read_write_data_t;

typedef struct dma_mem_s
{
    unsigned int size;

    /*
     * bus address or physical address if no IOMMU
     */
    unsigned int bus_addr;

    /*
     * kernel virtual address
     */
    unsigned long long kernel_addr;
} dma_mem_t;



#define DM_MAIN_IOC_MAGIC 'M'
#define DM_MAIN_SET_PCIE_MAPPING _IOW(DM_MAIN_IOC_MAGIC, 0, unsigned long long)


#define DM_SUB_IOC_MAGIC 'S'
#define DM_SUB_IOC_INTCONNECT    _IOWR(DM_SUB_IOC_MAGIC, 0, unsigned int)
#define DM_SUB_IOC_INTENABLE     _IOW (DM_SUB_IOC_MAGIC, 1, unsigned int)
#define DM_SUB_IOC_INTDISABLE    _IOW (DM_SUB_IOC_MAGIC, 2, unsigned int)
#define DM_SUB_IOC_WAIT          _IO  (DM_SUB_IOC_MAGIC, 3)

#define DM_SUB_IOC_PCIECONFIGWRITEREG    _IOW (DM_SUB_IOC_MAGIC, 4, unsigned long long)
#define DM_SUB_IOC_PCIECONFIGREADREG     _IOR (DM_SUB_IOC_MAGIC, 5, unsigned long long)
#define DM_SUB_IOC_GETINTVEC             _IOWR(DM_SUB_IOC_MAGIC, 6, unsigned long long)
#define DM_SUB_IOC_FIND_DEV              _IOWR(DM_SUB_IOC_MAGIC, 7, unsigned long long)
#define DM_SUB_IOC_CONFIG_PPREGS_SIZE    _IOR (DM_SUB_IOC_MAGIC, 8, unsigned long long)
#define DM_SUB_IOC_U64_READ              _IOWR(DM_SUB_IOC_MAGIC, 9, unsigned long long)
#define DM_SUB_IOC_U64_WRITE             _IOW (DM_SUB_IOC_MAGIC, 10, unsigned long long)

#define DM_SUB_IOC_ALLOC_DMA_MEM    _IOWR(DM_SUB_IOC_MAGIC, 11, unsigned long long)
#define DM_SUB_IOC_FREE_DMA_MEM     _IOW (DM_SUB_IOC_MAGIC, 12, unsigned long long)
#define DM_SUB_IOC_DMA_READ         _IOWR(DM_SUB_IOC_MAGIC, 13, unsigned long long)
#define DM_SUB_IOC_DMA_WRITE        _IOW (DM_SUB_IOC_MAGIC, 14, unsigned long long)

#define DM_SUB_IOC_EVENT            _IO(DM_SUB_IOC_MAGIC, 15)
#define DM_SUB_IOC_INTDISCONNECT    _IO(DM_SUB_IOC_MAGIC, 16)

#endif /* NIM_DM_IOCTL_H_ */
/*
 *------------------------------------------------------------------
 * $Log: nim_dm_ioctl.h,v $
 * Revision 1.2  2015/02/27 10:02:21  iachang
 *
 * Add support dreamliner NIM
 *
 *
 * Revision 1.1.4.2  2015/01/28 22:59:22  iachang
 * Dreamliner-branch2 initial check-in.
 *
 * Revision 1.1.2.1  2014/12/02 08:04:13  iachang
 * Dreamliner Diag initial check-in.
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
