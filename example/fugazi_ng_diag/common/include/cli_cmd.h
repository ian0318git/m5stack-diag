/* $Id: cli_cmd.h,v 1.9 2014/06/03 10:53:32 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/cli_cmd.h,v $
 ***********************************************************************
 *
 * cli_cmd.c - new CLI command for manufacturing.
 *
 * Mar 2008 - steja
 *
 * Copyright (c) 2009-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Scott Tsai 
 ***********************************************************************
 */

#ifndef _CLI_CMD_H_
#define _CLI_CMD_H_

#define CLI_HEX     16
#define CLI_DEC     10
#define CLI_BIN     2

#define CLI_MODE   TRUE
#define MENU_MODE  FALSE
#define CTERR_MODE  2

#define CLI_DISCOVERY 1
#define CLI_COOKIE    2
#define CLI_DEVICE_IS_VACANT  3

/********************* Enum *******************/
enum cli_ge_ethernet_port{
    E0 = 140,  /* Ethernet 0 */
    E1,        /* Ethernet 1 */
    E2,        /* Ethernet 2 */
    E3,        /* Ethernet 3 */
    E4,        /* Ethernet 4 */
    E5         /* Ethernet 5 */
};

/* DEFINE COOKIE MODULE */
enum cli_cookie_module {
    CLI_COOKIE_MB = 0,
    CLI_COOKIE_BP,
    CLI_COOKIE_VM,
    CLI_COOKIE_SM,
    CLI_COOKIE_WIC,
    CLI_COOKIE_NM,
    CLI_COOKIE_PSU,
    CLI_COOKIE_MP,
    CLI_COOKIE_ISM,
    CLI_COOKIE_SMDC,       /* SM 2nd level */
    CLI_COOKIE_NMEHWIC,    /* NM 2nd level */
    CLI_COOKIE_NMDC,       /* NM 2nd level */
    CLI_COOKIE_NMDCGE,     /* NM 2nd level */
    CLI_COOKIE_NMDCPWR,    /* NM 2nd level */
    CLI_COOKIE_NMEM,       /* NM 2nd level */
    CLI_COOKIE_NMEHWICECAN,  /* NM 3rd level */
    CLI_COOKIE_NMPVDM,       /* NM 4th level */
    CLI_COOKIE_WICDC,      /* WIC 2nd level */
    CLI_COOKIE_EHWICECAN     /* EHWIC 2nd level */
};

/* DEFINE AUTH AND QUACK MODULE */
enum cli_auth_quack_module {
    CLI_AUTH_MB = 0,
    CLI_AUTH_VM,
    CLI_AUTH_SM,
    CLI_AUTH_SMDC,
    CLI_AUTH_WIC,
    CLI_AUTH_WICDC,
    CLI_AUTH_NM,
    CLI_AUTH_ISM
};

/* DEFINE TEST MODULE */
enum cli_test_module {
    CLI_TEST_MB = 0,
    CLI_TEST_IO,
    CLI_TEST_AIM,
    CLI_TEST_VM,
    CLI_TEST_WIC,
    CLI_TEST_SM,
    CLI_TEST_ALL,
    CLI_TEST_DASH,
    CLI_TEST_MBWO,
    CLI_TEST_ISM,
    CLI_TEST_DATA_PLN
};

enum cli_set_cmd_type {
    CLI_SET_DATE = 0,
    CLI_SET_TIME
};

enum cli_cookie_cmd_type {
    CLI_COOKIE_BEGIN = 0,
    CLI_COOKIE_CHANGE,
    CLI_COOKIE_ADD,
    CLI_COOKIE_REMOVE,
    CLI_COOKIE_FILL,
    CLI_COOKIE_DISPLAY_RAW,
    CLI_COOKIE_DISPLAY_FMT,
    CLI_COOKIE_DISPLAY_M2M,
    CLI_COOKIE_SAVE
};

enum cli_eeprom_type {
    CLI_UNKNOWN_EEPROM = 0,
    CLI_SMART_EEPROM,
    CLI_ATMEL_EEPROM
};

enum cli_cookie_size {
    CLI_UNKNOWN_INDEX = -1,
    CLI_S128 = 0x80,
    CLI_S256 = 0x100,
    CLI_S512 = 0x200
};

enum cli_format {
    CLI_RAW = 0,
    CLI_FMT
};

enum cli_dc_level {
    firstlevel = 0,
    secondlevel,
    thirdlevel
};

enum cli_time {
    hour = 1,
    minute,
    second
};

