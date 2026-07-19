/* $Id: cpss_ioctl.h,v 1.2 2021/09/24 01:21:39 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/cpss_common/cpss_ioctl.h,v $
 *------------------------------------------------------------------
 *
 * Filename:	cpss_ioctl.h
 *
 *------------------------------------------------------------------
 */

#ifndef __CPSS_IOCTL_H__
#define __CPSS_IOCTL_H__

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
    unsigned int exregs_size;
    
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

#endif /* CPSS_IOCTL_H_ */
/*
 *------------------------------------------------------------------
 * $Log: cpss_ioctl.h,v $
 * Revision 1.2  2021/09/24 01:21:39  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.1  2021/04/12 08:38:35  illiu
 * Add file: cpss common code
 *
 * Revision 1.1.2.1  2021/01/26 02:55:25  illiu
 * Rename nim_dm prefix file to cpss prefix
 *
 * Revision 1.1.2.1  2020/09/09 09:18:40  illiu
 * Modified to support Dreamliner with CPSS 4.2 library.
 *
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
