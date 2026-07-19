/* $Id: nightwatch_diag_shell_api.c,v 1.2 2019/08/06 06:56:09 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/nightwatch/nightwatch_diag_shell_api.c,v $
 *------------------------------------------------------------------
 *
 * nightwatch_diag_shell_api.c: for Nightwatch Diag shell api.
 *
 * May. 2018 - Mingchun Ding
 *
 * Copyright (c) 2019 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "types.h"
#include "pty.h"
#include "nightwatch_diag_shell_api.h"
#include "common.h"
#include "nvmonvars.h"
#include "proto.h"
#include "plat_defs.h"
#include "cross_platform.h"
#include "dash_fpga.h"

#define TRUE 1
//#define NWK_TIMESTAMP_ENABLE
extern long nightwatch_boot_image(int);

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct _nightwatch_shell_config_t nwk_diag_shell_cfg_default = {
    .pcie_hotplug_delay = 8,
    .bp_cap = {
        .port    = {23, 24, 23},
        .speed   = {NWK_BP_MODE_10GKR, NWK_BP_MODE_10GKR, NWK_BP_MODE_1G},
        .ability = {NWK_BP_MODE_10GKR|NWK_BP_MODE_1G, NWK_BP_MODE_10GKR|NWK_BP_MODE_1G, NWK_BP_MODE_1G},
        /* NWK GE0(23/51) <-> Host GE0(KR)/GE2(XAUI)
         * NWK GE1(24/52) <-> Host GE1
         */
        .portm   = {NGIO_GE0, NGIO_GE1, NGIO_GE0}
    },
#if !defined(STRESS_NGSM_ETH_TRAFFIC)
    .ge_port_pkts = {300, 300, 300},
#else
    .ge_port_pkts = {10000, 10000, 10000},
#endif
    .ge_port_burst = {4294967295, 10000, 10000},
    .nwk_cmd_exit = {
        "exit",
        "yes",
        NULL
    },
    .nwk_shell_prompt = {
        nwk_diag_shell_cfg_default.nwk_main_prompt,
        "Diag> ",
        "Traf> ",
        " [Yes] ",
        "Monitor]  ",
        "StarDust> ",
        NULL
    }
};
static struct _nightwatch_shell_config_t nwk_diag_shell_cfg;

struct _nightwatch_shell_config_t *nwk_get_shell_cfg()
{
    return &nwk_diag_shell_cfg;
}

static int nwk_match_prompt(char *str, int len)
{
    int i;
    char **prompt = nwk_diag_shell_cfg.nwk_shell_prompt;
    int prompt_len;

    for (i = 0; prompt[i]; i++) {
        prompt_len = strlen(prompt[i]);
        if (!prompt_len)
            continue;

        if (len >= prompt_len &&
                !strncmp(str+len-prompt_len, prompt[i], prompt_len))
            return i;
    }

    return -1;
}

static int echo_ptmin_to_stdout(char *std_input, int *stdin_len, char *ptm_input, int ptm_len, boolean silent)
{
    static int line = 0;
    int off = 0;
    int std_len = *stdin_len;

    /* do NOT echo input from STDIN */
    if (std_len > 1 &&
            ptm_len >= std_len+1 &&
            !strncmp(std_input,ptm_input,std_len-1) &&
            ptm_input[std_len-1] == '\r') {
        /* stdin ends with '\n'
         * ptmin ends with '\r\n'
         */
        off = std_len+1;
        *stdin_len = 0;
    } else if (std_len > 0 &&
            std_len >= ptm_len &&
            !strncmp(std_input, ptm_input, std_len)) {
        /* stdin ends with '\n'
         * ptmin ends with '\n'
         */
        off = std_len;
        *stdin_len = 0;
    } else if (std_len == 1 && ptm_len == 2) {
        *stdin_len = 0;
        return 0;
    } else if (std_len == 1 && ptm_len == 4) {
        *stdin_len = 0;
        ptm_len = 2;
    }
    ptm_len -= off;
    if (ptm_len == 0)
        return 0;

    if (!silent) {
#if defined(NWK_TIMESTAMP_ENABLE)
        if (DIAGFLAG & D_VERBOSE) {
            char timestamp[64];
            for (; off < ptm_len; off++) {
                if (ptm_input[off]=='\r')
                    off++;
                else if (ptm_input[off]!='\n')
                    break;
                sprintf(timestamp, "+%.8d: \n", line++);
                write(1, timestamp, strlen(timestamp));
            }

            if (ptm_len > off) {
                sprintf(timestamp, "%c%.8d: ", off?'+':'-', line++);
                write(1, timestamp, strlen(timestamp));
                write(1, ptm_input+off, ptm_len-off);
            }
        }
        else
#endif
        {
            line++;
            write(1, ptm_input+off, ptm_len);
        }
    } else
        return 1;

    return line;
}

