/* $Id: router_if.h,v 1.2 2015/05/25 03:59:10 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/include/router_if.h,v $
 *------------------------------------------------------------------
 * router_if.h - provides the protocol interface defines and structures
 *               between Volant Host and CE
 *
 * Note: the structures and defines must match the CE structures
 *       and defines.
 *
 * October 2001, Frank Liu, Steve Shih
 * 
 * Copyright (c) 2007-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef ROUTER_IF_H
#define ROUTER_IF_H

#define CE_MODULE          0xfeed

#define MAX_FE_BUF_SIZE    1470

typedef char mac_addr_t[6];

typedef struct {
    mac_addr_t  dest_addr;
    mac_addr_t  src_addr;
    ushort      pkt_len;
} ether_hdr_t;    

typedef struct fe_packet_t {
    ether_hdr_t eth_hdr;
    uchar       data[1500];
} fe_packet_t;

typedef struct {
    int     slot;
    int     (*func)();
    int     flag;
    uchar   cmd;
    ulong   pci_base_addr;        /* PCI mem base address space */
    ulong   pci_modem_addr;       /* PCI modem base addr space */
    int     idsel;
    uchar   platform;             /* Router platform type */
    ushort  cookie_id;
    boolean double_wide;          /* Single wide or double wide NM */
    boolean ce_is_awake;
    boolean ide_present;
    ushort  ide_id;
    boolean scsi_present;
    ushort  scsi_id;
    boolean debug;                /* only turns on debug when set */
} module_info_t;

typedef struct {
    ushort  src_id;       /* type of host NM or AIM... (use cookie ID)  */
    ushort  dest_id;      /* type of receiver NM or AIM...              */
    uchar   op_type;      /* type of operation, request or response     */
    uchar   cmd;          /* command                                    */
    uchar   ret_code;     /* return code, ACK, PASS, FAIL, etc          */
    uchar   platform;     /* router platform type                       */
    ushort  data_len;     /* ethernet data length; does not include     */
} if_hdr_t;               /* header above                               */
   
typedef struct {
    if_hdr_t if_hdr;
    uchar    *if_buf_p;   /* buffer contains data for host/ce interface */  
} if_t;

typedef struct {
    ulong   op_type;      /* type of operation, request or response     */
    ulong   cmd;          /* command                                    */
    ulong   ret_code;     /* return code, ACK, PASS, FAIL, etc          */
    ulong   platform;     /* router platform type                       */
    ulong   param1;
    ulong   param2;
    ulong   param3;
    ulong   nvram_val;
} if_info_t;

typedef struct {
    ether_hdr_t ether_hdr;  /* 14 bytes */
    ushort      pad;
    if_info_t   if_info;
    uchar       if_buf[MAX_FE_BUF_SIZE]; /* data buffer for host/ce intf */
} if_ether_t;

/*************************** WARNING *************************/
/*************************************************************/
/*    All constants defined below must match the CE code     */
/*************************************************************/
/*************************** WARNING *************************/

/* Host Interface Structure - operation type defines */
#define OP_NULL            0
#define OP_TEST_REQUEST    1   /* host->CE */
#define OP_INFO_REQUEST    2   /* host->CE */
#define OP_CE_REQUEST      3   /* CE->host */
#define OP_RESPONSE        4   /* mutual   */
#define OP_READY           5   /* to host : moudle is up and ready */
#define OP_TEST_STOP       6   /* host->module: to stop current test */

/* Host Interface Structure - command defines shared between host and CE */
#define CMD_NULL           0x00
#define CMD_KEEP_ALIVE     0x01
#define CMD_KEEP_ALIVE_UP  0x02

/* Host Interface Structure - Host Command defines for tests */

/**************************************************************
 * Each command being added, the timeout value in 
 * /src/router_if.c set_cmd_timeout() needs to be added
 *
 * Also, the prpass needs to be added in the same file's
 * prpass_cmd().
 *************************************************************/
