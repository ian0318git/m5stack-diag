/* $Id: platform_aikido.h,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/platform_aikido.h,v $
 *------------------------------------------------------------------
 *
 * Filename   : platform_aikido.h
 * Description: Summary of all Aikido related header files.
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "act2l_typedef.h"
#include "common.h"
#include "common_utils.h"
#include "cross_platform.h"
#include "defs.h"
#include "error.h"
#include "i2c_address.h"
#include "platform_fpga.h"
#include "platform_cookie_api.h"
#include "tam_act2_api_drv_support.h"
#include "n2g_api_rc.h"
#include "nvmonvars.h"
#include "object.h"
#include "platform_cookie.h"
#include "platform_i2c_api.h"
#include "platform_poe_psu.h"
#include "proto.h"
#include "queryflags.h"
#include "types.h"
#include "tam_aikido_mailbox.h"
#include "tam_library.h"

#include "cookie_4.h"  // put in the lower list
#include "menu.h"  // put in the lower list
#include "nmc93c46.h"  // put in the lower list
#include "smart_cookie.h"  // put in the lower list than menu.h & nmc93c46.h



#define I2CBUS0      "/dev/i2c-0"
#define MB_I2C_ADDR_AIKIDO_ACT2  0x77
#define PLATFORM_BUFF_SIZE          259
#define EEPROM_RD_WR_LENGTH     512
#define EEPROM_WRITE_ADDR       0x380

#define BRIDGE_WIN_BASE_REG_ADDR(x)   (0xf270ff04 + (0x8 * (x)))
#define DEVBUS_WINBASE_MSK            (0xffff0000)
#define MAX_PID_LEN             31
#define MAX_CSN_LEN             11

/* AIKIDO Register */
#define FPGA_AIKIDO_BASE_REG_ADDR         0xFDFF8000
#define FPGA_AIKIDO_REG                   0x94



enum {
    RC_I2C_OP_OK = 0,
    RC_I2C_BUSY,
    RC_I2C_TIMEOUT,
    RC_I2C_DMA_ADDR_NOT_64ALIGN,
    RC_I2C_SLV_NACK,
    RC_I2C_SLV_SUB_ADDR_NACK,
    RC_I2C_BUS_ERR,
    RC_I2C_UNKNOWN,  /* always last item */
};


extern void act2_init_cont(void *con);

/*
 *------------------------------------------------------------------
 * $Log: platform_aikido.h,v $
 * Revision 1.2  2019/06/14 05:24:49  mikech2
 * Collapse katar-branch00 to Main Trunk
 *
 * Revision 1.1.2.2  2019/06/06 02:44:25  mikech2
 * clean out the unnecessary definitions and TSN string
 *
 * Revision 1.1.2.1  2019/02/12 08:06:29  mikech2
 * rename katar_*.h files
 *
 * Revision 1.1.2.6  2018/12/27 02:26:10  peteteng
 * Update Aikido LPC scratchpad reg. addr.
 *
 * Revision 1.1.2.5  2018/12/07 14:41:42  peteteng
 * Modify addr. of Aikido LPC scratchpad test
 *
 * Revision 1.1.2.4  2018/11/22 02:50:49  peteteng
 * Add Aikido register read/write utility
 *
 * Revision 1.1.2.3  2018/11/14 08:14:58  peteteng
 * Add Aikido FPGA register test
 *
 * Revision 1.1.2.2  2018/11/08 02:21:52  peteteng
 * Remove names in comment
 *
 * Revision 1.1.2.1  2018/10/22 08:02:22  mikech2
 * Move project folder to common/src/katar/x86
 *
 * Revision 1.1.2.8  2018/10/19 07:51:41  iachang
 * Update TAM lib. from v3.3.25 to v3.4.4, fixed endian function naming issue
 *
 * Revision 1.1.2.7  2018/07/24 09:54:12  peteteng
 * Add SFP cookie - read
 *
 * Revision 1.1.2.6  2018/07/12 08:02:08  peteteng
 * add tam_lib_platform_write/read
 *
 * Revision 1.1.2.5  2018/07/02 08:21:10  peteteng
 * Update katar tam lib
 *
 * Revision 1.1.2.4  2018/07/02 02:40:32  peteteng
 * update reset and unreset functions
 *
 * Revision 1.1.2.3  2018/06/29 07:17:31  mikech2
 * Remove compile warning and unused files
 *
 * Revision 1.1.2.2  2018/06/29 03:40:01  peteteng
 * Add ACT2 utility Menu
 *
 * Revision 1.1.2.1  2018/06/26 06:30:09  peteteng
 * Add Aikido Cookie menu
 *
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */



