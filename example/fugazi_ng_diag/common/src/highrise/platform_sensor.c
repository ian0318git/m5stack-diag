/* $Id:
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/highrise/platform_sensor.c,v $
 *------------------------------------------------------------------
 * platform_sensor.c
 *
 * Description: Digital Temperature Sensor Definitions.
 *              This file is based on TMP75/ADT75 Datasheet.
 *
 * Copyright (c) 2019 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>
#include "endians.h"
#include "common.h"
#include "platform_sensor.h"
#include "defs.h"
#include "error.h"
#include "platform_i2c.h"
#include "menu.h"
#include "i2c_dev.h"
#include "common_utils.h"
#include "byteswap.h"
#include "hr_commn_util.h"

/*
 * Main menu test flag defines
 */
#define MM_1    (MF_CONTINUOUS)
#define MM_2    (MM_1 | MF_DOALL)
#define MM_3    (MM_2 | MF_SHOW_ERRCOUNT)


/******************************************************************************
 *                                   Externs
 ******************************************************************************/

uint32 highrise_display_temp(void);
int build_ts_utils_menu (boolean ts_util_items_executed);
extern int highrise_mem_read32 (uint offset, uint *buf);
extern int highrise_mem_write32 (uint offset, uint buf);
extern int hr_version_v2;   
/******************************************************************************
 *                               Function prototypes
 ******************************************************************************/
static uint32 show_temperature (int);
static uint32 ts_sanity_test (int);
static uint32 alter_ts_reg(int);
static uint32 show_ts_reg(int);
static uint32 ts_interrupt_test(int);
static int hr_has_intr_test(void);

/******************************************************************************
 *                                 Global variables
 ******************************************************************************/

const static uint8_t cfg_shutdown_pos   = 0x0;
const static uint8_t cfg_shutdown_msk   = 0x1;
const static uint8_t cfg_mode_pos       = 0x1;
const static uint8_t cfg_mode_msk       = 0x1;
const static uint8_t cfg_polarity_pos   = 0x2;
const static uint8_t cfg_polarity_msk   = 0x1;
const static uint8_t cfg_fault_pos      = 0x3;
const static uint8_t cfg_fault_msk      = 0x3;
//const static uint8_t cfg_resolution_pos = 0x5;
//const static uint8_t cfg_resolution_msk = 0x3;
//const static uint8_t cfg_oneshot_pos    = 0x7;
//const static uint8_t cfg_oneshot_msk    = 0x1;

/* Temperature sensor registers table. This device has registers with different sizes.
 */
static reg_info_t ts_reg_table[] =
{
/*  { name, offset, rw type,
 *    size, mask, default value},
 */
    {"Temperature",		TS_PTR_TEMP,	READ_ONLY,
	{TS_PTR_TEMP_L},	0xFF80, 0x0000},

    {"Configuration",	TS_PTR_CFG,	READ_WRITE,
	{TS_PTR_CFG_L},	0x1F, 0x00},

    {"T_LOW",	TS_PTR_THYST,	READ_WRITE,
	{TS_PTR_THYST_L},	0xFF80, 0x0000},

    {"T_HIGH",	TS_PTR_TOS,	READ_WRITE,
	{TS_PTR_TOS_L},	0xFF80, 0x0000},

    // Only ADT75 has TS_PTR_OS
    {"One-shot mode",	TS_PTR_OS,	READ_WRITE,
	{TS_PTR_OS_L},	0xFF80, 0x0000},

    {0, 0, 0, {0}, 0, 0},
};

/******************************************************************************
 *                                   Menus
 ******************************************************************************/
/*
 * Temperature sensor Menu
 */
static submenu_xtable_t ts_menu_table[] = {
    {"TS utilities",
        (PFT)build_ts_utils_menu,    0,
        0,
        (type_t(*)())0, 0,
        (PFT)build_ts_utils_menu,    0},

    {"Show temperature",
        (PFT)show_temperature,       0,
        MM_3,
        (type_t(*)())0, 0,
        (PFT)show_temperature,       0},

    {"Register sanity test",
        (PFT)ts_sanity_test,         0,
        MM_3,
        (type_t(*)())0, 0,
        (PFT)ts_sanity_test,         0},

    {"Interrupt test",
        (PFT)ts_interrupt_test,      0,
        MM_3,
        (type_t(*)())hr_has_intr_test, 0,
        (PFT)ts_interrupt_test,      0},
};

