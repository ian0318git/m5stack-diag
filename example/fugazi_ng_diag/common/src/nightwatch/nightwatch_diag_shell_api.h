/* $Id: nightwatch_diag_shell_api.h,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/nightwatch_diag_shell_api.h,v $
 *------------------------------------------------------------------
 *
 * nightwatch_diag_shell_api.h: header file for Nightwatch Diag shell api.
 *
 * May. 2018 - Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 */

#ifndef _NIGHTWATCH_DIAG_SHELL_API_H_
#define _NIGHTWATCH_DIAG_SHELL_API_H_

#define NWK_BP_MODE_1G      0x1
#define NWK_BP_MODE_10GKR   0x2
#define NWK_BP_MODE_XAUI    0x4
#define NGIO_GE_PNUM        3

struct _nightwatch_shell_config_t {
    int pcie_hotplug_delay;
    int slot;
    int id;
    struct _bp_ports_cfg {
        int port[NGIO_GE_PNUM];
        int ability[NGIO_GE_PNUM];
        int speed[NGIO_GE_PNUM];
        int portm[NGIO_GE_PNUM];
    }bp_cap;
    uint32_t ge_port_pkts[NGIO_GE_PNUM];
    uint32_t ge_port_burst[NGIO_GE_PNUM];
    char serial[20];
    char nwk_main_prompt[32];
    char *nwk_shell_prompt[20];
    char *nwk_cmd_exit[5];
};

extern int exec_nwk_shell_cmd (int, char **, boolean, boolean);
extern void nwk_diag_shell_cfg_init(int, int, int, char* serial);
extern struct _nightwatch_shell_config_t *nwk_get_shell_cfg(void);
#endif

/*-------------------------------------------------
$Log: nightwatch_diag_shell_api.h,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2019/06/11 06:32:40  mingding
CSCvk64124-30: Integrate stardust to ISR menu test

    - Pass DIAGFLAG to stardust from ngdiag for test usage
    - Receive errors count from stardust
    - Report errors for bp test failure on O2/Utah/Neptune
    - Report errors while running test from menu 'Nighwatch SM Test'

Revision 1.1.2.1  2019/05/30 05:33:34  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
