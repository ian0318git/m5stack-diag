/* $Id: dev_object.h,v 1.3 2019/01/10 06:05:36 wilbhuan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/chips/dev_object.h,v $
 *------------------------------------------------------------------
 *
 * FILE NAME: Common Device Driver
 *
 * Nov 2005 - Anh Dang 
 *
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/*
 *------------------------------------------------------------------
 * dev_object.h - Common Device Driver Object Definition.
 *
 *    This file contains the definitions to describe a generic
 *    device driver object.
 *
 *    The device object can be used by any device type (a system
 *    controller, an FPGA or ASIC, a network interface framer, etc.)
 *    and embedded into any OS that provides POSIX type definitions.
 *
 *    Please refer to ENG-188171 for a complete description of the
 *    definitions, functions, and information below; This document
 *    describes how to use this device object in the construction
 *    of a specific device driver.
 *
 *  January 2002, Robert E. Coulson
 *
 *  Copyright (c) 2007-2019 by Cisco Systems, Inc.
 *  All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef _DEV_OBJECT_H_
#define _DEV_OBJECT_H_

#include "dev_print.h"
#include <assert.h>

/*
** 32 Bit field for various device debug flags
** For device flags, if the respective bit is set to '1', then
** the assigned flag is TRUE.
*/
#define DEVICE_FLAG_DEBUG_EVENTS      0x00000001
#define DEVICE_FLAG_DEBUG_ERRORS      0x00000002
#define DEVICE_FLAG_DEBUG_INTRS       0x00000004
#define DEVICE_FLAG_DEBUG_TIMERS      0x00000008
#define DEVICE_FLAG_DEBUG_DATA        0x00000010
#define DEVICE_FLAG_DEBUG_IO          0x00000020
#define DEVICE_FLAG_DEBUG_CMD         0x00000040
#define DEVICE_FLAG_DEBUG_RSVD_1      0x00000080
#define DEVICE_FLAG_DEBUG             (DEVICE_FLAG_DEBUG_EVENTS | \
                                       DEVICE_FLAG_DEBUG_ERRORS | \
                                       DEVICE_FLAG_DEBUG_INTRS | \
                                       DEVICE_FLAG_DEBUG_TIMERS | \
                                       DEVICE_FLAG_DEBUG_DATA | \
                                       DEVICE_FLAG_DEBUG_IO | \
                                       DEVICE_FLAG_DEBUG_CMD)
                                       

/*
** Device error report status, use the upper 16 bits to communicate in
** dev_error_report () function.

** WARNING - use cterr('w',.....
** RETRY   - Do not use cterr(...) because Host is in control for final cterr,
**           so the device does not want to report cterr.
** Default - use cterr('f',.....

** WARNING and FATAL are the two common error report have been used so far.
** RETRY is a new option introduced in here and it is up to Developer to handle
** how to report this scenario if the HW really need to do the RETRY before 
** giving up ... 
*/
typedef enum dev_err_report_ {
    WARNING = 0x00010000,
    RETRY   = 0x00020000,
    FATAL   = 0xFFFF0000,
} dev_err_report;

/*
** Device state information definitions
**
** While a device is operating, the device state will change. The device
** state can be tracked by using the values in this enumerated list.
*/
typedef enum dev_state_e_ {
    DEV_STATE_CREATE = 0,
    DEV_STATE_ATTACH,
    DEV_STATE_DETACH,
    DEV_STATE_INIT,
    DEV_STATE_ENABLE_OP,
    DEV_STATE_DISABLE_OP,
    DEV_STATE_COMMON_END = DEV_STATE_DISABLE_OP,
} dev_state_e;

/*
** Device show command definitions 
**
** When a device uses the dev_show(), a show command is given to
** define the type of device information to display; These
** common device show command definitions are shown below... 
*/
typedef uint32 dev_show_cmd;
typedef enum dev_show_cmd_e_ {
    DEV_SHOW_ALL = 0,
    DEV_SHOW_BRIEF = 1,
    DEV_SHOW_CONFIG = 2,
    DEV_SHOW_ERRORS = 3,
    DEV_SHOW_REGISTERS = 4,
    DEV_SHOW_STATUS = 5,
    DEV_SHOW_COUNTERS_ALL = 6,
    DEV_SHOW_COUNTERS_DROPS_ALL = 7,
    DEV_SHOW_COUNTERS_DROPS_RX = 8,
    DEV_SHOW_COUNTERS_DROPS_TX = 9,
    DEV_SHOW_COMMON_END  = DEV_SHOW_COUNTERS_DROPS_TX 
} dev_show_cmd_e;


/*
** Define a function vector prototype for devices to use when collecting
** device information when a platform/OS crash condition is present.
*/
typedef struct dev_object_t_ dev_object_t;

/*
** Callout function vector prototype for device to host error reporting.
*/
typedef void (*dev_error_report_t)(dev_object_t *, char *, uint32);

