/* $Id: intel_gpio.h,v 1.1 2020/01/09 01:02:00 jiajliu Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_2RU/intel_gpio.h,v $
 *------------------------------------------------------------------
 *
 * intel_p2sb.c - Intel P2SB GPIO interface
 *
 * Dec. 2018, Jiajia Liu <jiajliu@cisco.com>
 *
 * Copyright (c) 2018 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __INTEL_P2SB_H__
#define __INTEL_P2SB_H__

#define GPIO_OUTPUT         0
#define GPIO_IN             1

extern int intel_p2sb_get_bar(uint64_t *base_addr);
void *intel_gpio_ioremap(unsigned int community);
void intel_gpio_iounmap(void *addr);

int intel_gpio_request_enable(void *community, int offset);
void intel_gpio_set_direction(void *community, int offset, int input);
void intel_gpio_set(void *community, int offset, uint32_t value);
int intel_gpio_get(void *community, int offset);
void intel_gpio_dbg_show(void *community, int offset);

#endif

/*
 *-----------------------------------------------------------------------------
$Log: intel_gpio.h,v $
Revision 1.1  2020/01/09 01:02:00  jiajliu
Merge Curie 2RU to main trunk

$Endlog$
 *-----------------------------------------------------------------------------
 */
