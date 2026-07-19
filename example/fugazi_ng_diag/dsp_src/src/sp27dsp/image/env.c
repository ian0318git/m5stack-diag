/* $Id: env.c,v 1.2 2016/10/07 17:55:18 srane Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/image/env.c,v $
 *------------------------------------------------------------------
 * env.c
 *      environment library for SP2704
 *
 * June 2012, Smita Rane
 *
 * Copyright (c) 2012-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*------------------------------------------------------------------
 * env.c - SP2704 environment variables
 *
 * Copyright (c) 2012-2013 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 *
 * Implement an environment variable library for SP2704
 *
 * All environment variables are stored as name, value pairs.  Both name
 * and value are ASCII strings.
 *
 * Convenience functions are provided to format a value as an interger,
 * IP address, or MAC address to make using environment variables easier.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#ifdef ARM
#include "boot.h"
#include "ssp.h"
#include "byteorder.h"
#ifdef ENET_UBL
#include "uart.h"
#else
#include "bspboard.h"
#endif /* ENET_UBL */
#elif defined(__linux__)
#define ETH_ADDR_L 6
#include <arpa/inet.h>
#else
#include "bspboard.h"
#endif /* ARM */

#include "env.h"

#if defined(ARM) || defined(__linux__)
// This must be placed on uncached memory since it will be accessed from DSS as well
boot_environment_t boot_environment __attribute__((section(".env_var")));
#ifdef ENET_UBL
packed_boot_environment_t packed_boot_environment __attribute__((section(".sysmem")));
#else
packed_boot_environment_t packed_boot_environment __attribute__((section(".armddr2")));
#endif // ENET_UBL

packed_boot_environment_t *packed_boot_environment_p = NULL;
#endif

boot_environment_t *boot_environment_p = NULL;

/* read or write environment variables from/to eeprom. */

/*
 * find_entry
 *
 * Find an entry by name in the environment database. If not found and create
 * is true, create an entry for name.
 */    
static env_entry_t *find_entry (char *name, int create) {
    int i;

    if (boot_environment_p == NULL) {
#if defined(_SP27XX_) && !defined(ENET_UBL)
        bsp_debug_printf("env.c: find_entry: NULL boot_environment_p!\n");
#endif
        return NULL;
    }
    if (strlen(name) >= ENV_MAX_NAME) {
        return NULL;
    }
    for (i = 0; i < ENV_MAX_ENV; ++i) {
        if (boot_environment_p->env_db[i].name[0] &&
            (strcmp(name, boot_environment_p->env_db[i].name) == 0)) {
            return boot_environment_p->env_db + i;
        }
    }
    if (!create) {
        return NULL;
    }
    for (i = 0; i < ENV_MAX_ENV; ++i) {
        if (boot_environment_p->env_db[i].name[0] == '\0') {
            strncpy(boot_environment_p->env_db[i].name, name, ENV_MAX_NAME);
            boot_environment_p->env_db[i].value[ENV_MAX_NAME-1] = '\0';
            return boot_environment_p->env_db + i;
        }
    }
    return NULL;
}

/*
 * env_get_string
 *
 * Get an environment variable value as a string
 */
uint32_t env_get_string (char *name, char **value) {
    env_entry_t *env;

    env = find_entry(name, 0);
    if (env == NULL) {
        return 0;
    }
    *value = env->value;
    return 1;
}

/*
 * env_get_int
 *
 * Get an environment variable value as an integer
 */
uint32_t env_get_int (char *name, uint32_t *value) {
    env_entry_t *env;

    #ifdef DEBUG_ENV
    bsp_debug_printf("Trying to get env %s\n", name);
    #endif
    env = find_entry(name, 0);
    if (env == NULL) {
        return 0;
    }
    *value = strtol(env->value, NULL, 0);
    #ifdef DEBUG_ENV
    bsp_debug_printf("Got value %d\n", *value);
    #endif
    return 1;
}

#if defined(ARM) || defined(__linux__)
#define MAX_IP_LEN 20
/*
 * string_to_ip
 *
 * convert a string in the form "10.95.48.23" to a 32 bit IP adress
 */
static uint32_t string_to_ip (char *ipstr) {
    uint32_t ret = 0;
    uint8_t tmp = 0, cnt = 0;
    
    while(1) {
        switch(*ipstr) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            tmp = tmp * 10 + (*ipstr - '0');
            break;
        case '.':
            ret = (ret << 8) | tmp;
            tmp = 0;
            break;
        case '\0':
            ret = (ret << 8) | tmp;
            return ret;
        default:
            break;
        }
        ++ipstr;
        if (cnt++ > MAX_IP_LEN) {
            // something is wrong
            break;
        }
    }
    return 0;
}