#define TS_MENU_TABLE_SIZE (sizeof(ts_menu_table) / sizeof(submenu_xtable_t))

/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_menu_primary_items[TS_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t ts_menu_secondary_items[TS_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo tsdiag = {
    "Temperature Sensors Utility Menu",	    /* title */
    0,				    /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,	    /* shows major flags */
    0,				    /* generic prompt */
    0,				    /* size -- bumped by add_menu_item() */
    ts_menu_primary_items,
};

static struct menuinfo *tsdiagp = &tsdiag;




/* TS utils menu */
static submenu_xtable_t ts_util_menu_table[] = {
    {"Show register",
        (PFT)show_ts_reg, 0,
        MM_3,
        (type_t(*)())0, 0,
        (type_t(*)())0, 0},

    {"Alter register",
        (PFT)alter_ts_reg, 0,
        MM_3,
        (type_t(*)())0, 0,
        (type_t(*)())0, 0},
};

#define TS_UTIL_MENU_TABLE_SIZE (sizeof(ts_util_menu_table)/sizeof(submenu_xtable_t))
/*
 * Primary & secondary submenu items (filled in from xtable)
 */
static mitem_t ts_util_menu_primary_items[TS_UTIL_MENU_TABLE_SIZE +
                                         MAX_BASE_ITEMS];
static mitem_t ts_util_menu_secondary_items[TS_UTIL_MENU_TABLE_SIZE +
                                           MAX_BASE_ITEMS];

static struct menuinfo tsutildiag = {
  "TS Utility Menu",      /* title */
  0,                            /* title string added by init_empty_menu */
  (PFT)menu_show_dflags,        /* shows major flags */
  0,                            /* generic prompt */
  0,                            /* size -- bumped by add_menu_item() */
  ts_util_menu_primary_items,
};

static struct menuinfo *tsutildiagp = &tsutildiag;



/******************************************************************************
 *
 * function   : build_ts_menu
 * Description:	Build menu for temperature sensor related utility.
 * Inputs     :
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
int build_ts_menu (int show_menu)
{
    build_primary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                          "Temperature Sensors Utility Menu", &tsdiagp);
    build_secondary_submenu(ts_menu_table, TS_MENU_TABLE_SIZE,
                            ts_menu_secondary_items);
    if (show_menu) {
        /* Entered with submenu */
        menu(tsdiagp, ts_menu_secondary_items, 0);
    } else {
        /* Invoked the test from main menu */
        do_all_menu_items(tsdiagp);
    }

    return (PASSED);
}

static int hr_has_intr_test (void) 
{
    return (hr_version_v2); 
}


/*************************************************************************
 * Function:    build_ts_utils_menu
 *
 * Description: Entry to TS chip utilities menu.
 *
 * Inputs:      option - future use.
 *
 * Outputs:     PASSED.
 *
 *************************************************************************
 */
int build_ts_utils_menu (boolean option)
{

    build_primary_submenu(ts_util_menu_table, TS_UTIL_MENU_TABLE_SIZE,
                          "TS Utility Main Menu", &tsutildiagp);
    build_secondary_submenu(ts_util_menu_table, TS_UTIL_MENU_TABLE_SIZE,
                            ts_util_menu_secondary_items);

    menu(tsutildiagp, ts_util_menu_secondary_items, '\0' );

    return PASSED;
}

