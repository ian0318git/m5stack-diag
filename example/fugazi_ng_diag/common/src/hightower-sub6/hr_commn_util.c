/* $Id: hr_commn_util.c,v 1.2 2021/06/02 02:56:23 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-sub6/hr_commn_util.c,v $
 *********************************************************************
 *
 * hr_commn_util.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include <signal.h>
#include "hr_commn_util.h"
int _hr_commn_util_log_lvl = LOG_LVL_WRN;

static char **force_power_cycle_prompts = NULL;
static void copy_force_power_cycle_prompts(char *prompts[])
{
    int i = 0;
    int N = 0;

    for(i = 0; force_power_cycle_prompts && force_power_cycle_prompts[i]; i++)
        free(force_power_cycle_prompts[i]);

    if (force_power_cycle_prompts)
        free(force_power_cycle_prompts);

    force_power_cycle_prompts = NULL;

    for(N = 0; prompts && prompts[N]; N++);
    if (N == 0)
        return;

    force_power_cycle_prompts = malloc(sizeof(char *) * (N + 1));
    if (force_power_cycle_prompts) {
        memset(force_power_cycle_prompts, 0, sizeof(char *) * (N + 1));
    }

    for(i = 0; force_power_cycle_prompts && prompts && prompts[i]; i++) {
        force_power_cycle_prompts[i] = malloc(strlen(prompts[i]) + 32);
        if (force_power_cycle_prompts[i]) {
            memset(force_power_cycle_prompts[i], 0, strlen(prompts[i]) + 32);
            memcpy(force_power_cycle_prompts[i], prompts[i], strlen(prompts[i]));
        } else {
            force_power_cycle_prompts[i] = " ";
        }
    }
    force_power_cycle_prompts[N] = NULL;

    return;
}

static void show_force_power_cycle_prompts(void)
{
    int i = 0;
    for(i = 0; force_power_cycle_prompts && force_power_cycle_prompts[i]; i++) {
        printf("%s\n", force_power_cycle_prompts[i]);
    }
    printf("\nPlease power-cycle (cold reboot) the board.\n");
}


static void force_do_power_cycle_sigh(int sig)
{
    printf("\n");
    show_force_power_cycle_prompts();
}

/*******************************************************************************
 *
 * Function   : force_user_do_power_cycle
 * Description: Request powercycle and catch all inpts/signals.
 * Inputs     : Prompts array to show to user.
 * Outputs    : None
 *
 *******************************************************************************
 */
void force_user_do_power_cycle(char *prompts[])
{
    /* Forcely request power cycle */
    struct sigaction sa;
    fd_set fds;
    struct timeval to;

    copy_force_power_cycle_prompts(prompts);

    /* Catch all signals */
    sa.sa_handler = force_do_power_cycle_sigh;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT , &sa, NULL);
    sigaction(SIGHUP , &sa, NULL);
    sigaction(SIGINT , &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGSTOP, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);

    /* Catch all input */
    while(1) {
        printf("\n");
        show_force_power_cycle_prompts();
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        to.tv_sec = 3;
        to.tv_usec= 0;
        select(1, &fds, NULL, NULL, &to);
        if (FD_ISSET(STDIN_FILENO, &fds))
            _DRAIN_STDIN();
    }
}

/*********************************************************************
 * $Log: hr_commn_util.c,v $
 * Revision 1.2  2021/06/02 02:56:23  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/08/27 07:18:46  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