/*
 * env_get_ip
 *
 * Get an environment variable value as in IP address
 */
uint32_t env_get_ip (char *name, uint32_t *ip) {
    env_entry_t *env;

    env = find_entry(name, 0);
    if (env == NULL) {
        return 0;
    }
    *ip = string_to_ip(env->value);
    return 1;
}

/*
 * env_get_mac
 *
 * Get an environment variable that is a MAC address
 * in the form "00:11:22:33:44:55" and return as an array of 6 bytes
 */
uint32_t env_get_mac (char *name, uint8_t *mac) {
    env_entry_t *env;
    char *cp;
    int mac_i;

    memset(mac, 0, ETH_ADDR_L);
    env = find_entry(name, 0);
    if (env == NULL) {
        return 0;
    }
    cp = env->value;
    mac_i = 0;
    while(*cp) {
        char digits[3];

        if (*cp == ':') {
            ++cp;
        }
        if (isxdigit(*cp) && isxdigit(*(cp + 1))) {
            digits[0] = *cp++;
            digits[1] = *cp++;
            digits[2] = '\0';
            mac[mac_i] = strtoul(digits, NULL, 16);
            ++mac_i;
        } else {
            memset(mac, 0, ETH_ADDR_L);
            return 0;
        }
        if (mac_i == ETH_ADDR_L) {
            return 1;
        }
    }
    memset(mac, 0, ETH_ADDR_L);
    return 0;
}

/*
 * env_set_string
 *
 * Set an environment value to the given string value
 */
uint32_t env_set_string (char *name, char *value) {
    env_entry_t *env;

    if ((strlen(name) >= ENV_MAX_NAME) ||
        (strlen(value) >= ENV_MAX_VALUE)) {
        return 0;
    }
    // Make PID a read-only variable
    if (strncmp(name, "PID", ENV_MAX_NAME) == 0) {
#ifdef ENET_UBL
        uart_puts("\r\nPID is read-only!\r\n");
#elif defined(ARM)
        bsp_debug_printf("PID is read-only!\n");
#endif
        return 0;
    }
    env = find_entry(name, 1);
    if (env == NULL) {
        return 0;
    }
    strncpy(env->value, value, ENV_MAX_VALUE);
    env->value[ENV_MAX_VALUE-1] = '\0';
    return 1;
}


/*
 * env_set_pid
 *
 * Set the environment variable 'PID' to the given string value
 */
uint32_t env_set_pid (char *value) {
    env_entry_t *env;

    if ((strlen(value) >= ENV_MAX_VALUE)) {
        return 0;
    }
    env = find_entry("PID", 1);
    if (env == NULL) {
        return 0;
    }
    if (strncmp(env->value, value, ENV_MAX_VALUE) == 0) {
        return 0;
    }
    strncpy(env->value, value, ENV_MAX_VALUE);
    env->value[ENV_MAX_VALUE-1] = '\0';
    return 1;
}

/*
 * format_digit
 *
 * Take num and return a string representation in the requested base
 *
 */
#define DIGIT_BUFFER_SIZE ((2 * sizeof(uint32_t)) + 8)
static int format_digit (char *buf, uint32_t num,
                         int base, uint32_t *nchars) {
    /* Upper case hex digits. */
    static const char UCdigits[] = "0123456789ABCDEFX";
    const char *dc = UCdigits;

    /* Temp buffer big enough to hold largest formatted digit */
    char dbuf[DIGIT_BUFFER_SIZE], *db_end;

    db_end = &dbuf[DIGIT_BUFFER_SIZE - 1];
    *db_end = '\0';
    do {
        *--db_end = dc[(num % base)];
    } while((num /= base) && (db_end > dbuf) );
    /* Formatted digit starts at db_end */
    if (base == 16) {
        strcpy(buf, "0x");
        buf += 2;
    }
    strncpy(buf, db_end, ENV_MAX_VALUE);
    if (nchars) {
        *nchars = strlen(buf);
    }
    return 1;
}

/*
 * env_set_int
 *
 * Set an environment variable to the given numerical value and base
 */
uint32_t env_set_int (char *name, uint32_t value, uint32_t base) {
    env_entry_t *env;

    if (strlen(name) >= ENV_MAX_NAME) {
        return 0;
    }
    env = find_entry(name, 1);
    if (env == NULL) {
        return 0;
    }
    format_digit(env->value, value, base, NULL);
    return 1;
}

