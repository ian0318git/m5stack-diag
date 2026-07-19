/* $Id: platform_cookie.h,v 1.8 2019/08/06 06:56:10 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_cookie.h,v $
 *------------------------------------------------------------------
 *
 * platform_cookie.h - Platform specific cookie defines from Xformers.
 *
 * Sept. 2008, Shih-Nan Huang
 *
 * Copyright (c) 2013-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _PLATFORM_COOKIE_H_ 
#define _PLATFORM_COOKIE_H_ 

#include "cli_cmd.h"

/*temporarily define, on sunridge POE_DAUGHTER_CARD is 1 */
#define POE_DAUGHTER_CARD 0
#define PLATFORM_BUFF_SIZE          259


extern unsigned short get_cookie_id(int, int, uchar*, uint16_t *,
                                    char *);
extern unsigned short get_hwic_cookie_id(int, void *, uchar);
extern int get_cookie_pid (int, int, unsigned char *, char *);
extern int alter_hwic_cookie(void);
extern int alter_wic_cookie_cli(boolean, cli_cookie_cmd *);
extern int alter_sm_cookie(void);
extern int alter_sm_cookie_cli(boolean, cli_cookie_cmd *);
extern int alter_vm_cookie(void);
extern int alter_vm_cookie_cli(boolean, cli_cookie_cmd *);
extern int alter_wic_dc_cookie(void);
extern int alter_wic_dc_cookie_cli (boolean mode, cli_cookie_cmd *cmd);
extern int alter_sm_dc_cookie(void);
extern int alter_sm_dc_cookie_cli (boolean mode, cli_cookie_cmd *cmd);

extern int get_pcb_serial(uchar *, char *);
extern int get_tlv_serial (uchar *, char *, uchar);
extern int get_ngio_mac_addr(int, int, uchar *);
extern int get_mb_pid(char *);
extern int get_plat_sku_cookie(void);

#endif /* _PLATFORM_COOKIE_H_ */

/******** History ********
$Log: platform_cookie.h,v $
Revision 1.8  2019/08/06 06:56:10  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.7.70.1  2018/09/07 01:43:58  alpeng
add spi read/write util for aikido; change tam lib on Makefile

Revision 1.7  2016/06/06 18:30:39  huanngo
Add code to support programming SUDI/WDC with Chassis S/N

Revision 1.6  2016/03/04 19:19:15  ptong
Clean up obsolete ISR platfrom PID

Revision 1.5  2014/08/29 10:27:38  danchung
add extern of get_cookie_pid function

Revision 1.4  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.3  2013/08/19 01:53:19  alpeng
using both FPGA and MB cookie to get/check board type

Revision 1.2  2013/07/10 01:34:53  alpeng
moving get_plat_sku() to platform_cookie.c. since the sku number coming from cookie.

Revision 1.1  2013/05/09 05:42:39  alpeng
moving overlord common code from x86

Revision 1.10  2012/11/17 01:15:18  mcharon
reset i2c device; don't cterr in driver code..propogate err message to slot.c

Revision 1.9  2012/10/11 08:24:42  alpeng
unsupported SATA on wic slot 3

Revision 1.8  2012/09/26 03:23:07  alpeng
check sata available via PID

Revision 1.7  2012/08/28 10:12:57  alpeng
fixed warning

Revision 1.6  2012/08/28 09:31:00  alpeng
support CLI discovery for SATA, fix typo on alter_sm_dc_cookie_cli()

Revision 1.5  2012/06/04 10:35:16  palin2
Clean up compiler warnings.

Revision 1.4  2012/06/02 00:57:22  srane
Fix warnings.

Revision 1.3  2012/05/05 04:27:43  mcharon
add cli for daughter card

Revision 1.2  2012/03/28 00:38:22  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