/******************************************************************************
 *
 * function   : read_ts_reg
 * Description: Wrapper to read temperature sensor's register.
 * Inputs     : ts_id - sensor ID (Highrise only has one TS)
 *              offset - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int
read_ts_reg (int ts_id, int offset, char * data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO, MB_I2C_ADDR_TMP75);
    if (NULL == i2c_if) {
        cterr('f', 0, "%s[%d]: i2c_if NULL pointer error!\n", __FUNCTION__, __LINE__);
        return(FAILED);
    }
    i2c_if->offset = offset;
    i2c_if->buf = data_buf_p;
    i2c_if->size = size;

    rc = n2g_i2c_read(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s[%d]: Failed to read reg offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, offset, size, rc);
		return(FAILED);
    }
    if (size == sizeof(uint16_t)) {
        *((uint16_t *)data_buf_p) =  be16toh(*((uint16_t *)data_buf_p));
    }

    return (rc);
}


/******************************************************************************
 *
 * function   : write_ts_reg
 * Description: Wrapper to write temperature sensor's register.
 * Inputs     : ts_id - sensor ID (Highrise only has one TS)
 *              offset - pointer register ID.
 * 				data_buf_p - pointer to data buffer
 *              size - number of bytes to be read.
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static int
write_ts_reg (int ts_id, int offset, char *data_buf_p, int size)
{
    uint32_t rc = FAILED;
    n2g_i2c_if_t *i2c_if;

    /* Setup I2C API interface struct */
    i2c_if = (n2g_i2c_if_t *)get_n2g_i2c_if(I2C_CTRL_TWO, I2C_MUX_ZERO, MB_I2C_ADDR_TMP75);
    if (NULL == i2c_if) {
        cterr('f', 0, "%s[%d]: i2c_if NULL pointer error!\n", __FUNCTION__, __LINE__);
        return(FAILED);
    }
    i2c_if->buf = data_buf_p;
    i2c_if->offset = offset;
    i2c_if->size = size;

    rc = n2g_i2c_write(i2c_if);
    if (rc != PASSED) {
        cterr('f', 0, "%s[%d]: Failed to write reg offset %#x, size = %d(rc = %#x)",
                      __FUNCTION__, __LINE__, offset, size, rc);
        return (FAILED);
    }
    usleep(100000); /* sleep 100 ms after writing */
    return(rc);
}


/******************************************************************************
 *
 * function   : ts_sanity_test
 * Description: A quick sanity read and write access test to device's R/W register.
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED.
 *
 ******************************************************************************/
static uint32
ts_sanity_test (int ts_id)
{
    int offset;
    ts_t backup_buf, new_buf, compare_buf;
    uint32_t rc = FAILED;
    int uret = FAILED;
    int flag = 0;

    char *tname = "Temp Sensor Register";
    testname(tname);
    prpass(testpass, "\nStart ");
    printf("\n");

    for (offset = TS_PTR_THYST; offset <= TS_PTR_TOS; offset++) {
        flag = 0;
        backup_buf = new_buf = compare_buf = 0;

        /* read and backup reg data */
        rc = read_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t));
        TERR_URET_COND(rc != PASSED, FAILED, "backup old value: read_ts_reg @0x%x failed!\n", offset);

        /* write new data */
        new_buf = (backup_buf + TS_TEST_PATTERN) & TS_TEMP_MASK;
        rc = write_ts_reg (ts_id, offset, (char*) &new_buf, sizeof(ts_t));
        TERR_URET_COND(rc != PASSED, FAILED, "write new value: write_ts_reg @0x%x failed!\n", offset); 

        flag = 1;
        // read and compare reg data
        rc = read_ts_reg (ts_id, offset, (char*) &compare_buf, sizeof(ts_t));
        TERR_URET_COND(rc != PASSED, FAILED, "read new value: read_ts_reg @0x%x failed!\n", offset);
        if (compare_buf != new_buf) {
            cterr('f', 0, "%s:%d compare result: value not matched @0x%x (%#x != %#x)\n",
                    __FUNCTION__, __LINE__, offset, new_buf, compare_buf);
            goto _EXIT_POINT;
        }

        /* restore original data */
        rc = write_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t));
        TERR_URET_COND(rc != PASSED, FAILED, "restore old value: write_ts_reg @0x%x failed!\n", offset);
    }

    uret = PASSED;

_EXIT_POINT:
    if (flag && (uret == FAILED)) {
        write_ts_reg (ts_id, offset, (char*) &backup_buf, sizeof(ts_t));
    }
    prpass(testpass, "%s ", (uret == PASSED ? "Passed" : "Failed"));
    return(uret);
}