/*
 * env_set_ip
 *
 * Set an environment variable to an IP address in the form "10.95.48.55"
 */
uint32_t env_set_ip(char *name, uint32_t ip) {
    env_entry_t *env;
    int j;
    uint32_t size;

    if (strlen(name) >= ENV_MAX_NAME) {
        return 0;
    }
    env = find_entry(name, 0);
    if (env == NULL) {
        return 0;
    }
    j = 0;
    format_digit(env->value + j, (ip >> 24) & 0xFF, 10, &size);
    j += size;
    format_digit(env->value + j, (ip >> 16) & 0xFF, 10, &size);
    j += size;
    format_digit(env->value + j, (ip >> 8) & 0xFF, 10, &size);
    j += size;
    format_digit(env->value + j, ip & 0xFF, 10, &size);
    j += size;
    env->value[j] = '\0';

    return 1;
}

/*
 * env_set_mac
 *
 * Set an environment variable to a MAC address in the form "00:11:22:33:44:55"
 */
uint32_t env_set_mac (char *name, uint8_t *mac) {
    env_entry_t *env;
    int i;
    char *cp;

    if (strlen(name) >= ENV_MAX_NAME) {
        return 0;
    }
    env = find_entry(name, 1);
    if (env == NULL) {
        return 0;
    }
    cp = env->value;
    for (i = 0; i < ETH_ADDR_L; ++i) {
        format_digit(cp, mac[i], 16, NULL);
        cp +=2;
        *cp = ':';
    }
    *cp = '\0';
    return 1;
}

/*
 * env_unset
 *
 * Remove an environment variable from the database
 */
uint32_t env_unset (char *name) {
    env_entry_t *env;

    env = find_entry(name, 0);
    if (env) {
        memset(env, 0, sizeof(*env));
        return 1;
    }
    return 0;
}

/*
 * compute_sum
 *
 * return the checksum of all significant bytes in the environment database
 */
static uint32_t compute_sum (env_entry_t *env_db) {
    int i;
    uint32_t sum = 0;

    for (i = 0; i < ENV_MAX_ENV; ++i) {
        if (env_db[i].name[0]) {
            unsigned char *cp;

            cp = (unsigned char *)(env_db[i].name);
            while (*cp) {
                sum += *cp++;
            }
            cp = (unsigned char *)(env_db[i].value);
            while (*cp) {
                sum += *cp++;
            }
        }
    }
    return htonl(sum);
}

static int env_addr_valid = 0;
#ifdef ARM
/* 
 * env_eeprom_read_write
 * 
 * read or write environment variables from/to eeprom. 
 */
static unsigned int env_addr = 0;
static void env_eeprom_read_write(int is_read)
{
    unsigned char tmp_buff[2];
    /* Note that it is assumes that the first access to
     * environment eeprom is a read operation.
     */
    if (!env_addr_valid) {
        /* read the map revision and type */
        eeprom_read(SPI_VERSION_OFFSET, (void *) tmp_buff, 2);
        if (tmp_buff[0] == 0 ||
            tmp_buff[1] == 1) {
            /* either older eeprom map, or is SDB. Either case,
             * configuration is located at 0x10000.
             */
            env_addr = SDB_CFG_REMAP_OFFSET;
        } else {
            env_addr = SPI_CONFIG_OFFSET;
        }
        env_addr_valid = 1;
    }

    if (is_read) {
#ifdef ENET_UBL
        uart_puts("\r\nReading env from ");
        uart_put_long(env_addr, 16);
        uart_puts("\r\n");
#else
        bsp_debug_printf("Reading env from 0x%x\n", env_addr);
#endif
        eeprom_read(env_addr, (void *)packed_boot_environment_p,
                    sizeof(packed_boot_environment));
    } else {
#ifdef ENET_UBL
        uart_puts("\r\nWriting env to ");
        uart_put_long(env_addr, 16);
        uart_puts("\r\n");
#else
        bsp_debug_printf("Environment can not be written from application\n", env_addr);
        return;
        
#endif
        eeprom_write(env_addr, (void *)packed_boot_environment_p,
                     sizeof(packed_boot_environment));
    }
}
#endif

/*
 * env_sync
 *
 * Convert the environment database to its packed format, and
 * write to EEPROM (if present)
 */
