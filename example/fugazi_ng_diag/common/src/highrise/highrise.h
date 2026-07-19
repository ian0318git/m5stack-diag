/*------------------------------------------------------------------
 *
 * highrise.h - specific APIs header file for Highrise platform
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef _HIGHRISE_H_
#define _HIGHRISE_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define HR_KERNEL_VER_4_4   0
#define HR_KERNEL_VER_4_14  1

static inline unsigned int highrise_kernel_ver(void)
{
    static unsigned int kver = ~0;
    const char *p = "/proc/sys/kernel/osrelease";
    FILE *fp = NULL;
    char buf[64] = {[0 ... sizeof(buf)-1] = 0};

    if (kver == (unsigned int)(~0)) {
        kver = 0;
        if (access(p, R_OK) == 0) {
            fp = fopen(p, "r");
            assert(fp != NULL);
            assert(fread(buf, 1, sizeof(buf) - 1, fp) > 0);
            if (strncmp(buf, "4.4", 3) == 0) {
                kver = HR_KERNEL_VER_4_4;
            }
            else if (strncmp(buf, "4.14", 4) == 0) {
                kver = HR_KERNEL_VER_4_14;
            }
            else {
                assert("Unknow kernel version" == NULL);
            }
        }
    }
    return kver;
}

static inline unsigned int highrise_gpio_base_offset(void)
{
    if (highrise_kernel_ver() == HR_KERNEL_VER_4_4)
        return 0;
    else if(highrise_kernel_ver() == HR_KERNEL_VER_4_14)
        return 12;
    else
        assert("Unknow kernel version" == NULL);
    return 0;
}

#define GPIO20 20
#define GPIO21 21
#define GPIO22 22
#define GPIO23 23
#define GPIO24 24
#define GPIO25 25
#define GPIO26 26
#define GPIO27 27
#define GPIO32 32
#define GPIO34 34
#define GPIO44 44
#define GPIO47 47
#define GPIO48 48
#define GPIO49 49
#define GPIO50 50
#define GPIO51 51
#define GPIO52 52
#define GPIO53 53
#define GPIO54 54
#define GPIO69 69
#define GPIO73 73
#define GPIO74 74
#define GPIO75 75
#define GPIO82 82

#define CP_MPP0  (GPIO20 + highrise_gpio_base_offset())
#define CP_MPP1  (GPIO21 + highrise_gpio_base_offset())
#define CP_MPP6  (GPIO26 + highrise_gpio_base_offset())
#define CP_MPP12 (GPIO32 + highrise_gpio_base_offset())
#define CP_MPP24 (GPIO44 + highrise_gpio_base_offset())
#define CP_MPP27 (GPIO47 + highrise_gpio_base_offset())
#define CP_MPP62 (GPIO82 + highrise_gpio_base_offset())
#define CP_MPP31 (GPIO51 + highrise_gpio_base_offset())
#define CP_MPP54 (GPIO74 + highrise_gpio_base_offset())

#define CPLD_CPU_INT_L                 CP_MPP0
#define USB_MUX_DEBUG_EN               CP_MPP1
#define THERM_CPU_INT_L                CP_MPP12
#define SIM0_DETECT_L                  CP_MPP27
#define SIM1_DETECT_L                  CP_MPP24
#define DDR4_CPU_ALERT_L               CP_MPP62
#define CPU_TO_CPLD_STATUS_0           CP_MPP31
#define CPU_TO_CPLD_STATUS_1           CP_MPP54


#define INFRA_ERR_HANDLE(msg, rc, terminate)                          \
{                                                                             \
    if (rc) {                                                                 \
        printf("Error: %s - %s line: %d\n", msg, __FUNCTION__, __LINE__);\
                                                                              \
        if (terminate == 1) {                                                 \
            return (-1);                                                      \
        }                                                                     \
    }                                                                         \
}


#endif

extern int highrise_init(void);
extern int highrise_reset_act2_chip(void);
extern int highrise_unreset_act2_chip(void);