static int ts_get_temperature(const ts_t *regv, float *temp)
{
    ts_t val;
    float resolution = TS_TEMP_RESOLUTION;

    if (!regv) {
        ERR_RET_COND(PASSED != read_ts_reg(0, TS_PTR_TEMP, (char*) &val, TS_PTR_TEMP_L),
                 -(__LINE__), "Failed to read ts reg-%d\n", TS_PTR_TEMP);
    } else {
        val = *regv;
    }

    if (val <= TS_TEMP_MAX) {
        *temp = (val >> 4) * resolution;
    } else {
        *temp = ((val >> 4) - 4096) * resolution;
    }
    return 0;
}

static int ts_tmperature2regv(const float temp, uint16_t *regv)
{
    float resolution = TS_TEMP_RESOLUTION;

    *regv = temp < 0 ?
            0xffff & (((uint32_t)(temp / resolution)) << 4)
            :
            0xffff & (((uint32_t)(temp / resolution + 4096)) << 4);
    return 0;
}

static int ts_get_threshold(float *low, float *high)
{
    uint16_t regv = 0;

    if (low) {
        ERR_RET_COND(PASSED != read_ts_reg(0, TS_PTR_THYST, (char*)&regv, TS_PTR_THYST_L),
                 -(__LINE__), "Failed to read ts reg-%d\n", TS_PTR_THYST);
        ERR_RET_COND(0 > ts_get_temperature(&regv, low), -(__LINE__), "Get temp failed.\n");
    }

    if (high) {
        ERR_RET_COND(PASSED != read_ts_reg(0, TS_PTR_TOS, (char*)&regv, TS_PTR_TOS_L),
                 -(__LINE__), "Failed to read ts reg-%d\n", TS_PTR_TOS);
        ERR_RET_COND(0 > ts_get_temperature(&regv, high), -(__LINE__), "Get temp failed.\n");
    }
    return 0;
}

static int ts_set_threshold(const float *low, const float *high)
{
    uint16_t regv = 0;
    if (low) {
        ERR_RET_COND(0 > ts_tmperature2regv(*low, &regv), -(__LINE__), "Failed to convert temp to reg value.\n");
        ERR_RET_COND(PASSED != write_ts_reg(0, TS_PTR_THYST, (char*)&regv, TS_PTR_THYST_L),
                 -(__LINE__), "Failed to write ts reg-%d\n", TS_PTR_THYST);
    }
    if (high) {
        ERR_RET_COND(0 > ts_tmperature2regv(*high, &regv), -(__LINE__), "Failed to convert temp to reg value.\n");
        ERR_RET_COND(PASSED != write_ts_reg(0, TS_PTR_TOS, (char*)&regv, TS_PTR_TOS_L),
                 -(__LINE__), "Failed to write ts reg-%d\n", TS_PTR_TOS);
    }
    return 0;
}

/******************************************************************************
 *
 * Function   : show_temperature
 * Description: show temperature of the selected sensor
 * Inputs     : ts_id - sensor ID (default is bezel temperature sensor)
 * Outputs    : PASSED/FAILED
 *
 ******************************************************************************/
static uint32
show_temperature (int ts_id)
{
    float t = 0;

    ERR_RET_COND(0 > ts_get_temperature(NULL, &t), FAILED, "Failed\n");
    printf("%-16s: %.4f Celcius\n", "Sensor Temp", t);

    return(PASSED);
}

/********************************************************************
 *
 * Function:	show_ts_reg
 *
 * Description:	show temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
show_ts_reg(int ts_id)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t buffer = 0;

    reg_p = &ts_reg_table[0];

    printf("ts_id %d\n", ts_id);
    while (reg_p->name) {
        buf_p  = (char *)&buffer;
        offset = reg_p->offset;
        size   = reg_p->size.size;
        if (offset == TS_PTR_OS) {
            reg_p++;
            continue;
        }
        rc = read_ts_reg (ts_id, offset, buf_p, size);
        if (rc != PASSED) {
             cterr('f', 0, "%s:%d Failed to read temperature sensor @0x%x\n",
                    __FUNCTION__, __LINE__, offset);
             return (FAILED);
        } else {
            if (size == sizeof(ts_t))
                printf("%-36s (0x%02X), data: 0x%04X\n", reg_p->name,
                        reg_p->offset, buffer);
            else
                printf("%-36s (0x%02X), data: 0x%02X\n", reg_p->name,
                        reg_p->offset, buffer & 0xff);
        }
        reg_p++;
    }
    return (PASSED);
}


/********************************************************************
 *
 * Function:	alter_ts_reg
 *
 * Description:	alter temperature sensor register.
 *
 * Inputs:	-
 *
 * Outputs:	PASSED - No errors encounterd.
 *		FAILED - Errors encounterd.
 *
 *********************************************************************
 */