int exec_nwk_shell_cmd(int flag, char **nwk_cmd, boolean silent, boolean quit)
{
    const int maxlen = 128;
    char cmd[maxlen];
    static int child_pid = 0, ptm_fd = 0;
    fd_set fd_in;
    int ptm_running = 1;
    char input[1024] = {'\n', '\0'};
    char ptm_input[8192];
    int len, ptm_len=0, std_len=0;
    int cmd_idx = 0;
    int matched_prompt = -1;

    if (!silent)
        printf("\n\n ### NOTE: Type exit followed by <Enter> "
                "to switch back to host's console\n\n");

    if (!ptm_fd) {
        if (nwk_cmd && !strncasecmp(nwk_cmd[0], "exit", 4))
            return (TRUE);

        if(nightwatch_boot_image(0) == FAILED) {
            cterr('f', 0, "Archive NIGHTWATCH firmware failed.");
            return (FAILED);
        } else {
            printf("\n ### Archive Firmware Image Successfully\n");
        }

        snprintf(cmd, maxlen-1, "./nwkdiag 0x%x 0x%x %s", flag,
                nwk_diag_shell_cfg.id, nwk_diag_shell_cfg.serial);

        pty_exec(cmd, &child_pid, &ptm_fd);
    } else {
        /* send <enter> to show nwk shell prompt */
        std_len = 1;
        input[0]='\n';
        write(ptm_fd, "\n", std_len);
    }

    while (ptm_running) {
        FD_ZERO(&fd_in);
        FD_SET(0, &fd_in);
        FD_SET(ptm_fd, &fd_in);
        switch(select(ptm_fd + 1, &fd_in, NULL, NULL, NULL)) {
        case -1:
            printf("Exit from nwk Nightwatch Disgnostic with %d\n", errno);
            ptm_running = 0;
            break;
        default:
            if (FD_ISSET(0, &fd_in)) {
                len = read(0, input, sizeof(input));
                if (len > 0) {
                    if (!strncasecmp(input, "quit", 4)) {
                        return (TRUE);
                    }

                    write(ptm_fd, input, len);
                    std_len = len;
                }
            }

            if (FD_ISSET(ptm_fd, &fd_in)) {
                ptm_len += read(ptm_fd, ptm_input+ptm_len, sizeof(ptm_input)-ptm_len);
                if (ptm_len < 0) {
                    /* Slave exit */
                    ptm_running = 0;
                    break;
                } else if (ptm_len == 0) {
                    break;
                }

                matched_prompt = nwk_match_prompt(ptm_input, ptm_len);
                if (ptm_input[ptm_len-1] != '\n' && matched_prompt < 0) {
                    /* continue read from ptm */
                    break;
                }

                echo_ptmin_to_stdout(input, &std_len, ptm_input, ptm_len, silent);
                ptm_len = 0;

                if (matched_prompt < 0) {
                    break;
                }

                /* Ready for input after nwk shell prompt */
                if (nwk_cmd && nwk_cmd[cmd_idx]) {
                    std_len = strlen(nwk_cmd[cmd_idx])+1;
                    sprintf(input, "%s\n", nwk_cmd[cmd_idx]);
                    if (!silent)
                        write(1, input, std_len);
                    write(ptm_fd, input, std_len);
                    msleep(100); /* delay 100ms to receive enough data */
                    cmd_idx++;
                } else if (cmd_idx > 0 && quit) {
                    if (!silent)
                        printf("\n"); /* new line for Master's output */
                    return (TRUE);
                } else if (4 == matched_prompt) {
                    /* Monitor mode to accept one key control */
                    /*
                    input[0] = getch();
                    write(ptm_fd, input, 1);
                    */
                }
            }
            break;
        }
    }

    close(ptm_fd);
    ptm_fd = 0;

    return (TRUE);
}

