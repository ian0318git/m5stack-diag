/* $Id: plug_testcard_test.h,v 1.6 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/plug_testcard/plug_testcard_test.h,v $
 *------------------------------------------------------------------
 *
 * plug_testcard_test.h - Header file for Pluggable Test card 
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
 
#ifndef __PLUG_TESTCARD_TEST__
#define __PLUG_TESTCARD_TEST__
/* On Curie2RU, after Module Reset pin Test, the NVMe device file disappeared.
 * So increase delay to ensure that the NVMe device file can appear for NVMe SSD Test */
#define PLUG_TESTCARD_UNRESET_WAIT          (5000)
#define PLUG_TESTCARD_GPS_PIN                (3)
#define PLUG_TESTCARD_GPS_MASK               (0x300)
#define PLUG_TESTCARD_USB_DEVFD_COUNT        (10)

#define UART_TEST_DELAY                     (100)
#define UART_FLUSH_FIFO_TIMES               (50)

#define PLUG_FPGA_I2C_ACK_MUX               (0)
#define PLUG_FPGA_TC_I2C_ADDR_GPIO_EXP      (0x38 >> 1)   /* Pluggable Test Card GPIO Expander */
#define PLUG_FGPA_TC_I2C_ADDR_ACT2          (0xE6 >> 1)   /* Pluggable Test Card ACT2 */
#define PLUG_FPGA_I2C_ACK_REG_ADD           (0)
#define PLUG_FPGA_I2C_ACK_SUB_ADD           (1)
#define PLUG_FPGA_I2C_ACK_DATA_LEN          (1)
#define DEV_NAME_LEN 64 

extern int plug_testcard_main(void *);
extern int system(const char *); 
extern int plug_testcard_pcie_post_pwr_up(void); 
extern int plug_testcard_pcie_post_pwr_down(void); 
extern void plug_testcard_pcie_device_remove(void); 
#endif

/*-------------------------------------------------
$Log: plug_testcard_test.h,v $
Revision 1.6  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.5  2020/01/09 01:02:33  jiajliu
Merge Curie 2RU to main trunk

Revision 1.4  2019/08/06 06:56:16  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.3  2018/11/23 09:10:40  hondwang
Re-instruct pluggable common code with CDETs CSCvn17216

Revision 1.2.62.3  2018/11/16 06:40:49  hondwang
modify PRRQ suggest with CSCvn17216 pluggable re-instruct

Revision 1.2.62.2  2018/10/15 09:43:56  hondwang
move GPS pin define back to platform code, because Star and Curie have diff FPGA configure

Revision 1.2.62.1  2018/10/15 06:50:50  hondwang
pluggable common code re-instruct modify code

Revision 1.2  2018/01/20 05:01:10  hondwang
prepare merge star-branch-c9xx to main trunk

Revision 1.1.4.4  2017/09/23 03:47:00  hondwang
Fix pluggable reset mode not working issue

Revision 1.1.4.3  2017/08/31 05:01:49  hondwang
Add GPS test with pluggable test card

Revision 1.1.4.2  2017/08/08 07:44:28  hondwang
add pluggable testcard for star-branch-c9xx

Revision 1.1.2.2  2017/07/21 09:00:05  hondwang
fix pluggable fail with USB enumerate time not enought

Revision 1.1.2.1  2017/07/13 06:32:22  tirawan
Reorganize Star Pluggable directory structure

Revision 1.1.2.5  2017/06/26 22:48:41  tirawan
UART test for Cisco pluggable FPGA

Revision 1.1.2.4  2017/06/26 08:11:55  steja
Fixed Pluggable testcard USB test

Revision 1.1.2.3  2017/06/22 19:27:12  tirawan
Add LTE Test items and add log section at the bottom of the code


*/