static uint32
alter_ts_reg(int ts_id)
{
    int offset;
    int size;
    uint32 rc;
    char* buf_p;
    reg_info_t *reg_p;
    ts_t temp_data, old_temp_data;
    ts_c cfg_data, old_cfg_data;

    reg_p = &ts_reg_table[0];

    printf("ts_id %d\n", ts_id);
    printf("\nRegister number:\n");
    /* Get the register to peek-n-poke */
    while (reg_p->name) {
        if (reg_p->type & READ_ONLY) {
            /* Read only register */
        } else if (reg_p->offset == TS_PTR_OS) {
            printf("   %02x - %s - only for ADT75!\n", reg_p->offset, reg_p->name);
        } else {
            /* Write only or read/write register */
            printf("   %02x - %s\n", reg_p->offset, reg_p->name);
        }
        reg_p++;	/* update the register table pointer */
    } /* endof while */

    /* Get the register to peek-n-poke */
    offset = gethex_answer("Enter the register number:", 0, 0, TS_PTR_OS);

    /* Check if the register is read/writeable */
    /* Find the register text in the register table */
    reg_p = &ts_reg_table[0]; /* Points to the beginning of the table */

    while (reg_p->name && reg_p->offset != offset) {
         /* Not requested register */
         reg_p++; /* update the register table pointer */
    }

    if (reg_p->name) {
         /* Got the register. */
         if (reg_p->type == READ_WRITE) {
             /* Read/Writeable. Read the register first. */
             if (reg_p->size.size == sizeof(ts_c)) {
                   /* Configuration register */
                   size = sizeof(ts_c);
                   buf_p = (char *)&old_cfg_data;
             } else {
                   /* Temperature registers */
                   size = sizeof(ts_t);
                   buf_p = (char *)&old_temp_data;
             }

             offset = reg_p->offset;

             /* Configuration register */
             rc = read_ts_reg (ts_id, offset, buf_p, size);
             if (rc != PASSED) {
                  cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
							  __FUNCTION__, __LINE__, ts_id);
                  return (FAILED);
             }

             /* Get the new data */
             if (reg_p->size.size == sizeof(ts_c)) {
                  /* Configuration register */
                  cfg_data = gethex_answer("Enter the data:", old_cfg_data, 0, 0x1F);
                  buf_p = (char*)&cfg_data;
             } else {
                  /* Temperature registers */
                  temp_data = gethex_answer("Enter the data:",
                  old_temp_data, 0, TS_TEMP_MASK);
                  buf_p = (char *)&temp_data;
             }

             /* Write the new data */
             rc = write_ts_reg (ts_id, offset, buf_p, size);
             if (rc != PASSED) {
                   cterr('f', 0, "%s:%d write_ts_reg failed\n", __FUNCTION__, __LINE__);
                   return (FAILED);
              }
          } else {
               /* Not read/writeable */
               printf("alter_ts_reg() %s is a read only register", reg_p->name);
               return (FAILED);
          }
    } else {
          /* Invalid register */
          printf("alter_ts_reg() %#x is not a valid register", offset);
           return (FAILED);
    }
    return (PASSED);
}


uint32 highrise_display_temp(void)
{
    int rc = 0;
    rc = show_temperature(0);
    if (rc != PASSED) {
        printf("[%s]:%d: Error to show temperature\n", __FUNCTION__, __LINE__);
        return FAILED;

    }
    return PASSED;
}

int highrise_config_ts_init(void)
{
    int ts_id = 0;
    int rc    = 0;
    const uint8 shutdown_val   = 0;
    const uint8 mode_val       = 1;
    const uint8 polarity_val   = 0;
    const uint8 fault_val      = 1;

    uint8 val = ((shutdown_val   & cfg_shutdown_msk  ) << cfg_shutdown_pos  ) |
                ((mode_val       & cfg_mode_msk      ) << cfg_mode_pos      ) |
                ((polarity_val   & cfg_polarity_msk  ) << cfg_polarity_pos  ) |
                ((fault_val      & cfg_fault_msk     ) << cfg_fault_pos     );


    rc = write_ts_reg(ts_id, TS_PTR_CFG, (char *)&val, TS_PTR_CFG_L);
    if (rc != PASSED) {
        cterr('f', 0, "%s:%d Failed to read temperature sensor#%d",
                      __FUNCTION__, __LINE__, ts_id);
        return (FAILED);
    }

    return (PASSED);
}

