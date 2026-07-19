/* $Id: linux_api.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/linux_api.h,v $
 *------------------------------------------------------------------
 * linux_api.h  - Definitions file for common linux application .
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */


#ifndef __LINUX_API__
#define __LINUX_API__

#include <termios.h>

/* linux_uart.c */
#define DEFAULT_CASE  0
#define SPECIAL_PAT   1
#define TRIG_DIAG_M   2

typedef struct s_uart_ {
    char *dev;
    char buf[1024];
    uint tst_typ;
} s_uart;
/* linux_uart.c */

typedef struct uart_baud_info_ {
    char    *name;
    speed_t baud_rate;
} uart_baud_info;

extern int fd_mb_i2c[];
extern void mmap_mb_i2c_open(void);
extern void mmap_mb_i2c_open(void);
#ifdef SHRINKRAY_NOUSED
extern unsigned long mmap_addr(unsigned long long, unsigned int);
extern void unmap_addr(unsigned long, unsigned int);
extern void mmap_open(void);
extern void mmap_close(void);
extern void unmap_dev(mem_info_t *addr, ulong,  int); 
extern unsigned long mmap_dev(mem_info_t *addr, const uint64_t phy_addr, 
			      const uint32_t size);
extern uint8_t mmap_rd8(mem_info_t *addr, uint32_t);
extern uint16_t mmap_rd16(mem_info_t *addr, uint32_t);
extern uint32_t mmap_rd32(mem_info_t *addr, uint32_t);
extern void mmap_wr8(mem_info_t *addr, uint32_t, uint8_t val);
extern void mmap_wr16(mem_info_t *addr, uint32_t, uint16_t val);
extern void mmap_wr32(mem_info_t *addr, uint32_t, uint32_t val);
extern uint32_t pci_config_read(uint32_t bus, uint16_t device, uint32_t, int offset);
extern uint32_t pci_config_write(uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value);
#endif //#ifdef SHRINKRAY_NOUSED
extern int addr_vtop(ulong vir_addr, ulong *phy_addr);
extern int ioptov_mmap(ulong phy_addr, ulong size, ulong *vir_addr);
extern ulong pg_align_addr(ulong addr);
extern ulong pg_merge_addr(ulong page_bits, ulong offset_bits);
extern ulong getmemfree(void);
extern int bin2hex (const char *varname, unsigned char *hex,
                    unsigned int size);
#ifdef SHRINKRAY_NOUSED
extern uint32_t linux_mb_i2c_read(uint16_t bus_no, uint16_t addr,
                                  uint32_t offset,
                                  uint32_t len, uint8_t *buf, uint8_t);
extern uint32_t linux_mb_i2c_write(uint16_t bus_no, uint16_t addr,
                                   uint32_t offset,
                                   uint32_t len, uint8_t *buf);
#endif //#ifdef SHRINKRAY_NOUSED
extern int get_mb_i2c_minor_num(int i2c_addr, int *val);
//extern void linux_init_cookie_info(void *);
extern void hwic_klm_open(int slot, char* name);
extern void diag_timer_open(void);
extern void diag_timer_close(void);
extern void mem_mgr_open(void);
extern void mem_mgr_close(void);
extern void hwic_klm_close(int, char *);
extern void nm_klm_open(int slot, char* name);
extern void nm_klm_close(int, char *);
extern int nm_get_klm_fd(int slot);
extern int hwic_get_klm_fd(int slot);
extern int nm_get_klm_slot(int slot);
extern int pci_enable_msi(int, int);
#ifdef SHRINKRAY_NOUSED
extern int pci_enable_msi_phys(int, uint64_t);
#endif //#ifdef SHRINKRAY_NOUSED
extern int pci_disable_msi(int);
extern int get_asic_utils_fd();
extern long getfreememstart(void);
extern void init_large_memeory_usage_cnt(void);
extern void init_getfreemem(void);
extern unsigned long PHY_ADDR(unsigned long);
extern unsigned long MEM_TO_PCI(unsigned long);
extern unsigned long PHY_TO_KSEG1(unsigned long);
extern unsigned long MEM_TO_PCI(unsigned long);
extern unsigned long VIR_ADDR(unsigned long);
extern unsigned long PCI_TO_MEM(unsigned long);
extern void init_fd(void);
extern void sm_intr_open(void);
extern int app_get_ref_cnt(void);
extern const char *gettestname(void);

extern void print_offset_val(unsigned char *, unsigned long, unsigned long,
                             unsigned int, unsigned char *);
extern void print_offset(unsigned char *str1, unsigned long, unsigned long,
                         unsigned int, unsigned char *);
extern void print_debug(char *);
extern void flush_test_progress_buf(void);
extern int32_t i2c_dev_rd(void *);

extern void print_spining_wheel(int);
extern int rx_uart(char *, int, char *, int, uint);
extern int tx_uart(char *, char *, int);
extern int uart_intf_test(char *, const char *, speed_t);
extern int uart_msg_exh_test(char *, const char *, const char *,uint);
extern int file_exist(unsigned char *dest);
extern int get_ip(const char *if_name, char *ip_addr);
#endif /* __LINUX_API__ */

/******** History ********/ 
/*
 * $Log: linux_api.h,v $
 * Revision 1.2  2015/05/25 03:59:10  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.2  2015/04/29 11:36:26  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *
 * ------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:37  palin2
 * Initial check-in Skye module side Diag code.
 *
 * $Endlog$
 */