enum cli_date{
    month = 1,
    date,
    year
};
/* GE port for "MB-WO" cli test command */
enum cli_phyport {
    PHY0 = 0,
    PHY1,
    PHY2,
    PHY3,
    PHY4,
    PHY5
};

enum cli_psu_no {
    ONEPSU = 1,
    TWOPSU
};

/* CLI cookie and discovery command for n level cookie info structure */
typedef struct cli_nlevel_t_ {
    char *name;
    unsigned short id;
    int (*secondlvl)();  /* 2nd level routine */
    int (*thirdlvl)();  /* 3rd level routine */
    int (*forthlvl)();  /* 4th level routine */
} cli_nlevel_t;

typedef struct cli_cookie_cmd_t {
    int type;
    int board_type;
    int slot;
    int size;
    int dc_slot;    /*daughter card slot*/
    int dcdc_slot;  /*daughter card the daughter card slot*/
    int max_vs;
    int dc_type;
    int cli_mode;   /* TRUE is CLI mode otherwise is Menu mode */
    char buf[20];
    char *str;
    //    char
    boolean present;
    ushort nm_cookie_id;
    ushort sm_cookie_id;
    ushort hwic_cookie_id;
    ushort venom_em_ctrl_type;
    uchar field;
    uchar wic_type;
    uchar *contents;
} cli_cookie_cmd;

typedef struct cli_value_t {
    char value[80];
    struct cli_value_t *next;
} cli_value;

typedef struct cli_token_t {
    int count;
    char token;
    struct cli_value_t *vlist;
} cli_token;

typedef struct cli_time_t {
    int second;
    int minute;
    int hour;
    int date;
    int month;
    int year;
} cli_time;

extern uint32_t get_platform_memsize(void);
extern int menu_flags (int);
extern int slot_start_with (void);
extern int get_max_num_vm (void);
extern int get_max_hwic_slots (void);
extern int pvdm_present (int);
extern boolean pas_pa_present (uint);
extern boolean is_sm_present (int);
extern ushort get_hwic_cookie_id (int, void *, uchar);
extern void initsigs (void);
extern int show_temp(int, int);
extern boolean has_ps1(void);
extern boolean has_ps2(void);
extern int cli_set_pid_to_psu_c(int);
extern int cli_clear_psu_c_pid(int);
extern int cli_check_ps_present(uint);
extern int it_is_shockwave(void);
extern int it_is_megatron(void);
extern int cli_main_menu_table_size(void);
extern ushort get_mb_id(void);
extern ushort get_pvdm_id (int);
extern ushort get_ism_id(int);
extern ushort get_bp_id(void);
extern ushort get_psu_id(uint, int);
extern int alter_mb_cookie_x (boolean, cli_cookie_cmd *);
extern int alter_ism_cookie_x (boolean, cli_cookie_cmd *);
extern int mp_show_cookie_x(boolean, cli_cookie_cmd *);
extern int alter_pvdm_cookie_x (boolean, cli_cookie_cmd *);
extern int alter_wic_cookie_cli (boolean, cli_cookie_cmd *);
extern int32_t alter_sm_cookie_cli (boolean, cli_cookie_cmd *);
extern int32_t alter_nm_cookie_x (boolean, cli_cookie_cmd *);
extern int psu_show_cookie_x(boolean, cli_cookie_cmd *);
extern int cli_change_cookie(int, char *, cli_cookie_cmd *);
extern void release_reset_of_sm (int);
extern int show_margins_x(int, int);

#endif /* _CLI_CMD_H_ */

/*end of file cli_cmd.h */

/***********************************************************************
$Log: cli_cmd.h,v $
Revision 1.9  2014/06/03 10:53:32  erwu2
python menu collapsed to main trunk

Revision 1.8.2.1  2014/04/24 09:01:28  erwu2
support basic utilily and test data plane CLI

Revision 1.8  2013/11/26 08:40:32  hroni
fix compiler warning

Revision 1.7  2013/10/08 11:03:47  erwu2
enhanced err msg first check-in

Revision 1.6  2013/03/11 03:33:15  alpeng
supporting CLI for NGIO-DC

Revision 1.5  2013/02/22 03:44:56  alpeng
update EHWIC to WIC for supporting CLI test cmd

Revision 1.4  2012/08/30 07:44:56  alpeng
infrom user with warning when device is vacant on CLI discovery

Revision 1.3  2012/05/08 06:12:57  alpeng
change the CLI cmd from PVDM to VM

Revision 1.2  2012/03/28 00:38:09  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