/* GPIO init is done in highrise.c */
static const struct {
    char *name;
    uint32_t rego;
    uint32_t boff;
    uint32_t blen;
    char *tag;
} ts_intr_mpp[] = {
    /*0 */{"function        ", 0xf2440004, 16,  4, ""                      } ,
    /*1 */{"data-out        ", 0xf2440100, 12,  1, ""                      } ,
    /*2 */{"data-out-en     ", 0xf2440104, 12,  1, "Active Low"            } ,
    /*3 */{"blink-en        ", 0xf2440108, 12,  1, ""                      } ,
    /*4 */{"data-in-polarity", 0xf244010c, 12,  1, "If set, 'data-in' is inverted of PIN" },
    /*5 */{"data-in         ", 0xf2440110, 12,  1, ""                      } ,
    /*6 */{"intr-cause      ", 0xf2440114, 12,  1, ""                      } ,
    /*7 */{"intr-mask       ", 0xf2440118, 12,  1, ""                      } ,
    /*8 */{"intr-level-mask ", 0xf244011c, 12,  1, ""                      } ,
    /*9 */{"blink-cntr      ", 0xf2440120, 12,  1, ""                      } ,
    /*10*/{NULL              , ~0        , ~0, ~0, ""                      } ,
};

static int ts_intr_gpio_dump(const char *tag)
{
    uint32_t regv = 0;
    int      idx  = 0;

    for(idx = 0; ts_intr_mpp[idx].name; idx++) {
        ERR_RET_COND(PASSED != highrise_mem_read32(ts_intr_mpp[idx].rego, &regv),
            -(__LINE__), "Failed to read ts intr gpio reg-%s.\n", ts_intr_mpp[idx].name);

        printf("%-16s:@%08x:%08x:boff-%-2u:blen-%u:bval-%x %s\n",
            ts_intr_mpp[idx].name,
            ts_intr_mpp[idx].rego, regv,
            ts_intr_mpp[idx].boff,
            ts_intr_mpp[idx].blen,
            (regv >> ts_intr_mpp[idx].boff) & ((1 << ts_intr_mpp[idx].blen) - 1),
            ts_intr_mpp[idx].tag
        );
    }
    return 0;
}

static int ts_is_intr_active(int clr)
{
    uint32_t regv = 0;
    uint32_t msk  = 0;
    int ret = 0;

    ERR_RET_COND(PASSED != highrise_mem_read32(ts_intr_mpp[6].rego, &regv),
        -(__LINE__), "Failed to read ts intr cause reg.\n");

    msk  = (((1 << ts_intr_mpp[6].blen) - 1) << ts_intr_mpp[6].boff);
    ret = regv & msk ? 1 : 0;

    if (ret && clr) {
        /* clear gpio intr status */
        regv &= ~msk;
        ERR_RET_COND(PASSED != highrise_mem_write32(ts_intr_mpp[6].rego, regv),
            -(__LINE__), "Failed to write ts intr cause reg.\n");
    }

    return ret;
}

