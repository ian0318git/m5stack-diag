/* $Id: linux_api.h,v 1.21 2014/06/11 17:28:58 siyen Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/linux_api.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#ifndef __LINUX_API__
#define __LINUX_API__

#include "mem_mgr.h"
#include <termios.h>

/* linux_uart.c */
enum { DEFAULT_CASE, 
       SPECIAL_PAT,  
       TRIG_DIAG_M
};

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};

#define UART_BUF_SIZE 2048

typedef struct s_uart_ {
    char *dev;
    char buf[UART_BUF_SIZE];
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
//extern unsigned long mmap_addr(unsigned long long, unsigned int);
//extern void unmap_addr(unsigned long, unsigned int);
//extern void mmap_open(void);
//extern void mmap_close(void);
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
extern int addr_vtop(ulong vir_addr, ulong *phy_addr);
extern int ioptov_mmap(ulong phy_addr, ulong size, ulong *vir_addr);
extern ulong pg_align_addr(ulong addr);
extern ulong pg_merge_addr(ulong page_bits, ulong offset_bits);
extern ulong getmemfree(void);
extern int readfile(const char *varname, unsigned char *hex,
                    unsigned int size);
extern uint32_t linux_mb_i2c_read(uint16_t bus_no, uint16_t addr,
                                  uint32_t offset,
                                  uint32_t len, uint8_t *buf, uint8_t);
extern uint32_t linux_mb_i2c_write(uint16_t bus_no, uint16_t addr,
                                   uint32_t offset,
                                   uint32_t len, uint8_t *buf);
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
extern int pci_enable_msi_phys(int, uint64_t);
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

extern void print_offset_val(char *, unsigned long, unsigned long,
                             unsigned int, char *);
extern void print_offset(char *str1, unsigned long, unsigned long,
                         unsigned int, char *);
extern void print_debug(char *);
extern void flush_test_progress_buf(void);
extern int32_t i2c_dev_rd(void *);

extern void print_spining_wheel(int);
extern int rx_uart(char *, int, char *, int, uint);
extern int tx_uart(char *, char *, int);
extern int uart_intf_test(char *, const char *, speed_t);
extern int uart_msg_exh_test(char *, const char *, const char *,uint);
extern int file_exist(char *dest, size_t *);
extern int get_ip(const char *if_name, char *ip_addr);
extern void print_hex_dump(FILE *, int level, const char *, int,
                           int rowsize, int groupsize,
                           const void *buf, size_t len, unsigned char);
extern int logfile(const char *name, char *data, int);
extern int driver_loaded(char *name);
extern int linux_echo(char *name, char * buf, char *option);
#endif /* __LINUX_API__ */

/*-------------------------------------------------
$Log: linux_api.h,v $
Revision 1.21  2014/06/11 17:28:58  siyen
Expand the UART buffer size (CSCup02982)

Revision 1.20  2014/05/29 00:37:40  mcharon
rename bin2hex to readfile

Revision 1.19  2014/05/20 03:49:51  alpeng
add definition for UART_BUF_SIZE

Revision 1.18  2014/02/04 18:53:04  mcharon
add function 'driver_loaded' to check if driver is loaded

Revision 1.17  2013/12/18 00:24:40  mcharon
file_exist now returns size of file

Revision 1.16  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.15  2013/11/11 21:18:38  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.14  2013/02/15 10:31:38  palin2
Update UART test by add a new parameter to allow using specific baud rate.

Revision 1.13  2013/02/13 18:25:12  mcharon
add function to get ip address of system interface

Revision 1.12  2013/01/14 21:45:06  mcharon
move file_exist to linux_api.c

Revision 1.11  2012/09/24 05:58:25  alpeng
add argument for rx_uart(), for getting last character on rx

Revision 1.10  2012/09/17 08:28:55  alpeng
revert uart_intf_test(), add uart_msg_exh_test()

$Endlog$
*/