uint32_t env_sync (int refresh_addr) {
    int i;
    char *tp;
    char *sp;
    uint32_t size;

    if (refresh_addr) {
        env_addr_valid = 0;
    }

    memset(packed_boot_environment_p, 0, sizeof(packed_boot_environment));
    packed_boot_environment_p->sum = compute_sum(boot_environment_p->env_db);
    tp = packed_boot_environment_p->env;
    for (i = 0; i < ENV_MAX_ENV; ++i) {
        if (boot_environment_p->env_db[i].name[0]) {
            sp = boot_environment_p->env_db[i].name;
            while (*sp) {
                *tp++ = *sp++;
            }
            *tp++ = '\0';
            sp = boot_environment_p->env_db[i].value;
            while (*sp) {
                *tp++ = *sp++;
            }
            *tp++ = '\0';
        }
    }
    *tp++ = '\0';

    size = tp - packed_boot_environment_p->env + sizeof(packed_boot_environment_p->sum);
#ifdef ARM    
    env_eeprom_read_write(0);
#endif
    return 1;
}

void env_init(void)
{
    // initialize the pointers
    packed_boot_environment_p = &packed_boot_environment;
    boot_environment_p = &boot_environment;
}

/*
 * env_get
 *
 * Read the packed environment from EEPROM and expand it into the in-memory
 * representation
 */
uint32_t env_get (void) {
    uint32_t sum;
    char *sp, *tp;
    int i;

    env_init();
#ifdef ARM
    env_eeprom_read_write(1);

#ifdef ENET_UBL
    uart_puts("\r\nEnvironment found with chksum ");
    uart_put_long(packed_boot_environment_p->sum, 16);
#else
    bsp_debug_printf("Environment found with chksum 0x%x\n", packed_boot_environment_p->sum);
#endif
#endif
    memset(boot_environment_p, 0, sizeof(boot_environment));
    
    i = 0;
    sp = packed_boot_environment_p->env;
    while (*sp) {
        tp = boot_environment_p->env_db[i].name;
        while (*sp) {
            *tp++ = *sp++;
        }
        *tp = *sp++;
        tp = boot_environment_p->env_db[i].value;
        while (*sp) {
            *tp++ = *sp++;
        }
        *tp = *sp++;
        ++i;
        if (i >= ENV_MAX_ENV) {
            break;
        }
    }
    sum = compute_sum(boot_environment_p->env_db);
    if (sum != packed_boot_environment_p->sum) {
#ifdef ARM   
#ifdef ENET_UBL        
        uart_puts("\r\nSum mismatch ");
        uart_put_long(sum, 16);
#else
        bsp_debug_printf("Sum mismatch 0x%x\n", sum);
#endif
#endif
        memset(boot_environment_p, 0, sizeof(boot_environment));
        return 0;
    }
    boot_environment_p->sum = sum;
    return 1;
}
#endif

/*
 * env_iter_init
 *
 * Initialize an iterator so one can browse all environment variables
 */
uint32_t env_iter_init (void) {
    return 0;
}

/*
 * env_iter_next
 *
 * Return the next environment variable in the database
 */
uint32_t env_iter_next (uint32_t *iter, char **name, char **value) {
    *name = NULL;
    *value = NULL;
    while (*iter < ENV_MAX_ENV) {
        if (boot_environment_p->env_db[*iter].name[0]) {
            *name = boot_environment_p->env_db[*iter].name;
            *value = boot_environment_p->env_db[*iter].value;
            ++*iter;
            return 1;
        }
        ++*iter;
    }
    return 0;
}

void env_clear(void) {
    int i;
    if (boot_environment_p == NULL) {
        return;
    }
    for (i = 0; i < ENV_MAX_ENV; ++i) {
        boot_environment_p->env_db[i].name[0] = '\0';
    }
}

int env_is_empty(void) {
    int i;
    if (boot_environment_p == NULL) {
        return 1;
    }
    for (i = 0; i < ENV_MAX_ENV; ++i) {
        if (boot_environment_p->env_db[i].name[0] != '\0') {
            return 0;
        }
    }
    return 1;
}

#ifdef __linux__
/*
 * env_get_raw
 *
 * return the raw (in EEPROM) representation of the environment
 * for use by conv26x
 */
void *env_get_raw (uint32_t *size) {
    env_sync(0);
    *size = sizeof(packed_boot_environment);
    return packed_boot_environment_p;
}
#endif

/******** History ********
$Log: env.c,v $
Revision 1.2  2016/10/07 17:55:18  srane
CSCvb61570 - Move to SWIMS server for code signing

Revision 1.1  2012/06/28 13:33:09  srane
New boot loader requirements - environment variables, unique mgaic
number for SP2704 (will boot only 2704), SSP support.


$Endlog$
*/

