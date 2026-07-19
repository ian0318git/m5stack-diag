/* $Id: host2dp_mbox.h,v 1.9 2018/05/18 09:24:52 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/cavium/host2dp_mbox.h,v $
 *------------------------------------------------------------------
 * host2dp_mbox.h - Overlord mailbox messaging code.
 * 
 * March 2011 ptong
 * Copyright (c) 2011-2018 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

/* Define CPU */
#define CVMX_OVERLORD   0
#define CVMX_NEPTUNE    1

/* Message control and status indicators */
#define MSG_ID_MASK        0x00ffffff
#define MSG_CTRL_MASK      0xff000000
#define MSG_VALID          0x80000000
#define MSG_PASS           0x01000000

/* Message ID supported between the host and data plane
 */
typedef enum mbox_msg {
    MBOX_MSG_DP_UNKNOWN   = 0x100,  /* Data plane messages */
    MBOX_MSG_DP_CN6645,             /* 10-core Octeon for Overlord */
    MBOX_MSG_DP_CN6635,             /* 6-core Octeon for Omaha */
    MBOX_MSG_DP_HELLO_TEST,
    MBOX_MSG_DP_MAIN_TEST,
    MBOX_MSG_DP_MAIN_NOEXT_TEST,
    MBOX_MSG_DP_CLK_TRIG_VERIFY,
    MBOX_MSG_DP_CN7260R,
    MBOX_MSG_DP_CN7245R,
    MBOX_MSG_DP_CN7235R,
    MBOX_MSG_DP_END
} mbox_msg_t;

#define MBOX_FLAG_MSG_DP_GE_INT_LPBK (0x1 << 16)

#define MBOX_BUF_SZ                 512 /* bytes */

typedef struct mbox_ {
    volatile uint msg_ctl;
    volatile uint msg_addr;
    volatile char msgbuf[MBOX_BUF_SZ];
} mbox_t;

/* These fixed address are picked for the named block
 * based on the uboot and linux memory allocation
 */
#define OVERLORD_DP_MBOX_NAMED_BLOCK     "dp_mailbox"
#define OVERLORD_DP_MBOX_SIZE            1024
#define OVERLORD_DP_IN_MBOX_PHY_ADDR     0xdf00000
#define OVERLORD_DP_OUT_MBOX_PHY_ADDR    (OVERLORD_DP_IN_MBOX_PHY_ADDR + \
					 OVERLORD_DP_MBOX_SIZE)

/* it should be ok about using the same size with o2 */
#define NEPTUNE_DP_IN_MBOX_PHY_ADDR      0xf0000000
#define NEPTUNE_DP_OUT_MBOX_PHY_ADDR     (NEPTUNE_DP_IN_MBOX_PHY_ADDR + \
					 OVERLORD_DP_MBOX_SIZE)


extern mbox_t *in_mbxp, *out_mbxp;
extern mbox_t *host_out_mbxp, *host_in_mbxp;

extern int is_mbox_empty(mbox_t *mbxp);
extern void ack_msg(void);
extern uint get_msg(void);
extern uint get_msg_id(uint msg_ctl);
extern int is_msg_pass(uint msg);
extern void send_msg(uint msg);

extern void dp_mbox_init(int);

/*-------------------------------------------------
$Log: host2dp_mbox.h,v $
Revision 1.9  2018/05/18 09:24:52  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.8.20.5  2017/04/05 09:13:47  leschen
Sync with <ng_diag-tag-032917>

Revision 1.8.20.4  2016/12/27 02:03:29  meho
Added ge-Int loopback flag to control Cavium GE int/ext loopback test.

Revision 1.8.20.3  2016/12/01 09:37:53  alpeng
update mbox addr to 0xf0000000 for supporting new uboot

Revision 1.8.20.2  2016/11/15 07:16:53  alpeng
resolve cvm 2nd test issue

Revision 1.8.20.1  2016/11/03 08:26:54  alpeng
merge octeon_test.c with o2

Revision 1.8  2015/02/14 12:48:41  kodko
Collapse timing card branch code into main trunk.

Revision 1.7.22.1  2014/03/11 02:40:13  leschen
Support 1588 clk/trig verification.

Revision 1.7  2012/11/02 00:55:51  ptong
Add comment and clean-up

Revision 1.6  2012/06/19 23:20:14  ptong
Check correct Octeon model is used on the platform

Revision 1.5  2012/05/12 00:00:07  ptong
Added DP_MAIN_NOEXT_TEST to cavium

Revision 1.4  2012/04/17 22:01:26  ptong
Added more utility to run DP test from host.

Revision 1.3  2012/04/11 21:27:16  ptong
Setup cavium named block for mailbox area, and use nc server on cavium to take command from host

Revision 1.2  2012/03/28 00:38:18  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:07  ptong
Initial archive of ng_diag module


$Endlog$
*/
