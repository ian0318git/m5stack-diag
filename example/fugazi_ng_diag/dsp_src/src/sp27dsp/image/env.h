/* $Id: env.h,v 1.2 2016/10/07 17:56:16 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/env.h,v $
 *------------------------------------------------------------------
 * env.h
 *      environment variables for SP2704
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*------------------------------------------------------------------
 * env.h - SP2704 environment variables
 *
 * Copyright (c) 2012 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#ifndef __ENV_H__
#define __ENV_H__

#define ENV_MAX_ENV   64       // max number of environment variables
#define ENV_MAX_NAME  32
#define ENV_MAX_VALUE 128

typedef struct {
    char name[ENV_MAX_NAME];
    char value[ENV_MAX_VALUE];
} env_entry_t;

typedef struct {
    uint32_t sum;
    env_entry_t env_db[ENV_MAX_ENV];
} boot_environment_t;

typedef struct {
    uint32_t sum;
    char env[(ENV_MAX_NAME + 1 + ENV_MAX_VALUE + 1) * ENV_MAX_ENV];
} packed_boot_environment_t;

#if defined(ARM) || defined(__linux__)
void env_init(void);
#endif
uint32_t env_get_string(char *name, char **value);
uint32_t env_get_int(char *name, uint32_t *value);
uint32_t env_get_ip(char *name, uint32_t *ip);
uint32_t env_get_mac(char *name, uint8_t *mac);

uint32_t env_set_string(char *name, char *value);
uint32_t env_set_pid(char *value);
uint32_t env_set_int(char *name, uint32_t value, uint32_t base);
uint32_t env_set_ip(char *name, uint32_t ip);
uint32_t env_set_mac(char *name, uint8_t *mac);

uint32_t env_unset(char *name);

uint32_t env_sync(int);
uint32_t env_get(void);
uint32_t env_iter_init(void);
uint32_t env_iter_next(uint32_t *iter, char **name, char **value);
void env_clear(void);
int env_is_empty(void);

#ifdef __linux__
void *env_get_raw(uint32_t *size);
#endif

#ifdef _SP27XX_
extern boot_environment_t *boot_environment_p;
#endif

#endif

/******** History ********
$Log: env.h,v $
Revision 1.2  2016/10/07 17:56:16  srane
CSCvb61570 - Move to SWIMS server for code signing

Revision 1.1  2012/06/28 13:33:09  srane
New boot loader requirements - environment variables, unique mgaic
number for SP2704 (will boot only 2704), SSP support.


$Endlog$
*/