void nwk_diag_shell_cfg_init(int slot, int port_cnt, int id, char *serial)
{
    uint32_t host_10gkr_cap;

    switch (port_cnt) {
    case 48:
        nwk_diag_shell_cfg_default.bp_cap.port[0] = 51;
        nwk_diag_shell_cfg_default.bp_cap.port[1] = 52;
        nwk_diag_shell_cfg_default.bp_cap.port[2] = 51;
        break;
    case 20:
    default:
        nwk_diag_shell_cfg_default.bp_cap.port[0] = 23;
        nwk_diag_shell_cfg_default.bp_cap.port[1] = 24;
        nwk_diag_shell_cfg_default.bp_cap.port[2] = 23;
        break;
    }
    memset (nwk_diag_shell_cfg_default.nwk_main_prompt, 0,
            sizeof(nwk_diag_shell_cfg_default.nwk_main_prompt));
    snprintf(nwk_diag_shell_cfg_default.nwk_main_prompt,
            sizeof(nwk_diag_shell_cfg_default.nwk_main_prompt)-1,
            "Nightwatch%d> ", port_cnt);

    nwk_diag_shell_cfg_default.slot = slot;
    nwk_diag_shell_cfg_default.id   = id;

    memset (nwk_diag_shell_cfg_default.serial, 0,
            sizeof(nwk_diag_shell_cfg_default.serial));
    strncpy(nwk_diag_shell_cfg_default.serial, serial,
            sizeof(nwk_diag_shell_cfg_default.serial)-1);

    host_10gkr_cap = host_ngio_10gkr_capability(SM_MODULE, slot);

    /* Check BP GE0 mode */
    if (!(host_10gkr_cap & 0x1)) {
        nwk_diag_shell_cfg_default.bp_cap.portm[0]   = NGIO_XAUI;
        /* NGIO GE2 (XAUI) for Overlord and legacy Utah
         * NGIO GE0 is used for other platform
         * */
        if (is_overlord() || is_utah() || is_sword()) {
            nwk_diag_shell_cfg_default.bp_cap.ability[0] = NWK_BP_MODE_XAUI;
            nwk_diag_shell_cfg_default.bp_cap.speed[0]   = NWK_BP_MODE_XAUI;
        } else {
            nwk_diag_shell_cfg_default.bp_cap.ability[0] = NWK_BP_MODE_1G;
            nwk_diag_shell_cfg_default.bp_cap.speed[0]   = NWK_BP_MODE_1G;
        }
    }

    /* Check BP GE1 mode */
    if (!(host_10gkr_cap & 0x2)) {
        nwk_diag_shell_cfg_default.bp_cap.ability[1] = NWK_BP_MODE_1G;
    }

    nwk_diag_shell_cfg = nwk_diag_shell_cfg_default;

    nwk_diag_shell_cfg.nwk_shell_prompt[0] = nwk_diag_shell_cfg.nwk_main_prompt;

    memset (nwk_diag_shell_cfg.nwk_main_prompt, 0,
            sizeof(nwk_diag_shell_cfg.nwk_main_prompt));
    strncpy(nwk_diag_shell_cfg.nwk_main_prompt,
            nwk_diag_shell_cfg_default.nwk_main_prompt,
            sizeof(nwk_diag_shell_cfg.nwk_main_prompt)-1);
}

/*-------------------------------------------------
$Log: nightwatch_diag_shell_api.c,v $
Revision 1.2  2019/08/06 06:56:09  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.1.2.2  2019/06/11 06:32:40  mingding
CSCvk64124-30: Integrate stardust to ISR menu test

    - Pass DIAGFLAG to stardust from ngdiag for test usage
    - Receive errors count from stardust
    - Report errors for bp test failure on O2/Utah/Neptune
    - Report errors while running test from menu 'Nighwatch SM Test'

Revision 1.1.2.1  2019/05/30 05:33:33  mingding
CSCvk64124-29: Support PCIe-based Nightwatch Server Module

*/