/*
** Function table for device object
*/
typedef struct dev_object_fvt_t_{

    /*
     * Attach or Detach the device to the caller (eg., platform).
     */
    uint32 (*dev_attach)(dev_object_t *);
    uint32 (*dev_detach)(dev_object_t *);

    /*
     * Check whether device re-configuration is needed during initialization.
     * Based on this information and possibly other factors,
     * the caller will decide whether to invoke dev_restart or dev_init.
     */
    uint32 (*dev_reconfig_needed)(dev_object_t *, void *, boolean *);

    /*
     * Restart the device.
     */
    uint32 (*dev_restart)(dev_object_t *);

    /*
     * Initialize the device.
     */
    uint32 (*dev_init)(dev_object_t *);

    /*
     * Enable or Disable ALL device operation.
     */
    uint32 (*dev_oper_enable)(dev_object_t *);
    uint32 (*dev_oper_disable)(dev_object_t *);

    /*
     * Enable or Disable ALL device interrupt sources.
     */
    uint32 (*dev_intr_enable)(dev_object_t *);
    uint32 (*dev_intr_disable)(dev_object_t *);

    /*
     * Device Interrupt service routine.
     */
    uint32 (*dev_isr)(dev_object_t *);

    /*
     * Device Information (show) routine.
     */
    uint32 (*dev_show)(dev_object_t *, print_fn_t, dev_show_cmd);

    /*
     * Device Error Reporting routine.  (This is a callout vector.)
     */
    dev_error_report_t dev_error_report;

    /*
     * Device crashinfo collection
     */
    uint32 (*dev_collect_crashinfo)(dev_object_t *, print_fn_t, dev_show_cmd);

    /*
     * Destroy the device.
     */
    void (*dev_destroy)(dev_object_t **);

    /*
     * If necessary to extend this table, do it above this member;
     *
     * String Name for this device; The device name member, dev_name, is
     * meant to be the last member in this fuction vector table...
     */
    const char *dev_name;

}dev_object_fvt_t;



/*
** Convenience macros for calling well known dev object function vectors.
*/
#define DEV_ATTACH(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_attach((dev_object_t *)(d))

#define DEV_DETACH(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_detach((dev_object_t *)(d))

#define DEV_RECONFIG_NEEDED(d, x, y) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_reconfig_needed((dev_object_t *)(d), (x), (y))

#define DEV_RESTART(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_restart((dev_object_t *)(d))

#define DEV_INIT(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_init((dev_object_t *)(d))

#define DEV_OPER_ENABLE(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_oper_enable((dev_object_t *)(d))

#define DEV_OPER_DISABLE(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_oper_disable((dev_object_t *)(d))

#define DEV_INTR_ENABLE(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_intr_enable((dev_object_t *)(d))

#define DEV_INTR_DISABLE(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_intr_disable((dev_object_t *)(d))

#define DEV_ISR(d) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_isr((dev_object_t *)(d))

#define DEV_SHOW(d, x, y) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_show((dev_object_t *)(d), (x), (y))

#define DEV_ERROR_REPORT(d, x, y) ((dev_object_t *)(d))->dev_object_fvt-> \
    dev_error_report((dev_object_t *)(d), (x), (y))

#define DEV_COLLECT_CRASHINFO(d, x, y) ((dev_object_t *)(d))->dev_object_fvt->\
    dev_collect_crashinfo((dev_object_t *)(d), (x), (y))

#define DEV_DESTROY(d) ((dev_object_t *)(*d))->dev_object_fvt-> \
    dev_destroy((dev_object_t **)(d))


/*
** Device Object structure
**
** This device object structure is meant to be independent of any platform,
** or control plane, or OS information...
**
** Furthermore, this common structure together with a device specific
** structure, will provide all of the information that devices require
** for proper operation.
**
** Please refer to ENG-188171 for specific details regarding the
** members of this structure.
*/

struct dev_object_t_ {

    dev_object_fvt_t *dev_object_fvt; /* Device function vector tables.*/
    void    *dev_addr;                /* Device Virtual Base Address   */
    uint32  hw_ver_id;                /* Device HW Version ID          */
    uint32  dev_flag;                 /* Device flag, as defined above.*/
    uint32  dev_state;                /* Device state info, as defined above*/
    void *client_context;             /* Client context handle.        */
    /* Human readable string specifying location of device in
     * system. For example, slot/subslot.
     */
    char    *dev_location;
};

/*
** This is the basic structure of a device with base as the first member.
**
*/

typedef struct dev_xx_object_t_ {
    dev_object_t base;
}dev_xx_object_t;

extern void init_default_dev_object(dev_object_t *dev, dev_object_fvt_t *);
extern uint32 err_report(dev_object_t *, char *, uint32);

#endif   /* _DEV_OBJECT_H_ */

/******** History ******** 
$Log: dev_object.h,v $
Revision 1.3  2019/01/10 06:05:36  wilbhuan
Made function "err_report" as an external function.

Revision 1.2  2012/03/28 00:38:06  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:01  ptong
Initial archive of ng_diag module


$Endlog$
*/
