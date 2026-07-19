/* $Id: gpio_ioctl.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/gpio_ioctl.h,v $
 */
#ifndef GPIO_IOCTL_DEV_H
#define GPIO_IOCTL_DEV_H

/*
  Copyright (c) 1985-2016 by Cisco Systems, Inc.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/*****************************************************************************/
/*  This is the GPIO IOCTL driver for Nuova Systems BMC platform             */
/*  using the Pilot_II ARM-9 device.                                         */
/*****************************************************************************/

/* 
 * This file defines structure etc. used by both the device driver and applications
 * wishing to make gpio ioctl calls
 */


/*
 * IOCTL descriptor.
 *    char *gpio_name       -  name of GPIO to operate on
 *    unsigned int data    -  Data to read or data returned on write
 */
typedef struct {
  char *gpio_name;
  unsigned int data;
} gpio_ioctl_descriptor;

/* 
 * IOCTL argument
 *    unsigned int numEntries              - Number of entries in the list
 *    gpio_ioctl_descriptor *descriptors   - Array of numEntries descriptors
 */
typedef struct {
  unsigned int numEntries;
  gpio_ioctl_descriptor *descriptors;
} gpio_ioctl_list;

#define GPIO_IOC_MAGIC 'g'
#define GPIO_IOR    _IOR(GPIO_IOC_MAGIC, 1, gpio_ioctl_list *)
#define GPIO_IOW    _IOW(GPIO_IOC_MAGIC, 2, gpio_ioctl_list *)




#endif  // Guard
