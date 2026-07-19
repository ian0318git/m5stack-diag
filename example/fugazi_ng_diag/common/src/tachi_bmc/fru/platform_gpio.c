/* $Id: platform_gpio.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/platform_gpio.c,v $
 *
 *      File:   platform_gpio.c
 *      Name:   Qin Zou
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <string.h>
#include "gpio_ioctl.h"

#define BMC_GPIO_FILE "/dev/gpio_ioctl"

static char *platform_mezz_present[] =
{
	"mezz_p0",
	"mezz_p1",
	"mezz_present_n",
};

int platform_gpio_data_get(char *strname, unsigned int *data)
{
	FILE *fp = fopen(BMC_GPIO_FILE, "r");
	int rc=0;
	gpio_ioctl_list glist;
	gpio_ioctl_descriptor gdesc;
	char name[64];
	gdesc.gpio_name = name;

	glist.numEntries = 1;
	glist.descriptors = (gpio_ioctl_descriptor *) &gdesc;

	strcpy(gdesc.gpio_name, strname);
	glist.descriptors->data = -1;

	rc = ioctl(fileno(fp), GPIO_IOR, &glist);
	*data = glist.descriptors->data;

	fclose(fp);
	return rc;
}

int platform_gpio_data_set(char *strname, unsigned int data)
{
	FILE *fp = fopen(BMC_GPIO_FILE, "w");
	int rc=0;
	gpio_ioctl_list glist;
	gpio_ioctl_descriptor gdesc;
	char name[64];
	gdesc.gpio_name = name;

	glist.numEntries = 1;
	glist.descriptors = (gpio_ioctl_descriptor *)&gdesc;

	strcpy(glist.descriptors->gpio_name, strname);
	glist.descriptors->data = data;

	rc = ioctl(fileno(fp), GPIO_IOW, &glist);

	fclose(fp);
	return rc;
}

int platform_gpio_data_dump(char **gpio_name, unsigned int num_of_gpios)
{
	int	rc = 0;
	unsigned int data, cnt;

	for (cnt = 0; cnt < num_of_gpios; cnt++) {
		//printf("cnt=%d name=%s\n", cnt, gpio_name[cnt]);
		rc = platform_gpio_data_get(gpio_name[cnt], &data);
		if (rc) return (rc);

		printf("  %-32s : %d\n", gpio_name[cnt], data);
	}

	return (rc);
}

int platform_gpio_mezz_present_dump()
{
	return (platform_gpio_data_dump(platform_mezz_present, sizeof(platform_mezz_present)/sizeof(char*)));
}

int platform_gpio_mezz_present(int slot)
{
	unsigned int data;
	platform_gpio_data_get("fm_mezz_present", &data);
	return (data);
}