static int ts_intr_trigg(void)
{
    int idx = 0;
    float curtemp = 0.0;
    float lthresh = 0.0;
    float hthresh = 0.0;

    printf("\nTS regs & gpio before adjusting thresholds:\n");
    show_ts_reg(0);
    ts_intr_gpio_dump(NULL);

    ERR_RET_COND(0 >  ts_is_intr_active(1), -(__LINE__), "Clear intr failed.\n");
    ERR_RET_COND(0 != ts_is_intr_active(0), -(__LINE__), "Clear intr failed.\n");
    ERR_RET_COND(0 > ts_get_temperature(NULL, &curtemp), -(__LINE__), "Failed to get current temp.\n");
    ERR_RET_COND(0 > ts_get_threshold(&lthresh, &hthresh), -(__LINE__), "Failed to get temp thresholds.\n");

    printf("\nTS regs & gpio after intr status cleared:\n");
    show_ts_reg(0);
    ts_intr_gpio_dump(NULL);

    ERR_RET_COND(lthresh > hthresh, -(__LINE__),
        "Invalid config, low thresh(%2.3f) is greate than high thresh(%2.3f).\n", lthresh, hthresh);

    if (curtemp < lthresh) {
        /* cur < low < high */
        printf("cur < low < high\n");
        lthresh = curtemp - 5;
        hthresh = curtemp - 2;
    }
    else if (curtemp < hthresh) {
        /* low < cur < high */
        /* we don't know from which direction we arrive at this point */
        /* we adjust the threshold 2 times in sequence {L-C-H --> 1:L-H-C --> 2:C-L-H} */
        printf("low < cur < high\n");
        lthresh = curtemp - 5;
        hthresh = curtemp - 2;
        ERR_RET_COND(0 > ts_set_threshold(&lthresh, &hthresh), -(__LINE__), "Failed to trigger intr.\n");
        usleep(250000); /* give time to sensor to sample & trigger intr */
        ERR_RET_COND(0 > ts_get_threshold(&lthresh, &hthresh), -(__LINE__), "Failed to read thresh intr.\n");
        lthresh = curtemp + 2;
        hthresh = curtemp + 5;
    }
    else {
        /* low < high < cur */
        printf("low < high < cur\n");
        lthresh = curtemp + 2;
        hthresh = curtemp + 5;
    }
    ERR_RET_COND(0 > ts_set_threshold(&lthresh, &hthresh), -(__LINE__), "Failed to trigger intr.\n");

    /*
    1. The gpio is configed as edge-sensitive and the input is not inverted,
       the intr cause will be set at low-2-high trans;
    2. The sensor will assert(active low) the alert pin and remains asserted until
       any register is read(refer to highrise schematic and temp75 sepc);
    3. So after adjusting the thresholds, alert pin will assert; And we need to
       read some register to deassert to alert pin to make a 'low to high' condition.
    NOTE:
        a     b    c
        ----+   +-----
            |___|
        Intr-cause set at b->c
    */

    for(idx = 0; idx < 20; idx++) {
        /* keep reading to trigger interrupt */
        ERR_RET_COND(0 > ts_get_threshold(&lthresh, &hthresh), -(__LINE__), "Failed to trigger intr.\n");
        if (ts_is_intr_active(0) == 1)
            return 0;
        usleep(200000);
    }

    ERR_RET_COND(!0, -(__LINE__), "Failed to trigger intr\n");
}

static uint32 ts_interrupt_test(int opt)
{
    int ret  = 0;
    int uret = 0;
    char *tname = "Temp Sensor Intr Test";
    float lthresh = 0;
    float hthresh = 0;
    float curtemp = 0;
    int flag = 0;

    testname(tname);
    prpass(testpass, "\nStart ");
    printf("\n");

    ERR_RET_COND(0 > ts_get_temperature(NULL, &curtemp), -(__LINE__), "Failed to get current temp.\n");
    if (curtemp < -20.0 || curtemp > 100) {
        printf("Current temp is out of range [-20, 100], abort TS intr test.\n");
        return 0;
    }

    /* save orig thresholds */
    TERR_URET_COND(0 >  ts_get_threshold(&lthresh, &hthresh), FAILED, "%s failed.\n", tname);
    flag = 1;

    TERR_URET_COND(0 >  ts_intr_trigg(), FAILED, "%s failed.\n", tname);

    printf("\nTS regs & gpio after triggering:\n");
    show_ts_reg(0);
    ts_intr_gpio_dump(NULL);

    TERR_URET_COND(1 != (ret = ts_is_intr_active(0)), FAILED, "%s failed, ret:%d.\n", tname, ret);

    uret = PASSED;

_EXIT_POINT:
    if (flag) {
        /* restore threshold */
        ts_set_threshold(&lthresh, &hthresh);
    }
    printf("\nTS regs & gpio after restoring thresholds:\n");
    show_ts_reg(0);
    ts_intr_gpio_dump(NULL);

    prpass(testpass, "%s ", (uret == PASSED ? "Passed" : "Failed"));

    return uret;
}