#define HC_ETH0_INT_TEST      0x10
#define HC_ETH0_PHY_TEST      0x11
#define HC_ETH0_EXT_TEST      0x12
#define HC_ETH1_INT_TEST      0x13
#define HC_FLASH_ABBR_TEST    0x14
#define HC_FLASH_TEST         0x15
#define HC_USB_TEST           0x16
#define HC_IDE_RAND_TEST      0x17   /* do not insert cmd below */
#define HC_IDE_JUMP_TEST      0x18   /* between 0x17 - 0x1F     */
#define HC_IDE_FULL_TEST      0x19
#define HC_SCSI_RAND_TEST     0x1A
#define HC_SCSI_JUMP_TEST     0x1B
#define HC_SCSI_FULL_TEST     0x1C
#define HC_DW_SCSI_RAND_TEST  0x1D
#define HC_DW_SCSI_JUMP_TEST  0x1E
#define HC_DW_SCSI_FULL_TEST  0x1F   /* do not insert cmd above */
#define HC_MEM_ABBR_TEST      0x20
#define HC_MEM_TEST	      0x21
#define HC_ETH0_EMI_TEST      0x22
#define HC_NVRAM_TEST         0x23
#define HC_COMPACT_TEST       0x24
#define HC_COMPACT_FULL_TEST  0x25
#define HC_ETH1_EXT_TEST      0x26
#define HC_COMPLETE_TEST      0x27

/* Host Interface Structure - CE Command; CE-to-host failed command */
#define CC_INVALID_OP      0x30  /* invalid op_type received */
#define CC_INVALID_ID      0x31  /* invalid host ID received */
#define CC_INVALID_TEST    0x32  /* invalid test cmd received */
#define CC_INVALID_INFO    0x33  /* invalid info request received */
#define CC_TIMER_TIMEOUT   0x34  /* CE timer time out */

/* Host Interface Structure - command defines for info request */
#define HC_SET_COOKIE      0x40
#define HC_SHOW_COOKIE     0x41
#define HC_RECOVER_COOKIE  0x42
#define HC_SHOW_SCSI       0x43
#define HC_READ_MEM        0x44
#define HC_WRITE_MEM       0x45
#define HC_READ_FLASH      0x46
#define HC_WRITE_FLASH     0x47
#define HC_CPU_INFO        0x48
#define HC_MEM_INFO        0x49
#define HC_FLASH_INFO      0x4A
#define HC_SET_VERBOSE     0x4B
#define HC_CLR_VERBOSE     0x4C
#define HC_SHOW_VERBOSE    0x4D
#define HC_SHOW_VER        0x4E
#define HC_PGM_BOOTLOADER  0x4F  /* copy bootloader from compact->strata flash */
#define HC_PGM_DIAG        0x50  /* copy diag from compact->strata flash */

/* Host Interface Structure - return code defines */
#define RET_NULL           0   /* don't care about returned code */
#define RET_PASS           0
#define RET_FAIL           1
#define RET_ACK            2
#define RET_RUNNING        3   /* info timer the test is still running */
#define RET_DATA           4   /* return test data; need checking */

#define SWAP16(X) \
        ( \
        (((X) & 0x00ff) << 8) + \
        (((X) & 0xff00) >> 8) \
        )

#define SWAP32(X) \
        ( \
        (((X) & 0x000000ff) << 24) + \
        (((X) & 0x0000ff00) << 8)  + \
        (((X) & 0x00ff0000) >> 8)  + \
        (((X) & 0xff000000) >> 24) \
        )

extern void *module_malloc(ulong);
extern int  wake_up_ce(void);
extern void send_keep_alive(void);
extern void prpass_cmd(uchar);
extern int  prep_keep_alive(void);
extern int  prep_host_cmd(void);
extern int  prep_test_cmd(uchar);
extern void set_ce_cookie(void);
extern void show_ce_cookie(void);
extern void show_scsi_info(void);
extern void read_ce_mem(void);
extern void write_ce_mem(void);
extern void read_flash_mem(void);
extern void write_flash_mem(void);
extern void show_cpu_info(void);
extern void show_mem_info(void);
extern void show_flash_info(void);
extern void set_ce_verbose(void);
extern void clear_ce_verbose(void);
extern void show_ce_verbose(void);
extern void show_ce_version(void);
extern void cp_bootloader(void);
extern void cp_diag(void);

#endif   /* ROUTER_IF_H */

/******** History ******** 
$Log: router_if.h,v $
Revision 1.2  2015/05/25 03:59:10  steja
Add Support Skye SM

Revision 1.1.4.2  2015/04/29 11:36:27  steja
Code check-in to skye-branch2 for ER code review


------------------------------------------------------------------
Revision 1.1.2.1  2014/07/21 01:56:39  palin2
Initial check-in Skye module side Diag code.

$Endlog$
*/
