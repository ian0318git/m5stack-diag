/* $Id: platform_phy.c,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_phy.c,v $
 *********************************************************************
 *
 * platform_phy.c -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

//Platform Driver for Marvell 88x3310P MTD PHY
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <list.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <linux/ethtool.h>

#include "mdio/smi_drv.h"
#include "mdio/xsmi_drv.h"
#include "platform_phy.h"

#include "nvmonvars.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "common.h"
#include "common_utils.h"
#include "hr_commn_util.h"
#include "highrise_eth_traf.h"
#include "highrise_cpld_lib.h"

#define LINK_UP_MAX_POLL 120

extern int highrise_mem_read32 (uint offset, uint *buf);
extern int highrise_mem_write32 (uint offset, uint buf);

extern unsigned char marvell_mtd_x3310fw_hdr_0_3_7_0[];
extern unsigned int marvell_mtd_x3310fw_hdr_len_0_3_7_0;

extern unsigned char marvell_mtd_x3310fw_hdr_0_3_7_16[];
extern unsigned int marvell_mtd_x3310fw_hdr_len_0_3_7_16;

extern unsigned char marvell_mtd_x3310fw_hdr_0_3_10_0[];
extern unsigned int marvell_mtd_x3310fw_hdr_len_0_3_10_0;

extern unsigned char marvell_mtd_x3240flashdlslave_0_6_5_0[];
extern unsigned int marvell_mtd_x3240flashdlslave_0_6_5_0_len;

extern unsigned int getdec_answer(
        char *msgstr, unsigned int currentval, unsigned int min, unsigned int max);


MTD_DEV mtd3310Dev;
MTD_DEV_PTR pMtd3310Dev = &mtd3310Dev;

int phy_device_init_utils(void);
int phy_config_mode_sgmii_utils(void);
int phy_config_mode_100m_utils(void);
int phy_config_mode_2P5G_utils(void);
int phy_change_mtd_verbose_utils(void);
int phy_check_status_utils(void);
int phy_config_loopback_utils(void);
int phy_debug_lkdn_lpbk_utils(void);
int phy_config_tunit_powermode_utils(void);
void build_phy_utils_menu (void);
int phy_enable_pkt_counter_utils(void);
int phy_check_pkt_counter_utils(void);
int phy_clear_pkt_counter_utils(void);
int phy_config_advertisement_utils(void);
int phy_config_media_type_utils(void);
int phy_enable_10g_link_drop_counter(void);
int phy_check_10g_link_drop_counter(void);
static int phy_firmware_ram_download_utils(void);
static int phy_firmware_flash_program_utils(void);

static int marvell_mtd_enable_h_unit_pkt_checker(MTD_DEV *pMtdDev, MTD_U16 port, int enable);
int marvell_config_media_select(MTD_DEV *pMtdDev, MTD_U16 port, int media_select);
int phy_check_t_link_status(MTD_DEV *pMtdDev, MTD_U16 port, MTD_U16 *linkup_p);
int phy_check_h_link_status(MTD_DEV *pMtdDev, MTD_U16 port, MTD_U16 *linkup_p);
int phy_check_cpu_mac_port_status(uint32_t *linkup_p, uint);
int phy_get_t_link_speed(MTD_DEV *pMtdDev, MTD_U16 port, MTD_U16 *t_speed);
int phy_check_10g_link_status(MTD_DEV *,MTD_U16, MTD_U16, MTD_U16 *);
void phy_config_cpu_mac_port_10g(uint32_t); 
int phy_side_band_signals_utils(void);

static long _phy_reg_test(void);
static long _phy_reg_dump(void);
static long _phy_reg_rdwr(void);
static long _phy_shallow_mac_lpbk_test(void);
static long _phy_deep_mac_lpbk_test(void);
static long _phy_100m_ext_lpbk_test(void);
static long _phy_send_pkt_test(void);
static long _phy_intr_test(void);
static long _phy_intr_trig(int);
static long _phy_sfp_lpbk_test(void); 

/*******************************************************************************
 * Description: Build Main Diag Entry for PHY
 *******************************************************************************
 */
static submenu_xtable_t phy_test_tbl[] = {
    {"PHY Utility", (type_t(*)())build_phy_utils_menu, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0},
    { "PHY Register Test", (type_t(*)())_phy_reg_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PHY Shallow Mac Lpbk Test", (type_t(*)())_phy_shallow_mac_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PHY Deep Mac Lpbk Test", (type_t(*)())_phy_deep_mac_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "SFP 10G Lpbk Test", (type_t(*)())_phy_sfp_lpbk_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PHY 100M Ext Lpbk Test", (type_t(*)())_phy_100m_ext_lpbk_test, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PHY Send Packet Test", (type_t(*)())_phy_send_pkt_test, 0,
     0, (type_t(*)())0, 0, (type_t(*)())0, 0 },
    { "PHY Intr Test", (type_t(*)())_phy_intr_test, 0,
     MF_CONTINUOUS | MF_DOALL, (type_t(*)())0, 0, (type_t(*)())0, 0 },
};

#define PHY_TEST_TBL_SIZE \
        (sizeof(phy_test_tbl) / sizeof(submenu_xtable_t))
static mitem_t phy_pri_test_items[PHY_TEST_TBL_SIZE+ MAX_BASE_ITEMS];
static mitem_t phy_sec_test_items[PHY_TEST_TBL_SIZE+ MAX_BASE_ITEMS];

static menuinfo_t phy_test_menu = {
    "PHY Test Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    phy_pri_test_items,
};
static menuinfo_t *phy_test_menup = &phy_test_menu;

int diag_phy_test (boolean items_executed)
{
    build_primary_submenu(phy_test_tbl, PHY_TEST_TBL_SIZE,
                          "PHY", &phy_test_menup);
    build_secondary_submenu(phy_test_tbl, PHY_TEST_TBL_SIZE,
                            phy_sec_test_items);
    if (items_executed) {
        do_all_menu_items(phy_test_menup);
    } else {
        menu(&phy_test_menu, phy_sec_test_items, 0);
    }

    return (PASSED);
}


/*******************************************************************************
 * Description: Build Utils Entry for PHY
 *******************************************************************************
 */
static submenu_xtable_t phy_utils_table[] = {
    {"PHY Debug Level Change",
     (PFT)phy_change_mtd_verbose_utils,  0,
     0,
     (type_t(*) ())0,                    0,
     (type_t(*) ())0,                    0},

    {"PHY Firmware RAM Download",
     (PFT)phy_firmware_ram_download_utils,   0,
     0,
     (type_t(*) ())0,                    0,
     (type_t(*) ())0,                    0},

    {"PHY Firmware SPI Flash Program",
     (PFT)phy_firmware_flash_program_utils,   0,
     0,
     (type_t(*) ())0,                    0,
     (type_t(*) ())0,                    0},

    {"PHY Set 100M Mode",
     (PFT)phy_config_mode_100m_utils,    0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"PHY Set SGMII Mode",
     (PFT)phy_config_mode_sgmii_utils,   0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"PHY Set 2500-BASEX Mode",
     (PFT)phy_config_mode_2P5G_utils,    0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy Registers Dump",
     (PFT)_phy_reg_dump,                 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy Registers read/write",
     (PFT)_phy_reg_rdwr,                 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy Status Check",
     (PFT)phy_check_status_utils,        0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy Loopback configure",
     (PFT)phy_config_loopback_utils,     0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy power mode configure",
     (PFT)phy_config_tunit_powermode_utils,    0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy enable pkt counter",
     (PFT)phy_enable_pkt_counter_utils,  0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy clear pkt counter",
    (PFT)phy_clear_pkt_counter_utils,     0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy check pkt counter",
     (PFT)phy_check_pkt_counter_utils,   0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy config advertisement",
    (PFT)phy_config_advertisement_utils, 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy enable 10G link drop counter",
    (PFT)phy_enable_10g_link_drop_counter, 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy check link drop counter",
    (PFT)phy_check_10g_link_drop_counter,    0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"Phy config media_type",
    (PFT)phy_config_media_type_utils, 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},

    {"PHY TSensor Intr Trigg",
    (PFT)_phy_intr_trig, 0,
    MF_HIDDEN_EXE,
    (type_t(*)())0,                      0,
    (type_t(*)())0,                      0},

    {"Phy debug linkdown loopback",
     (PFT)phy_debug_lkdn_lpbk_utils,     0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},
    {"PHY side band signals",
    (PFT)phy_side_band_signals_utils, 0,
     0,
     (type_t(*)())0,                     0,
     (type_t(*)())0,                     0},
};

#define PHY_UTILS_TBL_SIZE (sizeof(phy_utils_table) / sizeof(submenu_xtable_t))

/* Primary & secondary submenu items (filled in from xtable) */
static mitem_t phy_utils_primary_items[PHY_UTILS_TBL_SIZE + MAX_BASE_ITEMS];
static mitem_t phy_utils_secondary_items[PHY_UTILS_TBL_SIZE + MAX_BASE_ITEMS];

static struct menuinfo phy_utils_diag = {
    "PHY Utilities Submenu",    /* title */
    0,                          /* title string added by init_empty_menu */
    (PFT)menu_show_dflags,      /* shows major flags */
    0,                          /* generic prompt */
    0,                          /* size -- bumped by add_menu_item() */
    phy_utils_primary_items,
};

static struct menuinfo *phy_utils_diagp = &phy_utils_diag;


void build_phy_utils_menu (void) {

    build_primary_submenu(phy_utils_table, PHY_UTILS_TBL_SIZE,
                          "PHY Utilities Submenu",
                          &phy_utils_diagp);
    build_secondary_submenu(phy_utils_table, PHY_UTILS_TBL_SIZE,
                            phy_utils_secondary_items);
    menu(&phy_utils_diag, phy_utils_secondary_items, 0);
}





/************************************************
 * Function: marvell_mtd_smi_read/write()
 * platform only support Clause 22 mdio,
 * mtd SDK handle the conversion to access caluse 45 register with Clause 22 mdio
 * refer to mtd/mtdHwCntl.c Cl45UsingCl22ReadWrite
 *
 * ***********************************************/
MTD_STATUS marvell_mtd_smi_read(MTD_DEV *pMtdDev,
                                 MTD_U16 port,
                                 MTD_U16 reg,
                                 MTD_U16* pdata)
{
    MTD_STATUS rc = 0;
    struct smi_dev *smi = (struct smi_dev *)pMtdDev->appData;
    rc = (MTD_STATUS)smi->drv->read(smi, port, reg, pdata);
    return(rc);
}


MTD_STATUS marvell_mtd_smi_write(MTD_DEV *pMtdDev,
                                  MTD_U16 port,
                                  MTD_U16 reg,
                                  MTD_U16 data)
{
    MTD_STATUS rc = 0;
    struct smi_dev *smi = (struct smi_dev *)pMtdDev->appData;
    rc = (MTD_STATUS)smi->drv->write(smi, port, reg, &data);
    return(rc);
}


int marvell_mtd_smi_create(MTD_DEV *pMtdDev)
{
    int rc = 0;
    struct smi_dev *smi = NULL;
    rc = smi_dev_create(&smi, SMI_REG_SPACE_ADDR, SMI_REG_SPACE_SIZE, "smi-a");
    if (rc < 0) {
        cterr('f', 0, "[%s]%d: Error Create smi dev failed! rc=%d\n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }

    pMtdDev->appData = (MTD_PVOID)smi;

    return (PASSED);
}


/************************************************
 * marvell_mtd_xsmi_read/write()
 *  Clause 45
************************************************/
MTD_STATUS marvell_mtd_xsmi_read(MTD_DEV *pMtdDev,
                                  MTD_U16 port,
                                  MTD_U16 dev,
                                  MTD_U16 reg,
                                  MTD_U16 *pdata)
{
    struct xsmi_dev *xsmi = (struct xsmi_dev *)pMtdDev->appData;
    return (0 > xsmi->drv->read(xsmi, port, dev, reg, pdata, 1)) ? MTD_FAIL : MTD_OK;
}

MTD_STATUS marvell_mtd_xsmi_write(MTD_DEV *pMtdDev,
                                  MTD_U16 port,
                                  MTD_U16 dev,
                                  MTD_U16 reg,
                                  MTD_U16 data)
{
    struct xsmi_dev *xsmi = (struct xsmi_dev *)pMtdDev->appData;
    return (0 > xsmi->drv->write(xsmi, port, dev, reg, &data)) ? MTD_FAIL : MTD_OK;
}


int marvell_mtd_xsmi_create(MTD_DEV *pMtdDev)
{

    int rc = 0;
    struct xsmi_dev *xsmi = NULL;
    rc = xsmi_dev_create(&xsmi, XSMI_REG_SPACE_ADDR, XSMI_REG_SPACE_SIZE, "xsmi-a");
    if (rc < 0) {
        cterr('f', 0, "[%s]%d: Error Create xsmi dev failed! rc=%d\n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    pMtdDev->appData = (MTD_PVOID)xsmi;

    return (PASSED);
}




#if MTD_CLAUSE_22_MDIO
int marvell_mtd_init_drv(MTD_DEV *pMtdDev)
{
    int rc = 0;
    MTD_BOOL macsecIndirectAccess = MTD_FALSE;
    FMTD_SEM_CREATE semCreate   = NULL;
    FMTD_SEM_DELETE semDelete   = NULL;
    FMTD_SEM_TAKE   semTake     = NULL; //marvell_mtd_sem_take;
    FMTD_SEM_GIVE   semGive     = NULL; //marvell_mtd_sem_give;
    MTD_U16         port        = MTD_PORT_0;

    rc = mtdLoadDriver((FMTD_READ_MDIO)marvell_mtd_smi_read,
            (FMTD_WRITE_MDIO)marvell_mtd_smi_write,
            macsecIndirectAccess,
            semCreate,
            semDelete,
            semTake,
            semGive,
            port,
            pMtdDev);
    if(rc != 0 ) {
        cterr('f', 0, "[%s]%d: Error load driver failed! rc=%d\n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }

    return (PASSED);
}
#else
int marvell_mtd_init_drv(MTD_DEV *pMtdDev)
{
    int rc = 0;
    MTD_BOOL macsecIndirectAccess = MTD_FALSE;
    FMTD_SEM_CREATE semCreate   = NULL;
    FMTD_SEM_DELETE semDelete   = NULL;
    FMTD_SEM_TAKE   semTake     = NULL; //marvell_mtd_sem_take;
    FMTD_SEM_GIVE   semGive     = NULL; //marvell_mtd_sem_give;
    MTD_U16         port        = MTD_PORT_0;

    rc = mtdLoadDriver((FMTD_READ_MDIO)marvell_mtd_xsmi_read,
            (FMTD_WRITE_MDIO)marvell_mtd_xsmi_write,
            macsecIndirectAccess,
            semCreate,
            semDelete,
            semTake,
            semGive,
            port,
            pMtdDev);
    if(rc != 0 ) {
        cterr('f', 0, "[%s]%d: Error load driver failed! rc=%d\n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }

    return (PASSED);
}
#endif


static int marvell_mtd_ram_firmware_download(MTD_DEV *pMtdDev, MTD_U16 port)
{

    MTD_U16 rc = 0;
    MTD_U16 errorCode;
    MTD_U8 *appData;
    size_t image_size;
    char *envFw = NULL;

    envFw = getenv("fw_3_7_16");
    if (envFw) {
        appData = (MTD_U8*) marvell_mtd_x3310fw_hdr_0_3_7_16;
        image_size = marvell_mtd_x3310fw_hdr_len_0_3_7_16;
    } else {
        appData = (MTD_U8*) marvell_mtd_x3310fw_hdr_0_3_7_0;
        image_size = marvell_mtd_x3310fw_hdr_len_0_3_7_0;
    }

    rc = mtdUpdateRamImage(pMtdDev, port, appData, image_size, &errorCode);
    if (MTD_OK == rc) {
        MTD_U8 major;
        MTD_U8 minor;
        MTD_U8 inc;
        MTD_U8 test;

        /* if the mtdUpdateRamImage() is successful, calls the mtdGetFirmwareVersion()
           to check and verify the updated version number */
        mtdWait(pMtdDev, 1000);
        mtdGetFirmwareVersion(pMtdDev, port, &major, &minor, &inc, &test);
        printf("RAM image loaded successful on mtd\n");
        printf("Firmware version: %d.%d.%d.%d\n", major, minor, inc, test);
    } else {
        cterr('f', 0, "[%s]%d: mtd update RamImage failed! rc=%d\n",
                __FUNCTION__, __LINE__, errorCode);
        return(FAILED);
    }

    ATTEMPT(mtdEnableTemperatureSensor(pMtdDev, port));

    return PASSED;
}

int marvell_mtd_revision_check(MTD_DEV *pMtdDev, MTD_U16 port)
{
    //0. Get Chip Revision through SDK API
    int rc = PASSED;
    rc = mtdGetPhyRevision(pMtdDev, port,
            &(pMtdDev->deviceId),
            &(pMtdDev->numPorts),
            &(pMtdDev->thisPort));
    if (rc != MTD_OK) {
        cterr('f', 0, "[%s]%d: mtd failed to get PhyRevison rc=%d\n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    printf("-----------------------Revision Info -----------------------------\n");
    printf("deviceId:                       0x%hx \n", pMtdDev->deviceId);
    printf("numPorts:                       %hd \n", pMtdDev->numPorts);

    //1. Get Chip Revision through register
    MTD_U16 device_identifier;
    MTD_U16 revision;
    MTD_U16 model_num;
    MTD_U16 uniq_id;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 1, 0x0003, 0, 16, &device_identifier));
    mtdHwGetRegFieldFromWord(device_identifier, 0, 4,&revision);
    mtdHwGetRegFieldFromWord(device_identifier, 4, 6,&model_num);
    mtdHwGetRegFieldFromWord(device_identifier, 10, 6,&uniq_id);
    printf("1.0003:                         0x%hx\n", device_identifier);
    printf("Chip revision:                  0x%hx\n", revision);
    printf("model_num:                      0x%hx\n", model_num);
    printf("uniq_id:                        0x%hx\n", uniq_id);

    //2. Get Firmware Revision and Bootup Status
    MTD_U8 major;
    MTD_U8 minor;
    MTD_U8 inc;
    MTD_U8 test;
    mtdGetFirmwareVersion(pMtdDev, port, &major, &minor, &inc, &test);
    printf("RAM image loaded successful on mtd\n");
    printf("Firmware version:               %d.%d.%d.%d\n", major, minor, inc, test);

    MTD_U16 boot_status;
    MTD_U16 progress;
    MTD_U16 app_started;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 1, 0xC050, 0, 16, &boot_status));
    mtdHwGetRegFieldFromWord(boot_status, 1, 1, &progress);
    mtdHwGetRegFieldFromWord(boot_status, 4, 1, &app_started);
    printf("1.C050:                         0x%hx\n", boot_status);
    printf("progress:                       0x%hx\n", progress);
    printf("appStarted:                     0x%hx\n", app_started);


    //3. Get 31.F000.[2:0] Media Select b'000:Copper_only or b'010 auto-media select
    //   31.F000.[12:13] MacSec Power
    printf("---------------------Mode & Port Config--------------------------\n");
    MTD_U16 mode_config;
    MTD_U16 media_select;
    MTD_U16 macsec_power;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 31, 0xF000, 0, 16, &mode_config));
    mtdHwGetRegFieldFromWord(mode_config, 0, 3, &media_select);
    mtdHwGetRegFieldFromWord(mode_config, 12, 1, &macsec_power);
    printf("31.F000:                        0x%hx\n", mode_config);
    printf("Media Select:                   0x%hx\n", media_select);
    printf("macsec_power:                   0x%hx\n", macsec_power);

    //4. Get 31.F001, bit_14 software reset, bit_2_0 Mac Type
    MTD_U16 port_control;
    MTD_U16 soft_reset;
    MTD_U16 mac_type;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 31, 0xF001, 0, 16, &port_control));
    mtdHwGetRegFieldFromWord(port_control, 14, 1, &soft_reset);
    mtdHwGetRegFieldFromWord(port_control, 0, 3, &mac_type);
    printf("31.F001:                        0x%hx\n", port_control);
    printf("soft_reset:                     0x%hx\n", soft_reset);
    printf("mac_type:                       0x%hx\n", mac_type);

    return PASSED;
}

MTD_STATUS marvell_mtd_wait_t_unit_low_power_mode
(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 timeoutMs,
    MTD_BOOL expctLowPowerMode
)
{
    MTD_U16 counter = 0;
    MTD_BOOL lowPowerMode;


    ATTEMPT(mtdIsTunitInLowPowerMode(pMtdDev,port,&lowPowerMode));

    while (lowPowerMode != expctLowPowerMode && counter < timeoutMs) {
        ATTEMPT(mtdWait(pMtdDev,1));
        ATTEMPT(mtdIsTunitInLowPowerMode(pMtdDev,port,&lowPowerMode));
        counter++;
    }

    if (counter > timeoutMs) {
        printf("X3310 int %d port %d timed out in waiting power mode %d  ready.\n",
                0, port, expctLowPowerMode);
        return MTD_FAIL; /* timed out without becoming ready */
    }
    return MTD_OK;
}


MTD_STATUS marvell_mtd_enable_sgmii_an_on(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{

    if(mode > MARVELL_MTD_MODE_1G) {
        cterr('f', 0, "[%s]%d: unsupported mode:%d\n",
                __FUNCTION__, __LINE__, mode);
        return(FAILED);
    }

    /*Keep copper link up.  This will prevent the link from toggling*/
    /*when you switch the host interface mode.                      */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x1F));
    /*config MAC to SGMII with auto-neg, 31.F001.2:0 = b'100 */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x8004));
    ATTEMPT(mtdWait(pMtdDev, 2000));

    //ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 15, 1,0x1));
    //ATTEMPT(mtdWait(pMtdDev, 100));

    /*Set 31.F045 back to default*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x0F));
    ATTEMPT(mtdWait(pMtdDev, 100));

    //set SGMII speed and enable AN
    ATTEMPT(mtdSet1000BXSGMIIControl(pMtdDev, port, MTD_H_UNIT,
                MTD_FALSE, mtd_sgmii_speed[mode], MTD_TRUE,
                MTD_FALSE, MTD_FALSE, MTD_TRUE));
    ATTEMPT(mtdWait(pMtdDev, 100));

    ATTEMPT(mtdRerunSerdesAutoInitialization(pMtdDev, port, MTD_H_UNIT));
    ATTEMPT(mtdWait(pMtdDev, 100));

    return 0;
}

MTD_STATUS marvell_mtd_enable_2500basex(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{

    /*Keep copper link up.  This will prevent the link from toggling*/
    /*when you switch the host interface mode.                      */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x1F));

    /*config MAC to SGMII without auto-neg,31.F001.2:0 = b'101 */
    /*config MAC to 2500Base-X 31.F001.2:0 = b'101 */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x8005));
    ATTEMPT(mtdWait(pMtdDev, 2000));

    //ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 15, 1,0x1));
    //ATTEMPT(mtdWait(pMtdDev, 100));

    /*Set 31.F045 back to default*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x0F));
    ATTEMPT(mtdWait(pMtdDev, 100));

    //set SGMII speed and disable AN
    /* 2500BASE-X is identical to 1000GBASE-X operation as described in section 6.2.4.1
     * except at 250% speed. Note that Auto-Negotiation is not supported in 2500BASE-X.
     * */
    ATTEMPT(mtdSet1000BXSGMIIControl(pMtdDev, port, MTD_H_UNIT,
                MTD_FALSE, MTD_SGMII_SPEED_1G, MTD_FALSE,
                MTD_FALSE, MTD_FALSE, MTD_TRUE));
    ATTEMPT(mtdWait(pMtdDev, 100));

    ATTEMPT(mtdRerunSerdesAutoInitialization(pMtdDev, port, MTD_H_UNIT));
    ATTEMPT(mtdWait(pMtdDev, 100));

    return 0;
}


/* support 1G/2.5G/5G/10G mode with AN on?*/
MTD_STATUS marvell_mtd_config_t_unit_speed(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{
    MTD_U16 speed_to_try = mtd_t_speed[mode];

    ATTEMPT(mtdRemoveTunitLowPowerMode(pMtdDev,port));
    ATTEMPT(marvell_mtd_wait_t_unit_low_power_mode(pMtdDev, port, 5000, MTD_FALSE));
    ATTEMPT(mtdWait(pMtdDev, 100));

    /* enable speed on port with mtdAutonegRestart*/
    ATTEMPT(mtdEnableSpeeds(pMtdDev, port, speed_to_try, MTD_FALSE));

    //T unit software reset instead of restart AN
    ATTEMPT(mtdSoftwareReset(pMtdDev, port,1000));

    return MTD_OK;
}


MTD_STATUS marvell_mtd_force_speed(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{
    MTD_U16 speed_to_try;

    switch (mode) {
        case MARVELL_MTD_MODE_10M:
            speed_to_try = MTD_SPEED_10M_FD_AN_DIS;
            break;

        case MARVELL_MTD_MODE_100M:
            speed_to_try = MTD_SPEED_100M_FD_AN_DIS;
            break;

        default:
            printf("%s[%d] Error: Invalid mode: 0x%x\n", __FUNCTION__, __LINE__, mode);
            return (FALSE);
            break;
    }

    ATTEMPT(mtdRemoveTunitLowPowerMode(pMtdDev,port));
    ATTEMPT(marvell_mtd_wait_t_unit_low_power_mode(pMtdDev, port, 5000, MTD_FALSE));

    printf("%s[%d] force to speed_to_try: 0x%x\n", __FUNCTION__, __LINE__, speed_to_try);
    /* enable speed on port with mtdAutonegRestart*/
    ATTEMPT(mtdForceSpeed(pMtdDev, port, speed_to_try));

    //T unit software reset instead of restart AN
    ATTEMPT(mtdSoftwareReset(pMtdDev, port,1000));

    return MTD_OK;
}


MTD_STATUS marvell_mtd_enable_sgmii_an_off(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{
    if(mode > MARVELL_MTD_MODE_1G) {
        cterr('f', 0, "[%s]%d: unsupported mode:%d\n",
                __FUNCTION__, __LINE__, mode);
        return(FAILED);
    }

    /*Keep copper link up.  This will prevent the link from toggling*/
    /*when you switch the host interface mode.                      */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x1F));
    /*config MAC to SGMII without auto-neg,31.F001.2:0 = b'101 */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x8005));
    ATTEMPT(mtdWait(pMtdDev, 2000));

    //ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 15, 1,0x1));
    //ATTEMPT(mtdWait(pMtdDev, 100));

    /*Set 31.F045 back to default*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF045, 0, 5, 0x0F));
    ATTEMPT(mtdWait(pMtdDev, 100));

    //set SGMII speed and disable AN
    ATTEMPT(mtdSet1000BXSGMIIControl(pMtdDev, port, MTD_H_UNIT,
                MTD_FALSE, mtd_sgmii_speed[mode], MTD_FALSE,
                MTD_FALSE, MTD_FALSE, MTD_TRUE));
    ATTEMPT(mtdWait(pMtdDev, 100));

    ATTEMPT(mtdRerunSerdesAutoInitialization(pMtdDev, port, MTD_H_UNIT));
    ATTEMPT(mtdWait(pMtdDev, 100));

    return 0;
}

MTD_STATUS marvell_mtd_config_mode(
    MTD_DEV *pMtdDev,
    MTD_U16 port,
    MTD_U16 mode)
{
    //Common code leveraged from starfleet
    ATTEMPT(mtdPutTunitInLowPowerMode(pMtdDev, port));
    ATTEMPT(marvell_mtd_wait_t_unit_low_power_mode(pMtdDev, port, 5000, MTD_TRUE));
    ATTEMPT(mtdTunitConfigure10GLinkDropCounter (pMtdDev,
                port, MTD_FALSE, MTD_FALSE,MTD_TRUE));
    ATTEMPT(mtdWait(pMtdDev, 100));

    //enable echo and FEXT Canceler to improve performance
    //ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC000, 0, 16, 0x0054));
    //enable FEXT Canceler to improve performance
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC000, 2, 1, 1));
    ATTEMPT(mtdWait(pMtdDev, 50));
    //enable echo to improve performance
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC000, 4, 2, 0x2));
    ATTEMPT(mtdWait(pMtdDev, 50));

    //optimze for EMC test
    if(MARVELL_MTD_MODE_10G == mode)
    {
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC040, 2, 1, 0x1));
        ATTEMPT(mtdWait(pMtdDev, 50));
    }else if( MARVELL_MTD_MODE_5G == mode){
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC040, 1, 1, 0x1));
        ATTEMPT(mtdWait(pMtdDev, 50));
    }else if( MARVELL_MTD_MODE_2P5G == mode){
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0xC040, 0, 1, 0x1));
        ATTEMPT(mtdWait(pMtdDev, 50));
    }

    //Send idles during fast retrain
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,1,0x0093, 0, 16, 0x0010));
    ATTEMPT(mtdWait(pMtdDev, 100));

    switch (mode) {
        case MARVELL_MTD_MODE_10M:
        case MARVELL_MTD_MODE_100M:
            printf("Warning: Only disable AN is verfied for 100M till now\n");

#if 0
            if (an) {
                printf("Enable AN\n");
                /* 1. Config T-Unit to SGMII with AN */
                ATTEMPT(marvell_mtd_config_t_unit_speed(pMtdDev, port, mode));

                /* 2. Config H-Unit to SGMII with AN */
                ATTEMPT(marvell_mtd_enable_sgmii_an_on(pMtdDev, port, mode));

                /* 3. Enable H-Unit packet checker */
                ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));
            } else {
#endif
                printf("Disable AN for 100M\n");
                /* 1. Config T-Unit to SGMII without AN */
                ATTEMPT(marvell_mtd_force_speed(pMtdDev, port, mode));

                /* 2. Config H-Unit to SGMII without AN */
                ATTEMPT(marvell_mtd_enable_sgmii_an_off(pMtdDev, port, mode));

                /* 3. Enable H-Unit packet checker */
                ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));
            break;


        case MARVELL_MTD_MODE_1G:
            printf("Warning: Only enable AN is verfied for 1G till now\n");
            printf("Enable AN for SGMII\n");
            /* 1. Config T-Unit to SGMII with AN */
            ATTEMPT(marvell_mtd_config_t_unit_speed(pMtdDev, port, mode));

            /* 2. Config H-Unit to SGMII with AN */
            ATTEMPT(marvell_mtd_enable_sgmii_an_on(pMtdDev, port, mode));

            /* 3. Enable H-Unit packet checker */
            ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));
#if 0
            } else {
                printf("Disable AN\n");
                /* 1. Config T-Unit to SGMII without AN */
                ATTEMPT(marvell_mtd_config_t_unit_speed(pMtdDev, port, mode));

                /* 2. Config H-Unit to SGMII without AN */
                ATTEMPT(marvell_mtd_enable_sgmii_an_off(pMtdDev, port, mode));

                /* 3. Enable H-Unit packet checker */
                ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));
            }
#endif

            break;

        case MARVELL_MTD_MODE_2P5G:

            /* 88X3310_88X3320_88X3340_88X3310P_88X3320P_88X3340P-Datasheet-Draft-Rev0-15_09162016.pdf
             * H-Unit:
             * 2500BASE-X PCS is enabled by setting register 31.F001.2:0 = 000, 011, 100, 101
             * and the line rate is 2.5 Gbps.
             * 2500BASE-X is identical to 1000GBASE-X operation as described in section 6.2.4.1
             * except at 250% speed. Note that Auto-Negotiation is not supported in 2500BASE-X.
             *
             * T-Unit:
             * Note that Auto-Negotiation must be enabled if the PHY wishes to operate in
             * 1000BASE-T, 2.5GBASE-T, 5GBASE-T, or 10GBASE-T.
             * Furthermore, the extended next page control bit(7.0000.13) must also be set to 1
             * if the PHY wishes to operate in 2.5GBASE-T, 5GBASE-T, and 10GBASE-T.
             */

            /* 1. Config Media Select */
            ATTEMPT(marvell_config_media_select(pMtdDev, port, MTD_COPPER_ONLY));

            /* 2. Config T-Unit to 2500Base-T with AN */
            ATTEMPT(marvell_mtd_config_t_unit_speed(pMtdDev, port, mode));

            /* 3. Config H-Unit to 2500Base-X without AN, same to SGMII without AN? */
            ATTEMPT(marvell_mtd_enable_2500basex(pMtdDev, port, mode));

            /* 4. Enable H-Unit packet checker */
            ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));

            break;

        case MARVELL_MTD_MODE_5G:

            /* 88X3310_88X3320_88X3340_88X3310P_88X3320P_88X3340P-Datasheet-Draft-Rev0-15_09162016.pdf
             * H-Unit:
             * 5GBASE-R PCS is enabled by setting register 31.F001.2:0 = 000, 011, 100, 101
             * and the line rate is 5 Gbps.
             * 5GBASE-R is identical to 10GBASE-R operation as described in section 6.2.1
             * except at 50% speed. 
             *
             * T-Unit:
             * Note that Auto-Negotiation must be enabled if the PHY wishes to operate in
             * 1000BASE-T, 2.5GBASE-T, 5GBASE-T, or 10GBASE-T.
             * Furthermore, the extended next page control bit(7.0000.13) must also be set to 1
             * if the PHY wishes to operate in 2.5GBASE-T, 5GBASE-T, and 10GBASE-T.
             */

            /* 1. Config Media Select */
            /* Control Unit (C Unit) 0xF000, bit[2:0] = 0, copper only */
            /* Control Unit (C Unit) 0xF001, bit15    = 1, SW reset    */
            ATTEMPT(marvell_config_media_select(pMtdDev, port, MTD_COPPER_ONLY));

            /* 2. Config T-Unit to 5GBase-T with AN */
            ATTEMPT(marvell_mtd_config_t_unit_speed(pMtdDev, port, mode));

            /* 3. Config H-Unit (MAC) to 2500Base-X without AN, same to SGMII without AN? */
            ATTEMPT(marvell_mtd_enable_2500basex(pMtdDev, port, mode));

            /* 4. Enable H-Unit packet checker */
            ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, 1));

            break; 

        case MARVELL_MTD_MODE_10G:
            cterr('f', 0, "[%s]%d: Error: Unsupported mode:%d !\n",
                    __FUNCTION__, __LINE__, mode);
            return(FAILED);
            break;

        default:
            cterr('f', 0, "[%s]%d: Error: Unsupported mode:%d !\n",
                    __FUNCTION__, __LINE__, mode);
            return(FAILED);
            break;
    }
    return MTD_OK;
}


int marvell_mtd_host_link_get(MTD_DEV *pMtdDev, uint32_t port)
{
    MTD_U16 sgmii_link_up = 0;
    if (port != 0) {
        cterr('f', 0, "[%s]%d: Error port number :%d !\n",
                __FUNCTION__, __LINE__, port);
        return(FALSE);
    }


    /* check SGMII real time link*/
    if (MTD_OK != mtdHwGetPhyRegField(pMtdDev, port, 4, 0xA003, 10, 1, &sgmii_link_up))
        return MARVELL_MTD_PORT_LINK_UNKNOWN;

    printf("[%s]:%d  4.A003 sgmii_link_up:0x%x\n", __FUNCTION__, __LINE__, sgmii_link_up);

    if (sgmii_link_up)
        return MARVELL_MTD_PORT_LINK_UP;
    else
        return MARVELL_MTD_PORT_LINK_DOWN;
}

int marvell_config_media_select(MTD_DEV *pMtdDev, MTD_U16 port, int media_select)
{
    /* MV-S302715-00-88X3310(P)+88X3340(P)-RevA1-Release-Notes.pdf:
     * Auto media support is only available for 10G/1G/100M/10M.
     * Auto-media is not supported for 2.5G and 5G speeds*/

    /* > 0x7 
     *     MTD_COPPER_ONLY = 0,
     *     MTD_FIBER_ONLY = 1,
     *     MTD_AUTO_MEDIA_COPPER_PRE = 2,
     *     MTD_AUTO_MEDIA_FIBER_PRE = 3,
     */
    if (media_select > MTD_AUTO_MEDIA) {
        cterr('f', 0, "[%s]%d: Error Media Select value:%d\n",
                __FUNCTION__, __LINE__, media_select);
        return (FAILED);
    }
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xf000, 0, 3, media_select));

    // toggle soft reset to take effect
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xf001, 15, 1, 1));
    mtdWait(pMtdDev, 100);

    return MTD_OK;
}



int highrise_init_phy_device(void)
{
    int rc = PASSED;
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;

    if (pMtd3310Dev->appData)
        return (PASSED);

    memset(pMtd3310Dev, 0, sizeof(*pMtd3310Dev));
#if MTD_CLAUSE_22_MDIO
    rc = marvell_mtd_smi_create(pMtd3310Dev);
#else
    rc = marvell_mtd_xsmi_create(pMtd3310Dev);
#endif
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }

    rc = marvell_mtd_init_drv(pMtd3310Dev);
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
#if MTD_CLAUSE_22_MDIO
        smi_dev_free(pMtd3310Dev->appData);
#else
        xsmi_dev_free(pMtd3310Dev->appData);
#endif
        pMtd3310Dev->appData = NULL;
        return(FAILED);
    }

    /* GPIO Data, register 0xF012 bit 2 = 0 ; 
     * GPIO Tristate Control, register 0xF013, bit 2 = 1 ;
     * ==> driver the GPIO2 data output state for SFP TX enable. 
     */
    mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF013, 2,  1, 0x1);
    mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF012, 2,  1, 0x0);

    return (PASSED);
}


static int phy_firmware_ram_download_utils(void)
{
    int rc = PASSED;
    MTD_U16 port = MTD_PORT_0;

    rc = marvell_mtd_ram_firmware_download(pMtd3310Dev, port);
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    return (PASSED);
}


struct phy_bin {
    char          *name;
    unsigned char *data;
    unsigned int   len;
};

static int _phy_bin_select(const struct phy_bin *bins, unsigned char **bin, size_t *len)
{
    int ret = FAILED;
    int num = 0;
    int idx = 0;
    char ch = 0;
    char binf[128] = {[0 ... sizeof(binf) - 1] = 0};
    FILE *fp = NULL;

    /* Ask for SPI flash image */
    _DRAIN_STDIN();
    for(idx = 0, num = 0; bins[idx].name; idx++, num++) {
        printf("      %d) %s\n", idx, bins[idx].name);
    }
    printf("  Choose the image:");
    fflush(stdout);
    ch = getchar();
    _DRAIN_STDIN();
    if (!(ch >= '0' && ch < '0' + num)) {
        cterr('f', 0, "[%s]%d:Invalid choice '%c'\n", __FUNCTION__, __LINE__, ch);
        goto _EXIT_POINT;
    }

    if (ch == '0' + num - 1) {
        printf("  Choose the file:");
        fflush(stdout);

        memset(binf, 0, sizeof(binf));
        fgets(binf, sizeof(binf) - 1, stdin);
        for(idx = strlen(binf) - 1; idx >= 0 && isspace(binf[idx]); binf[idx] = 0, idx--);
        for(idx = 0; idx < sizeof(binf) && isspace(binf[idx]); idx++);
        if (!binf[idx] || isspace(binf[idx])) {
            printf("Abort.\n");
            goto _EXIT_POINT;
        }

        if (NULL == (fp = fopen(&binf[idx], "rb"))) {
            cterr('f', 0, "[%s]%d:Open '%s' failed, err:%s!\n", __FUNCTION__, __LINE__, &binf[idx], strerror(errno));
            goto _EXIT_POINT;
        }

        fseek(fp, 0, SEEK_END);
        *len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        *bin = malloc(*len + 32);
        if (!*bin) {
            cterr('f', 0, "[%s]%d:Malloc failed!\n", __FUNCTION__, __LINE__);
            goto _EXIT_POINT;
        }
        memset(*bin, 0, *len + 32);

        *len = fread(*bin, sizeof(char), *len, fp);
        if (ferror(fp) || *len == 0) {
            cterr('f', 0, "[%s]%d:Read '%s' failed!\n", __FUNCTION__, __LINE__, binf);
            goto _EXIT_POINT;
        }
    } else {
        *len = bins[ch - '0'].len;
        *bin = malloc(*len + 32);
        if (!*bin) {
            cterr('f', 0, "[%s]%d:Malloc failed!\n", __FUNCTION__, __LINE__);
            goto _EXIT_POINT;
        }
        memset(*bin, 0, *len + 32);
        memcpy(*bin, bins[ch - '0'].data, *len);
    }

    ret = PASSED;
_EXIT_POINT:
    if (ret != PASSED) {
        if (*bin) {
            free(*bin);
            *bin = NULL;
        }
        *len = 0;
    }
    if (fp)
        fclose(fp);
    return ret;
}

static int phy_firmware_flash_program_utils(void)
{
    MTD_U8 *appData = NULL;
    size_t appSize  = NULL;
    MTD_U8 *slaveCode = NULL;
    size_t slaveSize  = 0;
    char ch = 0;
    int ret = FAILED;
    MTD_STATUS retStatus;
    MTD_U16 error;

    const struct phy_bin fws[] = {
        {"3310fw_0_3_7_16", marvell_mtd_x3310fw_hdr_0_3_7_16, marvell_mtd_x3310fw_hdr_len_0_3_7_16},
        {"3310fw_0_3_7_0" , marvell_mtd_x3310fw_hdr_0_3_7_0 , marvell_mtd_x3310fw_hdr_len_0_3_7_0 },
        {"3310fw_0_3_10_0", marvell_mtd_x3310fw_hdr_0_3_10_0, marvell_mtd_x3310fw_hdr_len_0_3_10_0},
        {"From file ..."  , NULL, 0},
        {NULL, NULL, 0},
    };

    const struct phy_bin helpers[] = {
        {"x3240flashdlslave_0_6_5_0", marvell_mtd_x3240flashdlslave_0_6_5_0, marvell_mtd_x3240flashdlslave_0_6_5_0_len},
        {"From file ..."  , NULL, 0},
        {NULL, NULL, 0},
    };

    printf("\nPlease specify the helper image\n");
    if (_phy_bin_select(helpers, &slaveCode, &slaveSize) != PASSED)
        goto _EXIT_POINT;

    /* Ask for SPI flash image */
    printf("\nPlease specify the firmware image\n");
    if (_phy_bin_select(fws, &appData, &appSize) != PASSED)
        goto _EXIT_POINT;

    _DRAIN_STDIN();
    printf("Double confirm to program the flash for phy(y/N):");
    fflush(stdout);
    ch = getchar();
    _DRAIN_STDIN();
    if (ch == 'y' || ch == 'Y') {
        retStatus = mtdUpdateFlashImage(pMtd3310Dev, MTD_PORT_0, appData, appSize, slaveCode, slaveSize, &error);
        if (retStatus == MTD_FAIL) {
            cterr('f', 0, "[%s]%d:Program flash image failed, error code:0x%x\n", __FUNCTION__, __LINE__, error);
            goto _EXIT_POINT;
        }
        /* Removes download mode from all ports and reloads all
        ** ports with the new code from flash
        */
        mtdChipHardwareReset(pMtd3310Dev, MTD_PORT_0);
    }

    ret = PASSED;
_EXIT_POINT:
    if (appData)
        free(appData);
    if (slaveCode)
        free(slaveCode);
    return ret;
}

int phy_config_mode_100m_utils(void)
{
    int rc = PASSED;

    rc = marvell_mtd_config_mode(pMtd3310Dev, MTD_PORT_0, MARVELL_MTD_MODE_100M);
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    return (PASSED);
}
int phy_config_mode_sgmii_utils(void)
{
    int rc = PASSED;

    rc = marvell_mtd_config_mode(pMtd3310Dev, MTD_PORT_0, MARVELL_MTD_MODE_1G);
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    return (PASSED);
}

int phy_config_mode_2P5G_utils(void)
{
    int rc = PASSED;

    rc = marvell_mtd_config_mode(pMtd3310Dev, MTD_PORT_0, MARVELL_MTD_MODE_2P5G);
    if (PASSED != rc) {
        cterr('f', 0, "[%s]%d: Error! return code:%d \n",
                __FUNCTION__, __LINE__, rc);
        return(FAILED);
    }
    return (PASSED);
}


int phy_change_mtd_verbose_utils(void)
{

    printf("Input mtd verbosity level: 0-OFF, 1-ERR, 2-INF, 3-ALL\n");

    mtd_debug_level = getdec_answer("\nEnter debug level",
            mtd_debug_level, MTD_DBG_OFF_LVL, MTD_DBG_ALL_LVL);

    printf("mtd_debug_level:%d\n ", mtd_debug_level);

    return 0;
}

int marvell_dump_link_status(MTD_DEV *pMtdDev, MTD_U16 port)
{
    /* H unit 10GBase-R status */
    MTD_BOOL tx_lpi_latch;
    MTD_BOOL rx_lpi_latch;
    MTD_BOOL tx_lpi_current;
    MTD_BOOL rx_lpi_current;
    MTD_BOOL fault;
    MTD_BOOL h_link_status_latch;
    MTD_BOOL h_link_status_current;

    /* H unit SGMI status */
    MTD_U16 autoSpeedDetected;
    MTD_BOOL an_complete;
    MTD_BOOL remote_fault;
    MTD_BOOL link_status_latched;
    MTD_BOOL link_status_current;
    MTD_U16 sgmi_reg;
    MTD_U16 sgmi_energy;
    MTD_U16 sgmi_sync;
    MTD_U16 sgmi_speed_du_resolved;
    MTD_U16 sgmi_speed;
    MTD_U16 sgmi_link;

    /* T unit */
    MTD_U16 t_speed;
    MTD_U16 t_an_reg;
    MTD_BOOL t_linkup;
    MTD_U16 t_an_complete;
    MTD_U16 t_remote_fault;
    MTD_U16 t_energy;

    int i = 0;
    char* bool_str[2] = {"FALSE", "TRUE"};
    char* sgmii_speed_desc[4] = {
        [0]              = "10 Mbps",
        [1]              = "100 Mbps",
        [2]              = "1000 Mbps",
        [3]              = "Unkown",
    };

    static char* host_speed_desc[16] = {
        [0 ... MTD_SERDES_SPEED_GIG-1]      = "unknown",
        [MTD_SERDES_SPEED_GIG]              = "SGMII",
        [MTD_SERDES_SPEED_RXAUI]            = "RXAUI",
        [MTD_SERDES_SPEED_XFI]              = "XFI",
        [MTD_SERDES_SPEED_2P5G]             = "2.5GBASE-X",
        [MTD_SERDES_SPEED_5G ]              = "5GBASE-R",
        [MTD_SERDES_SPEED_XAUI]             = "XAUI",
        [MTD_SERDES_SPEED_XAUI+1 ... 15]    = "unknown",
    };


    /*-----------------------H-Unit Status ---------------------*/
    // Doc: Basic-Debug-Guide-for-88X3xx ... Doc No.MV-S302891-00
    // H-Unit Link Status for 1000Base-X/SGMII/2500Base-X
    for (i = 0; i < 3; i ++) {
        // Always read 4.A003 several times to ensure it's consistent.
        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 4, 0x2001, 0, 16, &sgmi_reg));
        printf("[%s]:%d, 4.2001 = 0x%x\n", __FUNCTION__, __LINE__, sgmi_reg);

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 4, 0xA003, 0, 16, &sgmi_reg));
        printf("[%s]:%d, 4.A003 = 0x%x\n", __FUNCTION__, __LINE__, sgmi_reg);
    }
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 4, 0xA003, 0, 16, &sgmi_reg));
    mtdHwGetRegFieldFromWord(sgmi_reg, 4, 1, &sgmi_energy);
    mtdHwGetRegFieldFromWord(sgmi_reg, 5, 1, &sgmi_sync);
    mtdHwGetRegFieldFromWord(sgmi_reg, 11, 1, &sgmi_speed_du_resolved);
    mtdHwGetRegFieldFromWord(sgmi_reg, 14, 2, &sgmi_speed);
    mtdHwGetRegFieldFromWord(sgmi_reg, 10, 1, &sgmi_link);
    char *host_link_str = sgmi_link ? "UP" : "DOWN";

    ATTEMPT(mtdGetSerdesAutoInitSpeed(pMtdDev, port, MTD_H_UNIT,
                &autoSpeedDetected));
    ATTEMPT(mtdGet1000BXSGMIIStatus (pMtdDev, port, MTD_H_UNIT,
                &an_complete, &remote_fault, &link_status_latched,
                &link_status_current));
    ATTEMPT(mtdGet10GBRStatus1(pMtdDev, port, MTD_H_UNIT,
                &tx_lpi_latch, &rx_lpi_latch,
                &tx_lpi_current, &rx_lpi_current, &fault,
                &h_link_status_latch, &h_link_status_current));
    printf("-----------------------Host Side-----------------------------\n");
    printf("link:                          %s\n", host_link_str);
    printf("auto_speed_detect:             %s by mtdGetSerdesAutoInitSpeed()\n", host_speed_desc[autoSpeedDetected]);
    printf("\n");
    printf("10gbr_fault:                   %s\n", bool_str[fault]);
    printf("10gbr_tx_lpi_latch:            %s\n", bool_str[tx_lpi_latch]);
    printf("10gbr_rx_lpi_latch:            %s\n", bool_str[rx_lpi_latch]);
    printf("10gbr_tx_lpi_current:          %s\n", bool_str[tx_lpi_current]);
    printf("10gbr_rx_lpi_current:          %s\n", bool_str[rx_lpi_current]);
    printf("10gbr_link_status_latch:       %s\n", bool_str[h_link_status_latch]);
    printf("10gbr_link_status_current:     %s\n", bool_str[h_link_status_current]);
    printf("\n");

    printf("sgmii_an_complete:             %s\n", bool_str[an_complete]);
    printf("sgmii_remote_fault:            %s\n", bool_str[remote_fault]);
    printf("sgmii_link_status_latched:     %s\n", bool_str[link_status_latched]);
    printf("sgmii_link_status_current:     %s\n", bool_str[link_status_current]);
    printf("sgmii_sync_status:             %s\n", bool_str[sgmi_sync]);
    printf("sgmii_energy_detected:         %s\n", bool_str[!sgmi_energy]);
    printf("sgmii_speed_du_resolved:       %s\n", bool_str[sgmi_speed_du_resolved]);
    printf("sgmii_speed:                   %s by 4.A003.[15:14]\n", sgmii_speed_desc[sgmi_speed]);
    printf("-------------------------------------------------------------\n");
    printf("\n");



    /*-----------------------T-Unit Status ---------------------*/
    char* line_link_strs[2] = {"DOWN", "UP"};
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 3, 0x8008, 4, 1, &t_energy));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0001, 0, 16, &t_an_reg));
    mtdHwGetRegFieldFromWord(t_an_reg, 4, 1,&t_remote_fault);
    mtdHwGetRegFieldFromWord(t_an_reg, 5, 1,&t_an_complete);
    ATTEMPT(mtdIsBaseTUp(pMtdDev, port, &t_speed, &t_linkup));

    printf("-----------------------Line Side-----------------------------\n");
    printf("link:                          %s\n", line_link_strs[t_linkup]);
    printf("speed:                         %s\n", mtd_speed_desc[t_speed]);
    printf("an_complete:                   %s\n", bool_str[t_an_complete]);
    printf("remote_fault:                  %s\n", bool_str[t_remote_fault]);
    printf("energy_detected:               %s\n", bool_str[!t_energy]);
    printf("-------------------------------------------------------------\n");
    printf("\n");

    return(PASSED);
}

int marvell_dump_loopback_status(MTD_DEV *pMtdDev, MTD_U16 port)
{
    char* bool_str[2] = {"FALSE", "TRUE"};
    MTD_LOOPBACKS_STATUS_STRUCT lpbk;
    MTD_U16 deep_lpbk = 0;

    ATTEMPT(mtdGetLoopbackSetting(pMtdDev, port, &lpbk));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 3, 0x8200, 14, 1,  &deep_lpbk));
    printf("-----------------------Loop Back-----------------------------\n");
    printf("tunitMacLoopback:              %s\n", bool_str[lpbk.tunitMacLoopback]);
    printf("tunitLineLoopback2P5GAbove:    %s\n", bool_str[lpbk.tunitLineLoopback2P5GAbove]);
    printf("tunitLineLoopback1GBelow:      %s\n", bool_str[lpbk.tunitLineLoopback1GBelow]);
    printf("hunitMacLoopback:              %s\n", bool_str[lpbk.hunitMacLoopback]);
    printf("hunitLineLoopback:             %s\n", bool_str[lpbk.hunitLineLoopback]);
    printf("hunitLinePassThrough:          %s\n", bool_str[lpbk.hunitLinePassThrough]);
    printf("3.8200.14:                     %s\n", bool_str[deep_lpbk]);
    printf("-------------------------------------------------------------\n");
    printf("\n");

    return(PASSED);
}


int phy_check_status_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;

    char* bool_str[2] = {"FALSE", "TRUE"};
    //Phy revision information check
    (void)marvell_mtd_revision_check(pMtdDev, port);
    (void)marvell_dump_loopback_status(pMtdDev, port);
    (void)marvell_dump_link_status(pMtdDev, port);

    /*-------------------------Power Mode Status ---------------------*/
    MTD_U16 t_power_mode_1;
    MTD_U16 t_power_mode_2;
    MTD_U16 h_power_mode_basex;
    MTD_U16 c_power_mode;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 1, 0x0000, 11, 1, &t_power_mode_1));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 3, 0x0000, 11, 1, &t_power_mode_2));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 4, 0x2000, 11, 1, &h_power_mode_basex));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 31, 0xF001, 11, 1, &c_power_mode));
    printf("-----------------------Power Status---------------------------\n");
    printf("T-Unit Low Power:               %s\n", bool_str[t_power_mode_1]);
    printf("T-Unit Low Power:               %s\n", bool_str[t_power_mode_2]);
    printf("H-Unit BASE-X Low Power:        %s\n", bool_str[h_power_mode_basex]);
    printf("C-Unit Low Power:               %s\n", bool_str[c_power_mode]);
    printf("-------------------------------------------------------------\n");
    printf("\n");


    /*-------------------------Speed Advertisement Status ---------------------*/
    MTD_U16 mgBaseT_an;
    MTD_U16 an_10g_fd_cap;
    MTD_U16 an_5g_ability;
    MTD_U16 an_2p5g_ability;

    MTD_U16 an_1g_10m_an;
    MTD_U16 an_1g_fd_cap;
    MTD_U16 an_1g_hd_cap;

    MTD_U16 an_adver;
    MTD_U16 an_10m_hd_cap;
    MTD_U16 an_10m_fd_cap;
    MTD_U16 an_100m_fd_cap;
    MTD_U16 an_100m_hd_cap;

    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0020, 0, 16, &mgBaseT_an));
    mtdHwGetRegFieldFromWord(mgBaseT_an, 12, 1, &an_10g_fd_cap);
    mtdHwGetRegFieldFromWord(mgBaseT_an, 8, 1, &an_5g_ability);
    mtdHwGetRegFieldFromWord(mgBaseT_an, 7, 1, &an_2p5g_ability);
    printf("7.0020 mgBaseT_an:0x%hx\n", mgBaseT_an);

    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x8000, 0, 16, &an_1g_10m_an));
    mtdHwGetRegFieldFromWord(an_1g_10m_an, 9, 1, &an_1g_fd_cap);
    mtdHwGetRegFieldFromWord(an_1g_10m_an, 8, 1, &an_1g_hd_cap);
    printf("7.8000 an_1g_10m_an: 0x%hx\n", an_1g_10m_an);

    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0010, 0, 16, &an_adver));
    mtdHwGetRegFieldFromWord(an_adver, 5, 1, &an_10m_hd_cap);
    mtdHwGetRegFieldFromWord(an_adver, 6, 1, &an_10m_fd_cap);
    mtdHwGetRegFieldFromWord(an_adver, 7, 1, &an_100m_fd_cap);
    mtdHwGetRegFieldFromWord(an_adver, 8, 1, &an_100m_hd_cap);
    printf("7.0010 an_adver: 0x%hx\n", an_adver);

    printf("-----------------------Advertisement---------------------------\n");
    printf("10g Full Duplex Advertisement:        %hd\n", an_10g_fd_cap);
    printf("5gBASE-T ability Advertisement:       %hd\n", an_5g_ability);
    printf("2.5GBASE-T ability Advertisement:     %hd\n", an_2p5g_ability);
    printf("1000BASE_T Full Duplex Advertisement  %hd\n", an_1g_fd_cap);
    printf("1000BASE-T Half Duplex Advertisement  %hd\n", an_1g_hd_cap);
    printf("100BASE-X Full Duplex Advertisement   %hd\n", an_100m_fd_cap);
    printf("100BASE-X Half Duplex Advertisement   %hd\n", an_100m_hd_cap);
    printf("10BASE-X Full Duplex Advertisement    %hd\n", an_10m_fd_cap);
    printf("10BASE-X Half Duplex Advertisement    %hd\n", an_10m_hd_cap);
    printf("-------------------------------------------------------------\n");
    printf("\n");


    /*-------------------------Downshift Control ---------------------*/
    MTD_U16 downshift_1;
    MTD_U16 downshift_2;
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 1, 0xc034, 0, 16, &downshift_1));
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 1, 0xc035, 0, 16, &downshift_2));
    printf("-----------------------Downshift Control---------------------------\n");
    printf("1.c034:                                0x%hx \n", downshift_1);
    printf("1.c035:                                0x%hx \n", downshift_2);


    /*-------------------------Pkt Checker ---------------------*/
    MTD_U16 temp;
    printf("------------------------Pkt Checker Reg dump ------------------\n");
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev,port,MTD_TUNIT_PKTGEN_CTRL,0,16,&temp));
    printf("3.8030:                                0x%hx for 1G\n", temp);

    ATTEMPT(mtdHwGetPhyRegField(pMtdDev,port,MTD_TUNIT_10G_PKTCHK_CNTL,0,16,&temp));
    printf("3.DCA0:                                0x%hx for 2.5G\n", temp);
    ATTEMPT(mtdHwGetPhyRegField(pMtdDev,port,MTD_TUNIT_TEST_CTRL,0,16,&temp));
    printf("1.C00B:                                0x%hx for 2.5G\n", temp);

    ATTEMPT(mtdHwGetPhyRegField(pMtdDev,port,4,MTD_PKT_GEN_CONTROL1,0,16,&temp));
    printf("4.f010:                                0x%hx for H-Unit\n", temp);

    return PASSED;

}


int marvell_mtd_1g_deep_mac_loopback_lkdn(
        MTD_DEV *pMtdDev, 
        MTD_U16 port, 
        MTD_U16 mode,
        int enable)
{
    if(enable) {
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF000, 6,  2, 0x2));
        //Release Note, toggle soft-reset twice
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF001, 15, 1, 0x1));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF001, 15, 1, 0x1));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7,  0x2020, 14, 2, 0x3));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7,  0x2000, 15, 1, 0x1));

        mtdWait(pMtdDev, 700);
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0x8FF6, 0,  16, 0x9021));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0x8200, 14, 1,  0x1));
    } else {
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0x8FF6, 0,  16, 0x1021));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0x8200, 14, 1,  0x0));

        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7,  0x2020, 14, 2, 0x0));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7,  0x2000, 15, 1, 0x0));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF000, 6,  2, 0x3));
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF001, 15, 1, 0x1));
    }

    return(PASSED);
}

int marvell_mtd_enable_deep_mac_loopback(
        MTD_DEV *pMtdDev, 
        MTD_U16 port, 
        MTD_U16 speed, 
        int enable)
{
    if (mtdSetTunitDeepMacLoopback(pMtdDev, port, speed, enable)) {
        cterr('f', 0, "[%s]%d: speed:%s, enable:%d\n",
                __FUNCTION__, __LINE__, mtd_speed_desc[speed], enable);
        return (FAILED);
    }
    return(PASSED);
}

int marvell_debug_lkdn_deep_lpbk(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 mode,
        int enable)
{
    ATTEMPT(marvell_mtd_1g_deep_mac_loopback_lkdn(pMtdDev, port, mode, enable));
    return(PASSED);
}

int marvell_mtd_enable_shallow_mac_loopback(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        int enable)
{
    //Shallow Mac loopback
    ATTEMPT(mtdSetHXunitMacLoopback(pMtdDev, port, MTD_H_UNIT, port, enable));
    return(PASSED);
}

int marvell_debug_lkdn_shallow_lpbk(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 mode,
        int enable)
{
    printf("\n Warning: mtdSetShallowMacLoopbackLinkDown Only support 1G mode\n");
    ATTEMPT(mtdSetShallowMacLoopbackLinkDown(pMtdDev, port, MTD_SPEED_1GIG_FD, enable));
    return(PASSED);
}


int phy_config_loopback_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 enable = 0;
    MTD_U16 level = 1;
    MTD_U16 speed;

    printf("Shallow LPBK - 0, Deep LPBK - 1\n");
    level = getdec_answer("Shallow/Deep", level, 0, 1);
    printf("Disable LPBK - 0, Enable LPBK - 1\n");
    enable = getdec_answer("Disable/Enable", enable, 0, 1);
 
    ATTEMPT(phy_get_t_link_speed(pMtdDev, port, &speed));

    switch (level) {
        case 0:
            //Shallow Mac loopback
            ATTEMPT(marvell_mtd_enable_shallow_mac_loopback(pMtdDev, port, enable));
            break;

        case 1:
            //Deep Mac loopback
            ATTEMPT(marvell_mtd_enable_deep_mac_loopback(pMtdDev, port, speed, enable));
            break;

        default:
            printf("[%s]:%d, parameter error!\n", __FUNCTION__, __LINE__);
            return(FALSE);
            break;
    }

    (void)marvell_dump_loopback_status(pMtdDev, port);

    return(PASSED);
}

int phy_debug_lkdn_lpbk_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 enable = 0;
    MTD_U16 level = 1;
    MTD_U16 mode = MARVELL_MTD_MODE_1G;

    printf("*** Debug 1G LPBK for link down status ***\n");
    printf("Shallow LPBK - 0, Deep LPBK - 1\n");
    level = getdec_answer("Shallow/Deep", level, 0, 1);
    printf("Disable LPBK - 0, Enable LPBK - 1\n");
    enable = getdec_answer("Disable/Enable", enable, 0, 1);

    switch (level) {
        case 0:
            ATTEMPT(marvell_debug_lkdn_shallow_lpbk(pMtdDev, port, mode, enable));
            break;

        case 1:
            //Deep Mac loopback
            ATTEMPT(marvell_debug_lkdn_deep_lpbk(pMtdDev, port, mode, enable));
            break;

        default:
            printf("[%s]:%d, parameter error!\n", __FUNCTION__, __LINE__);
            break;
    }

    (void)marvell_dump_loopback_status(pMtdDev, port);

    return PASSED;
}

int phy_config_tunit_powermode_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 mode = 0;
    MTD_BOOL inLowPowerMode;


    printf("TUnit LowPower-0, TUnit NormalPower-1, Phy LowPower-2, Phy NormalPower-3\n");
    mode = getdec_answer("Power Mode", mode, 0, 3);

    switch (mode) {
        case 0: //T-unit low power mode
            mtdIsTunitInLowPowerMode(pMtdDev, port, &inLowPowerMode);
            if (inLowPowerMode) {
                printf("T-Unit is already in %s mode", (mode ? "Normal power mode": "Low power mode"));
                return PASSED;
            }

            mtdPutTunitInLowPowerMode(pMtdDev, port);

            break;

        case 1: //T-unit normal power mode
            mtdRemoveTunitLowPowerMode(pMtdDev, port);
            break;

        case 2: //Phy low Power mode
            mtdPutPhyInLowPowerMode(pMtdDev, port);
            break;

        case 3: //Phy normal power mode
            mtdRemovePhyLowPowerMode(pMtdDev, port);
            break;
    }

    return PASSED;
}

static int marvell_mtd_10g_send_stop(
        MTD_DEV *pMtdDev,
        int port)
{

    /* stop T unit send*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0xDC95, 0, 16, 0x0));
    ATTEMPT(mtdWait(pMtdDev, 100));
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0xDC90, 0, 16, 0x100));
    ATTEMPT(mtdWait(pMtdDev, 100));

    return (PASSED);
}

static int marvell_mtd_1g_send_stop(
        MTD_DEV *pMtdDev,
        int port)
{
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0x8034, 0, 16, 0x0));
    ATTEMPT(mtdWait(pMtdDev, 100));

    return(PASSED);
}

int marvell_mtd_h_send_stop(
        MTD_DEV *pMtdDev,
        int port)
{
    //stop H unit
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 4, 0xF017, 0, 16, 0x0));
    ATTEMPT(mtdWait(pMtdDev, 50));
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 4, 0xF010, 1, 1, 0x0));
    ATTEMPT(mtdWait(pMtdDev, 50));
    return(PASSED);
}

static int marvell_mtd_traf_10g_counter_clear(
        MTD_DEV *pMtdDev,
        int port)
{

    MTD_U16 speed_bits = MTD_SPEED_2P5GIG_FD;
    /* enable T-unit packet generator */
    /* if not, T-unit tx counter is invalid and can't be cleared */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0xDC90, 0, 16, 0x103));
    ATTEMPT(mtdWait(pMtdDev, 100));

    /* T unit counter clear*/
    ATTEMPT(mtdTunitPktGeneratorCounterReset(pMtdDev, port, speed_bits));

    /*disable T-unit packet generator*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 3, 0xDC90, 0, 16, 0x100));
    ATTEMPT(mtdWait(pMtdDev, 100));


    /* enable H-unit packet generator */
    /* if not, H-unit tx counter is invalid and can't be cleared */
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 4, 0xF010, 1, 1, 0x1));
    ATTEMPT(mtdWait(pMtdDev, 50));

    /* H unit counter clear*/
    ATTEMPT(mtdPktGeneratorCounterReset( pMtdDev, port, MTD_H_UNIT));
    ATTEMPT(mtdWait(pMtdDev, 50));

    /*disable H-unit packet generator*/
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 4, 0xF010, 1, 1, 0x0));
    ATTEMPT(mtdWait(pMtdDev, 50));

    return 0;
}

static int marvell_mtd_traf_1g_counter_clear(
        MTD_DEV *pMtdDev,
        int port)
{
    MTD_U16 speed_bits = MTD_SPEED_1GIG_FD;

    /* T unit counter clear*/
    ATTEMPT(mtdTunitPktGeneratorCounterReset( pMtdDev, port, speed_bits));
    /* H unit counter clear*/
    ATTEMPT(mtdPktGeneratorCounterReset( pMtdDev, port, MTD_H_UNIT));

    return 0;
}


int phy_clear_pkt_counter_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    int port = MTD_PORT_0;
    int mode = 0;

    printf("pls input mode : 1G/2.5G\n");
    mode = getdec_answer("1G(0)/2.5G(1)", mode, 0, 1);

    if (mode == 0) {
        ATTEMPT(marvell_mtd_1g_send_stop(pMtdDev, port));
        return marvell_mtd_traf_1g_counter_clear(pMtdDev, port);
    } else {
        ATTEMPT(marvell_mtd_h_send_stop(pMtdDev, port));
        ATTEMPT(marvell_mtd_10g_send_stop(pMtdDev, port));
        return marvell_mtd_traf_10g_counter_clear(pMtdDev, port);
    }
    return (PASSED);
}


static int marvell_mtd_enable_h_unit_pkt_checker(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        int enable)
{
    if (enable) {
        // enalbe H unit rx counter
        ATTEMPT(mtdPktGeneratorCounterReset(pMtdDev, port, MTD_H_UNIT));
        ATTEMPT(mtdEnablePktGeneratorChecker (pMtdDev, port, MTD_H_UNIT, MTD_FALSE, MTD_TRUE));
        ATTEMPT(mtdWait(pMtdDev, 100));
    } else {
        // Disable H unit rx counter
        ATTEMPT(mtdPktGeneratorCounterReset(pMtdDev, port, MTD_H_UNIT));
        ATTEMPT(mtdEnablePktGeneratorChecker (pMtdDev, port, MTD_H_UNIT, MTD_FALSE, MTD_FALSE));
        ATTEMPT(mtdWait(pMtdDev, 100));
    }
    return MTD_OK;
}

static int marvell_mtd_enable_t_unit_pkt_checker(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 speed,
        int dir,
        int enable)
{
    int checker[2] = {MTD_CHECKER_FROM_LINE, MTD_CHECKER_FROM_MAC};

    // enable T unit rx counter
    ATTEMPT(mtdTunitPktGeneratorCounterReset(pMtdDev, port, speed));

    if (speed > MTD_SPEED_1GIG_FD) {
        //select checher dir
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, MTD_TUNIT_TEST_CTRL,
                    3, 1, checker[dir]));
        ATTEMPT(mtdWait(pMtdDev, 100));
    }

    if (enable) {
        ATTEMPT(mtdTunitEnablePktGeneratorChecker(pMtdDev, port, speed,
                    MTD_FALSE,     //disableGenerator,
                    MTD_TRUE       //enableChecker
                    ));
    } else {
        ATTEMPT(mtdTunitEnablePktGeneratorChecker(pMtdDev, port, speed,
                    MTD_FALSE,     //disableGenerator,
                    MTD_FALSE      //enableChecker
                    ));
    }
    return MTD_OK;

}

static int marvell_mtd_diag_common_counter_en(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 speed,
        int dir,
        int enable)
{
    ATTEMPT(marvell_mtd_enable_h_unit_pkt_checker(pMtdDev, port, enable));
    ATTEMPT(marvell_mtd_enable_t_unit_pkt_checker(pMtdDev, port, speed, dir, enable));
    return MTD_OK;
}


int phy_enable_pkt_counter_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 speed_bits = 0;
    MTD_U16 mode = 0;
    int dir = 0;
    int enable = 0;

    printf("\n");
    printf("1G mode,   T-unit checker only support Line-to-host direction,\n \
                       H-unit checker only support Host-to-Line?\n");
    printf("\n");
    printf("2.5G mode, T-unit checker support Line-to-host (0) OR host-to-Line(1),\n \
                       H-unit checker only support Host-to-Line\n");
    printf("\n");

    printf("pls input mode: 1G/2.5G\n");
    mode = getdec_answer("1G(0)/2.5G(1)", mode, 0, 1);
    if (mode == 0) {
        speed_bits = MTD_SPEED_1GIG_FD;
    } else {
        speed_bits = MTD_SPEED_2P5GIG_FD;
        printf("Pls input line-to-host(0)/host-to-line(1) direction\n");
        dir = getdec_answer("line-to-host(0)/host-to-line(1)", dir, 0, 1);
    }

    printf("pls input dis/en pkt counter\n");
    enable = getdec_answer("dis(0)/en(1)", enable, 0, 1);

    return marvell_mtd_diag_common_counter_en(pMtdDev, port, speed_bits, dir, enable);

}


int phy_check_pkt_counter_utils(void)
{
    MTD_U64 txPktCnt = 0;
    MTD_U64 txByteCnt = 0;
    MTD_U64 rxPktCnt = 0;
    MTD_U64 rxByteCnt = 0;
    MTD_U64 rxErrPktCnt = 0;
    MTD_U64 rxErrByteCnt = 0;
    MTD_DEV_PTR pMtdDev = pMtd3310Dev;
    int port = MTD_PORT_0;
    MTD_U16 speed_bits = 0;
    MTD_U16 speed = 0;

    //get speed through advertisement
    printf("pls input speed: 1G/2.5G\n");
    speed = getdec_answer("1G(0)/2.5G(1)", speed, 0, 1);
    if (speed == 0) {
        speed_bits = MTD_SPEED_1GIG_FD;
    } else {
        speed_bits = MTD_SPEED_2P5GIG_FD;
    }

    printf("\n");
    printf("port\ttxPkgCnt\t\ttxByteCnt\t\trxPkgCnt\t\trxByteCnt\t\trxErrCnt\n");
    printf("--------------------------------------------------------------\n");
    printf("-----------------T-Unit--------------------------------------\n");

    // T unit counter
    //tx counter
    ATTEMPT(mtdTunitPktGeneratorGetCounter( pMtdDev, port, speed_bits,
                MTD_PKT_GET_TX,  &txPktCnt, &txByteCnt));
    //rx counter
    ATTEMPT(mtdTunitPktGeneratorGetCounter( pMtdDev, port, speed_bits,
                MTD_PKT_GET_RX,  &rxPktCnt, &rxByteCnt));
    //err counter
    ATTEMPT(mtdTunitPktGeneratorGetCounter( pMtdDev, port, speed_bits,
                MTD_PKT_GET_ERR,  &rxErrPktCnt, &rxErrByteCnt));
    printf("%d\t%016llx\t%016llx\t%016llx\t%016llx\t%016llx\n",
            port, txPktCnt, txByteCnt, rxPktCnt, rxByteCnt, rxErrPktCnt);
    printf("\n");


    printf("-----------------H-Unit--------------------------------------\n");
    // H unit counter
    //tx counter
    ATTEMPT(mtdPktGeneratorGetCounter( pMtdDev, port, MTD_H_UNIT ,
                MTD_PKT_GET_TX,  &txPktCnt, &txByteCnt));
    //rx counter
    ATTEMPT(mtdPktGeneratorGetCounter( pMtdDev, port, MTD_H_UNIT,
                MTD_PKT_GET_RX,  &rxPktCnt, &rxByteCnt));
    //err counter
    ATTEMPT(mtdPktGeneratorGetCounter( pMtdDev, port, MTD_H_UNIT,
                MTD_PKT_GET_ERR,  &rxErrPktCnt, &rxErrByteCnt));
    printf("%d\t%016llx\t%016llx\t%016llx\t%016llx\t%016llx\n",
            port, txPktCnt, txByteCnt, rxPktCnt, rxByteCnt, rxErrPktCnt);
    printf("\n");


    return 0;
}

int phy_config_media_type_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    printf("Config media type to copper only\n");
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xf000, 0, 3, 0x0));

    //toggle software reset bit
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 31, 0xf001, 15, 1, 1));
    mtdWait(pMtdDev, 100);
    return 0;
}

int phy_side_band_signals_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 get_reg_val = 0;
    unsigned long cpld_reg = 0;

    printf("<To check CPLD SFP present bit, won't check other signals if SFP is not detected>\n");
    hr_cpld_reg_read_32(HR_CPLD_INT_STATUS, &cpld_reg);
    /* CPLD reg 0x44 bit 5 should be 1 */
    printf("CPLD reg 0x44 value is %lx\n", cpld_reg);
    if (cpld_reg & 0x20) {
        hr_cpld_reg_read_32(HR_CPLD_PHY_STATUS, &cpld_reg);
        printf("CPLD reg 0x50 value is %lx\n", cpld_reg);

        /* CPLD reg 0x50 bit 10 should be 1 */
        if (cpld_reg & 0x400) {
            printf("\n***SFP module is detected\n");
        } else {
            printf("\n***SFP module is not detected\n");
            return (0);
        }
    } else {
        printf("\n***SFP module is not detected\n");
        return (0);
    }

    /* 
     * Marvell 3310 dev 31 reg 0xf012 is GPIO data 
     * With SFP the default val is 0xa11 so no need to 
     * configure it
     */
    mtdHwGetPhyRegField(pMtdDev, port, 31, 0xf012, 0, 16, &get_reg_val);
    printf("\nGPIO Data : Dev 31 reg 0xf012 val = %hx\n", get_reg_val);

    /* 
     * Marvell 3310 dev 31 reg 0xf013 is GPIO Tristate Control i
     * With SFP the dafault val is 0xf04 so no need to 
     * configure it
     */
    mtdHwGetPhyRegField(pMtdDev, port, 31, 0xf013, 0, 16, &get_reg_val);
    printf("GPIO Tristate Control : Dev 31 reg 0xf013 val = %hx\n", get_reg_val);

    printf("\n<When Tx_dis value is 0x0>\n");
    printf("SFP EEPROM address 0xa2 reg 0x6e value should be 0x30\n");
    fflush(stdout);
    printf("***The read out value is - ");
    fflush(stdout);
    system("i2cget -y 1 0x51 0x6e");

    /* To set tx_dis with value 1 */
    printf("\n<When Tx_dis value is 0x1>\n");
    mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF012, 0, 16, 0xa15);
    mtdHwGetPhyRegField(pMtdDev, port, 31, 0xf012, 0, 16, &get_reg_val);
    printf("GPIO Data : Dev 31 reg 0xf012 val = %hx\n", get_reg_val);

    printf("SFP EEPROM address 0xa2 reg 0x6e value should be 0xb2\n");
    fflush(stdout);
    printf("***The read out value is - ");
    fflush(stdout);
    system("i2cget -y 1 0x51 0x6e");

    return (0);
}

int phy_config_advertisement_utils(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 tmpData;
    MTD_U16 speed = 0;
    MTD_U16 port = MTD_PORT_0;


    printf("Pls input speed for 1G /2.5G\n");
    speed = getdec_answer("1G (0) /2.5G (1)", speed, 0, 1);

    if (speed == 0) {
       //1G

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0010, 0, 16, &tmpData));
        tmpData &= ~(0x1e0); //clear bit 5,6,7,8
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x0010, 0, 16, tmpData));

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0020, 0, 16, &tmpData));
        tmpData &= ~(0x1180); //clear bit 7,8,12
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x0020, 0, 16, tmpData));

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x8000, 0, 16, &tmpData));
        tmpData &= ~(0x1 << 8); //clear bit 8
        tmpData |= (0x1 << 9); //set bit 9
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x8000, 0, 16, tmpData));

    } else {
        //2.5G

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0010, 0, 16, &tmpData));
        tmpData &= ~(0x1e0); //clear bit 5,6,7,8
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x0010, 0, 16, tmpData));

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x0020, 0, 16, &tmpData));
        tmpData &= ~(0x1180); //clear bit 8,12
        tmpData |= (0x1 << 7); //set bit7
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x0020, 0, 16, tmpData));

        ATTEMPT(mtdHwGetPhyRegField(pMtdDev, port, 7, 0x8000, 0, 16, &tmpData));
        tmpData &= ~(0x3 << 8); //clear bit 8, 9
        ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x8000, 0, 16, tmpData));
    }

    printf("toggle Auto-Negotiation enable bit 7.0.9\n");
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, 7, 0x0000, 9, 1, 1));
    mtdWait(pMtdDev, 100);


    return 0;
}

int phy_enable_10g_link_drop_counter(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;

    IN MTD_BOOL readToClear = MTD_FALSE;
    IN MTD_BOOL rollOver = MTD_FALSE;
    IN MTD_BOOL enable = MTD_TRUE;

    ATTEMPT(mtdTunitConfigure10GLinkDropCounter (pMtdDev,
                port, readToClear, rollOver, enable));
    return 0;
}


int phy_check_10g_link_drop_counter(void)
{
    int rc = 0;
    MTD_U16 linkDropCounter = 0;
    MTD_U16 fr_reg_val = 0;
    MTD_U16 lp_fr_count= 0;
    MTD_U16 ld_fr_count= 0;


    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;

    MTD_BOOL reset = 1;

    rc = mtdTunit10GGetLinkDropCounter( pMtdDev, port,
            reset, &linkDropCounter);
    rc += mtdHwGetPhyRegField(pMtdDev, port, 1, 0x0093, 0, 16, &fr_reg_val);

    mtdHwGetRegFieldFromWord(fr_reg_val,6,5,&ld_fr_count);
    mtdHwGetRegFieldFromWord(fr_reg_val,11,5,&lp_fr_count);

    if(MTD_OK == rc)
        printf("inst %02d port %d: link_drop: %04x ld_fr: %04x lp_fr: %04x\n",
                0, port, linkDropCounter, ld_fr_count, lp_fr_count);
    else
        printf("inst %02d port %d: get counter failed\n", 0, port);

    return 0;
}



static int _phy_reg_tst_read_fn(unsigned long addr, int size, unsigned long *buf, void *param)
{
    uint8_t  dev = (((unsigned long)param) & 0x00FF0000) >> 16UL;
    uint16_t reg = (((unsigned long)param) & 0x0000FFFF);
    if (MTD_OK != mtdHwXmdioRead(pMtd3310Dev, MTD_PORT_0, dev, reg, (MTD_U16 *)buf))
        return FAILED;
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("PHY RD 0x%02x %02u 0x%04x 0x%04x\n", MTD_PORT_0, dev, reg, (MTD_U16)*buf);
    }
    return PASSED;
}

static int _phy_reg_tst_write_fn(unsigned long addr, int size, unsigned long data, void *param)
{
    uint8_t  dev = (((unsigned long)param) & 0x00FF0000) >> 16UL;
    uint16_t reg = (((unsigned long)param) & 0x0000FFFF);
    if ((NVRAM)->diagflag & D_VERBOSE) {
        printf("PHY WR 0x%02x %02u 0x%04x 0x%04x\n", MTD_PORT_0, dev, reg, (MTD_U16)data);
    }
    if (MTD_OK != mtdHwXmdioWrite(pMtd3310Dev, MTD_PORT_0, dev, reg, (MTD_U16)data))
        return FAILED;
    return PASSED;
}

#define _PHY_TEST_REG(NAME,TYPE,DEV,OFF,MASK,DFLT) \
    static struct reg_info_t_ext_ _PHY_REG_EXT_##NAME = { \
        2, _phy_reg_tst_read_fn, _phy_reg_tst_write_fn, (void *)(((DEV)<<16UL)|((OFF)<<0UL)) \
    }; \
    static struct reg_info _PHY_REG_##NAME = { \
        #NAME, 0, TYPE, \
        {.ext = &_PHY_REG_EXT_##NAME}, \
        MASK, DFLT \
    }

//_PHY_TEST_REG(MTD_CUNIT_MODE_CONFIG, READ_WRITE | REG_ACCESS, MTD_C_UNIT_GENERAL, 0x0000F000, 0x00003FFF, 0x0000);
_PHY_TEST_REG(MTD_TUNIT_MDIO_TEST_1, READ_WRITE | REG_ACCESS, MTD_T_UNIT_PMA_PMD, 0x0000C003, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_TUNIT_MDIO_TEST_2, READ_WRITE | REG_ACCESS, MTD_T_UNIT_PMA_PMD, 0x0000C004, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_TUNIT_MDIO_TEST_3, READ_WRITE | REG_ACCESS, MTD_T_UNIT_PMA_PMD, 0x0000C005, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_LENGTH,     READ_WRITE|REG_ACCESS,  MTD_H_UNIT, 0x0000f016, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_BURSTSEQ,   READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000f017, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_IPG,        READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000f018, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXPKTCTR1,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F01B, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXPKTCTR2,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F01C, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXPKTCTR3,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F01D, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXBYTCTR1,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F01E, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXBYTCTR2,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F01F, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_TXBYTCTR3,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F020, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXPKTCTR1,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F021, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXPKTCTR2,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F022, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXPKTCTR3,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F023, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXBYTCTR1,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F024, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXBYTCTR2,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F025, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_RXBYTCTR3,  READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F026, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_ERRPKTCTR1, READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F027, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_ERRPKTCTR2, READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F028, 0x0000FFFF, 0x0000);
_PHY_TEST_REG(MTD_HUNIT_PKT_GEN_ERRPKTCTR3, READ_ONLY |REG_ACCESS,  MTD_H_UNIT, 0x0000F029, 0x0000FFFF, 0x0000);


#define MV88X3310_TEST_REGS                \
    _PHY_REG_MTD_TUNIT_MDIO_TEST_1,        \
    _PHY_REG_MTD_TUNIT_MDIO_TEST_2,        \
    _PHY_REG_MTD_TUNIT_MDIO_TEST_3,        \
    _PHY_REG_MTD_HUNIT_PKT_GEN_LENGTH,     \
    _PHY_REG_MTD_HUNIT_PKT_GEN_BURSTSEQ,   \
    _PHY_REG_MTD_HUNIT_PKT_GEN_IPG,        \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXPKTCTR1,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXPKTCTR2,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXPKTCTR3,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXBYTCTR1,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXBYTCTR2,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_TXBYTCTR3,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXPKTCTR1,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXPKTCTR2,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXPKTCTR3,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXBYTCTR1,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXBYTCTR2,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_RXBYTCTR3,  \
    _PHY_REG_MTD_HUNIT_PKT_GEN_ERRPKTCTR1, \
    _PHY_REG_MTD_HUNIT_PKT_GEN_ERRPKTCTR2, \
    _PHY_REG_MTD_HUNIT_PKT_GEN_ERRPKTCTR3, \
    {"END", 0x0 ,0 ,{0}, 0x0, 0x0},

/**********************************************************************
 * Function: _phy_reg_test
 *  This function perform the mv88x3310 register test.
 * Input : None
 * Output: PASSED/FAILED
 **********************************************************************
 */
static long _phy_reg_test(void)
{
    struct reg_info mv88x3310_test_regs[] = {
        MV88X3310_TEST_REGS
    };

    testname("Phy register test");

    if (register_tests(0, &mv88x3310_test_regs[0]) == FAILED) {
        cterr('f', 0, "[%s]%d: Marvel 88X3310 Register Test Failed\n",
                __FUNCTION__, __LINE__);
        return (FAILED);
    }
    printf("PHY Register Test Passed\n");
    return (PASSED);
}

/**********************************************************************
 * Function: _phy_reg_dump
 *  This function dump the mv88x3310 registers.
 * Input : None
 * Output: PASSED/FAILED
 **********************************************************************
 */
static long _phy_reg_dump(void)
{
    int idx = 0;
    MTD_STATUS ret_mtd = 0;
    MTD_U16 tail = 0;
    struct {
        const MTD_MDIO_ADDR_TYPE *arr;
        const MTD_U16 size;
    } to_dump[] = {
        {&cUnitRegData      [0], MTD_CUNIT_NUM_REGS         },
        {&f2rRegData        [0],  MTD_F2R_NUM_REGS          },
        {&hUnit10grRegData  [0], MTD_HUNIT_10GBASER_NUM_REGS},
        {&hUnit1000bxRegData[0], MTD_HUNIT_1000BX_NUM_REGS  },
        {&hUnitCommonRegData[0], MTD_HUNIT_COMMON_NUM_REGS  },
        {&hUnitRxauiRegData [0], MTD_HUNIT_RXAUI_NUM_REGS   },
        {&tUnitMmd1RegData  [0], MTD_TUNIT_MMD1_NUM_REGS    },
        {&tUnitMmd3RegData  [0], MTD_TUNIT_MMD3_NUM_REGS    },
        {&tUnitMmd3RegData2 [0], MTD_TUNIT_MMD3_2_NUM_REGS  },
        {&tUnitMmd3RegData3 [0], MTD_TUNIT_MMD3_3_NUM_REGS  },
        {&tUnitMmd7RegData  [0], MTD_TUNIT_MMD7_NUM_REGS    },
        {NULL, 0}
    };
    const MTD_U16 max = MTD_CUNIT_NUM_REGS;
    char out_buf[max * MTD_SIZEOF_OUTPUT];

    for(idx = 0; to_dump[idx].arr && to_dump[idx].size; idx++) {
        mtdCopyRegsToBuf(pMtd3310Dev, MTD_PORT_0, to_dump[idx].arr, to_dump[idx].size,
                         out_buf, max * MTD_SIZEOF_OUTPUT, &tail);

        if (ret_mtd == MTD_OK) {
            printf("%s", out_buf);
        }
        else {
            cterr('f', 0, "[%s]%d:Error when dump phy registers.\n",
                __FUNCTION__, __LINE__);
            return FAILED;
        }
    }
    return PASSED;
}

static long _phy_reg_rdwr(void)
{
    uint16_t opr = 0;
    uint16_t dev = 0;
    uint16_t reg = 0;
    uint16_t val = 0;

    printf("Read-0, Write-1\n");
    opr = getdec_answer("Read/Write", opr, 0, 1);

    printf("Device one is of [1, 3, 4, 7, 31]\n");
    dev = getdec_answer("Device(of port)", dev, 1, 31);

    reg = gethex_answer("Register", reg, 0, 0xffff);

    if (MTD_OK != mtdHwXmdioRead(pMtd3310Dev, MTD_PORT_0, dev, reg, (MTD_U16 *)&val)) {
        cterr('f', 0, "[%s]%d: Read dev-%u, reg-0x%04x failed\n",
            __func__, __LINE__, dev, reg);
        return (FAILED);
    }

    if (opr == 1) {
        val = gethex_answer("Value", val, 0, 0xffff);
        if (MTD_OK != mtdHwXmdioWrite(pMtd3310Dev, MTD_PORT_0, dev, reg, val)) {
            cterr('f', 0, "[%s]%d: Write dev-%u, reg-0x%04x failed\n",
                __func__, __LINE__, dev, reg);
            return (FAILED);
        }
    } else {
        printf("dev-%u, reg-0x%04x:0x%04x\n", dev, reg, val);
    }

    return PASSED;
}

static time_t phy_temp_en_time = 0;
int phy_enable_temperature(void)
{
    if (phy_temp_en_time == 0) {
        ERR_RET_COND(MTD_OK != mtdEnableTemperatureSensor(pMtd3310Dev, MTD_PORT_0), FAILED, "Enable phy temperature failed.\n");
        phy_temp_en_time = time(NULL);
    }
    return PASSED;
}

int phy_show_temperature(const char *tag)
{
    MTD_S16 tempr = 0;
    const char *flgf = "/tmp/phy_temp_read_flg";

    phy_enable_temperature();

    if (access(flgf, R_OK) != 0) {
        while(time(NULL) - phy_temp_en_time < 5) {
            usleep(100000);
            ERR_RET_COND(MTD_OK != mtdReadTemperature(pMtd3310Dev, MTD_PORT_0, &tempr), FAILED, "Read phy temperature failed.\n");
        }
        system("echo haha >> /tmp/phy_temp_read_flg");
    }
    ERR_RET_COND(MTD_OK != mtdReadTemperature(pMtd3310Dev, MTD_PORT_0, &tempr), FAILED, "Read phy temperature failed.\n");
    printf("%-16s: %-7d Celsius\n", tag, tempr);

    return PASSED;
}

static int highrise_phy_socket_test(void)
{
    struct highrise_eth_traf_tx_task_settings tx_settings;
    struct highrise_eth_traf_rx_task_settings rx_settings;

    tx_settings.mode = HIGHRISE_ETH_TRAF_TX_MODE_RADOM;
    tx_settings.check = HIGHRISE_ETH_TRAF_TX_CHECK_BIT_ADD_YES;
    tx_settings.len = 256;
    tx_settings.burst = 1;
    tx_settings.interval = 10000;
    rx_settings.chk_mode = HIGHRISE_ETH_TRAF_RX_MODE_CHECK_BIT;

    if (highrise_eth_traf_util_test("eth0", "eth0", &tx_settings, &rx_settings, 10)) {
        printf("eth traf failed on eth0");
        return -1;
    }
    return 0;
}

static long _phy_shallow_mac_lpbk_test(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 speed;
    MTD_U16 h_linkup;
    uint32_t mac_linkup;
    char tname[512];
    int rc = FAILED;

    ATTEMPT(phy_get_t_link_speed(pMtdDev, port, &speed));

    sprintf(tname, "Phy %s Shallow LPBK Test", mtd_speed_desc[speed]);
    testname("%s", tname);

    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_config_cpu_mac_port_10g(TRUE); 
    }

    /* 1. Set loopback */
    printf("1. Set loopback\n");
    ATTEMPT(marvell_mtd_enable_shallow_mac_loopback(pMtdDev, port, 1));
    sleep(5);

    /* 2. Check linkstatus */
    printf("2. Check link status\n");
    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_check_cpu_mac_port_status(&mac_linkup, SPEED_10000);
        phy_check_10g_link_status(pMtdDev, port, MTD_H_UNIT, &h_linkup);
    } else {
        phy_check_cpu_mac_port_status(&mac_linkup, SPEED_1000);
        phy_check_h_link_status(pMtdDev, port, &h_linkup);
    } 

    if (mac_linkup && h_linkup) {
        printf("Interfaces MAC and H-unit link up\n");
    } else {
        cterr('f', 0,
                "[%s]%d: Error! link status down!\n",
                __FUNCTION__, __LINE__);
        ATTEMPT(marvell_dump_link_status(pMtdDev, port));
        prcomplete(testpass, errcount, (char *)0);
        goto exit;
    }

    /* 3. Send packet*/
    printf("3. Send packets\n");
    if (highrise_phy_socket_test()) {
        cterr('f', 0,
                "***[%s]%d: Test Failed, pls clear lpbk manually\n",
                __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        goto exit;
    } else {
        rc = PASSED;
        printf("\n%s PASSED\n", tname);
    }

exit:
    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_config_cpu_mac_port_10g(FALSE); 
    }

    /* 4. Clear loopback*/
    printf("4. Clear loopback\n");
    ATTEMPT(marvell_mtd_enable_shallow_mac_loopback(pMtdDev, port, 0));
    sleep(2);

    return (rc);
}

static long _phy_deep_mac_lpbk_test(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 speed;
    MTD_U16 h_linkup;
    uint32_t mac_linkup;
    char tname[512];
    int rc = FAILED;

    ATTEMPT(phy_get_t_link_speed(pMtdDev, port, &speed));

    sprintf(tname, "Phy %s Deep Mac LPBK Test", mtd_speed_desc[speed]);
    testname(tname);

    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_config_cpu_mac_port_10g(TRUE); 
    }

    /* 1. Set loopback */
    printf("1. Enable deep mac loopback\n");
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, MTD_TUNIT_IEEE_PCS_CTRL1, 14, 1, 1));
    sleep(5);

    printf("2. Toggle ETH0 AN\n");
    system("ethtool -r eth0\n");
    /*
     *  As highrise, it looks like that Chrysler P1B also needs 
     *  30 secs before checking link.
     */
    sleep(30);

    /* 2. Check linkstatus */
    printf("3. Check link status\n");
    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_check_cpu_mac_port_status(&mac_linkup, SPEED_10000);
        phy_check_10g_link_status(pMtdDev, port, MTD_H_UNIT, &h_linkup);
    } else {
        phy_check_cpu_mac_port_status(&mac_linkup, SPEED_1000);
        phy_check_h_link_status(pMtdDev, port, &h_linkup);
    }

    if (mac_linkup && h_linkup) {
        printf("Interfaces MAC and H-unit link up\n");
    } else {
        cterr('f', 0,
                "[%s]%d: Error! link status down!\n",
                __FUNCTION__, __LINE__);
        ATTEMPT(marvell_dump_link_status(pMtdDev, port));
        prcomplete(testpass, errcount, (char *)0);
        goto exit;
    }

    /* 3. Send packet*/
    printf("4. Send packets\n");
    if (highrise_phy_socket_test()) {
        cterr('f', 0, 
                "***[%s]%d: Test Failed, pls clear lpbk manually\n",
                __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        goto exit;
    } else {
        printf("\n%s PASSED\n", tname);
        rc = PASSED;
    }

exit:
    if (speed == MTD_SPEED_10GIG_FD) { 
        phy_config_cpu_mac_port_10g(FALSE); 
    }

    /* 4. Clear loopback */
    printf("5. Clear loopback\n");
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev, port, MTD_TUNIT_IEEE_PCS_CTRL1, 14, 1, 0));
    sleep(2);

    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF000, 0, 16, 0x20cf));
    sleep(1);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x800c));
    sleep(2);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x800c));
    sleep(2);

    printf("6. Toggle ETH0 AN\n");
    system("ethtool -r eth0\n");
    /*
     *  As highrise, it looks like that Chrysler P1B also needs 
     *  30 secs delay time.
     *  Otherwise 10G SFP loopback MAC link doesn't have enough
     *  time to comeback.
     */
    sleep(30);

    return (rc);
}

static long _phy_send_pkt_test(void)
{
    char *tname = "Send and Check Packet Test";
    testname("%s", tname);

    // Send packets
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the external loopback test\n");
        return PASSED;
    }
    if (highrise_phy_socket_test()) {
        cterr('f', 0, "[%s]%d: phy socket test failed Failed\n", __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    printf("\n%s PASSED\n", tname);
    return (PASSED);
}

static long _phy_100m_ext_lpbk_test(void)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;
    MTD_U16 t_linkup;
    MTD_U16 h_linkup;
    uint32_t mac_linkup;
    
    char *tname = "Phy 100M external loopback test";
    testname(tname);

    /* 0. Config Mac & PHY Mode to 100M */
    printf("0. Config mode to 100M via ethtool\n");
    system("ethtool -s eth0 autoneg off speed 100 duplex full\n");
    sleep(30);
    
    /* 1. Check linkstatus*/
    printf("1. Check Link status\n");
    phy_check_cpu_mac_port_status(&mac_linkup, SPEED_100);
    phy_check_h_link_status(pMtdDev, port, &h_linkup);
    phy_check_t_link_status(pMtdDev, port, &t_linkup);

    if (mac_linkup && h_linkup && t_linkup) {
        printf("Interfaces all link up\n");
    } else {
        cterr('f', 0,
                "[%s]%d: Error! link status down!\n",
                __FUNCTION__, __LINE__);
        ATTEMPT(marvell_dump_link_status(pMtdDev, port));
        prcomplete(testpass, errcount, (char *)0);
        return(FAILED);
    }

    // Send packets
    printf("2. Send packets\n");
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip the external loopback test\n");
        return PASSED;
    }
    if (highrise_phy_socket_test()) {
        cterr('f', 0,
                "[%s]%d: phy socket test failed, PHY is in 100M mode!\n",
                __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
        return (FAILED);
    }

    // Restore back to SGMII 
    printf("3. Restore Mac & PHY mode via ethtool\n");
    system("ethtool -s eth0 autoneg on; ethtool -r eth0\n");
    sleep(20);

    printf("\n%s PASSED\n", tname);
    return (PASSED);
}

int phy_check_10g_link_status(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
	MTD_U16 HorXunit,
        MTD_U16 *linkup_p)
{
    MTD_BOOL tx_lpi_latch; 
    MTD_BOOL rx_lpi_latch; 
    MTD_BOOL tx_lpi_current; 
    MTD_BOOL rx_lpi_current; 
    MTD_BOOL fault;
    MTD_BOOL link_status_latch;
    MTD_BOOL link_status_current;
    int ix = 0;
    *linkup_p = 0;

    for (ix = 0; ix < LINK_UP_MAX_POLL; ix++) {
        ATTEMPT(mtdGet10GBRStatus1(pMtdDev, port, HorXunit,
                    &tx_lpi_latch, &rx_lpi_latch, 
                    &tx_lpi_current, &rx_lpi_current, &fault,
                    &link_status_latch, &link_status_current));
        if (link_status_current) {
            *linkup_p = TRUE;
            break; 
        } else {
            sleep(1);
        }
    }

    if ((ix == LINK_UP_MAX_POLL) && !(*linkup_p)) {
        printf("Error: X-unit link error for 10G after polling 10s\n");
        printf("Error: H-unit link error for 10G after polling 10s\n");
        return (FAILED);
    }

    return(PASSED);
}

int phy_check_h_link_status(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 *linkup_p)
{
    /* Highrise only support speed < 2.5G */
    /* H unit SGMI status */
    MTD_BOOL an_complete;
    MTD_BOOL remote_fault;
    MTD_BOOL link_status_latched;
    MTD_BOOL link_status_current;
    int i = 0;
    *linkup_p = 0;

    for (i = 0; i < LINK_UP_MAX_POLL; i++) {
        ATTEMPT(mtdGet1000BXSGMIIStatus(pMtdDev, port, MTD_H_UNIT,
                    &an_complete, &remote_fault, &link_status_latched,
                    &link_status_current));
        if (link_status_current) {
            *linkup_p = 1;
            return(PASSED);
        } else {
            sleep(1);
        }
    }
    if ((i == LINK_UP_MAX_POLL) && !(*linkup_p)) {
        printf("Error: H-unit link error for 10M/100M/1G after polling 12s\n");
        return (FAILED);
    }
    return(PASSED);
}

int phy_check_t_link_status(
        MTD_DEV *pMtdDev,
        MTD_U16 port, 
        MTD_U16 *linkup_p)
{
    int i = 0;
    MTD_U16 t_speed;
    MTD_BOOL t_linkup;
    *linkup_p = 0;

    for (i = 0; i < LINK_UP_MAX_POLL; i++) {
        ATTEMPT(mtdIsBaseTUp(pMtdDev, port, &t_speed, &t_linkup));
        if (t_linkup) {
            *linkup_p = 1;
            return(PASSED);
        } else {
            sleep(1);
        }
    }
    if ((i == LINK_UP_MAX_POLL) && !(*linkup_p)) {
            printf("Error: T-unit link error after polling 10s\n");
            return (FAILED);
        }
    return(PASSED);
}

int phy_get_t_link_speed(
        MTD_DEV *pMtdDev,
        MTD_U16 port,
        MTD_U16 *t_speed)
{
    MTD_BOOL t_linkup;
    ATTEMPT(mtdIsBaseTUp(pMtdDev, port, t_speed, &t_linkup));
    return(PASSED);
}

int phy_check_cpu_mac_port_status(uint32_t *linkup_p, uint speed)
{
    int ix = 0;
    uint32_t linkup_msk = 0x1;
    uint32_t mac_port_status_reg = 0; 
    uint32_t mac_port_status = 0;
    *linkup_p = FALSE; 

    if (speed == SPEED_10000) { 
        mac_port_status_reg = 0xF2130F0C; 
    } else { 
        mac_port_status_reg = 0xF2130E10;
    } 

    for (ix = 0; ix < LINK_UP_MAX_POLL; ix++) {
        ATTEMPT(highrise_mem_read32(mac_port_status_reg, &mac_port_status));
        if (mac_port_status & linkup_msk) {
            *linkup_p = TRUE;
            break;
        } else {
            sleep(1);
        }
    }
    if ((LINK_UP_MAX_POLL == ix) && !(*linkup_p)) {
        printf("Error: MAC link failed after polling 10s, [0x%x]:0x%hx\n",
                mac_port_status_reg, mac_port_status);
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
} phy_intr_mpp[] = {
    /*0 */{"function        ", 0xf2440000, 0,  4},
    /*1 */{"data-out        ", 0xf2440100, 0,  1},
    /*2 */{"data-out-en     ", 0xf2440104, 0,  1},
    /*3 */{"blink-en        ", 0xf2440108, 0,  1},
    /*4 */{"data-in-polarity", 0xf244010c, 0,  1},
    /*5 */{"data-in         ", 0xf2440110, 0,  1},
    /*6 */{"intr-cause      ", 0xf2440114, 0,  1},
    /*7 */{"intr-mask       ", 0xf2440118, 0,  1},
    /*8 */{"intr-level-mask ", 0xf244011c, 0,  1},
    /*9 */{"blink-cntr      ", 0xf2440120, 0,  1},
    /*10*/{NULL              , ~0        ,~0, ~0},
};

static long _phy_intr_trig (int enable)
{
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 port = MTD_PORT_0;

    if (enable) { 
        mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF041, 0,  1, 0x1);
    } else {
        mtdHwSetPhyRegField(pMtdDev, port, 31, 0xF041, 0,  1, 0x0);
    }

    return 0;
}

static long _phy_intr_test(void)
{
    char *tname = "Phy Intr test"; 
    unsigned long val = 0, rc = FAILED;

    testname(tname);

    prpass(testpass, "Force PHY intr "); 
    _phy_intr_trig(TRUE); 
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PHY_STATUS, &val), FAILED, "Read cpld failed.\n");

    if (val & HR_CPLD_PHY_STA_INT) { 
        prpass(testpass, "CPLD receive intr"); 
        printf("pass\n");
        rc = PASSED; 
    } else {
        cterr('f', 0, "[%s]%d: phy intr test failed, CPLD PHY status = 0x%x\n",
                __FUNCTION__, __LINE__, val);
    }
    /* disable PHY intr and clear CPLD PHY intr */
    _phy_intr_trig(FALSE);

    val &= ~HR_CPLD_PHY_STA_INT; 
    ERR_RET_COND(PASSED != hr_cpld_reg_write_32(HR_CPLD_PHY_STATUS, val), FAILED, "Read cpld failed.\n");

    prcomplete(testpass, errcount, (char *)0);
    return rc; 
}


static long _phy_sfp_lpbk_test (void) 
{
    char *tname = "Phy SFP external loopback test";
    MTD_U16 port = MTD_PORT_0;
    MTD_DEV *pMtdDev = pMtd3310Dev;
    MTD_U16 x_linkup;
    uint32_t mac_linkup;
    int rc = FAILED;
    unsigned long val; 

    testname(tname);

    /* check external loopback flag on */
    if ((NVRAM)->diagflag & D_EXT_LOOPBACK) {
        printf("External loopback flag is off, skip SFP external loopback test\n");
        return (PASSED);
    }

    prpass(testpass, "start test "); 

    /* check SFP present */ 
    ERR_RET_COND(PASSED != hr_cpld_reg_read_32(HR_CPLD_PHY_STATUS, &val), FAILED, "Read cpld failed.\n");
    if (val & HR_CPLD_PHY_STA_SFP_PRST) { 
        prpass(testpass, "SFP present"); 
    } else {
        cterr('f', 0, "[%s]%d: SFP is not present, CPLD PHY status register = 0x%x (bit10 = 0)\n",
                __FUNCTION__, __LINE__, val);
        return (FAILED); 
    }

    /* setup mac to 10g */
    prpass(testpass, "config cpu mac to 10G"); 
    phy_config_cpu_mac_port_10g(TRUE);  

    /* config to SFP mode, 31.F000.2:0 = 001 */ 
    prpass(testpass, "config sfp mode"); 
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF000, 0, 3, 1));
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 3, 2, 3));

    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 15, 1, 1));
    /* 
     * For P1B, write reset bit twice to make it take effect.
     * Maybe this is not really needed.
     */
    sleep(2);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 15, 1, 1));
    sleep(30);  /* Per Vendor suggestion, reset following 3 sec delay */

    /* check x-unit link up */
    prpass(testpass, "check link up"); 
    phy_check_cpu_mac_port_status(&mac_linkup, SPEED_10000);
    phy_check_10g_link_status(pMtdDev, port, MTD_X_UNIT, &x_linkup);
    if ((x_linkup) & (mac_linkup)) { 
        prpass(testpass, "x-unit link up"); 
    } else {
        /* send packet anyway.. */
    }

    /* send packet */
    prpass(testpass, "send packet "); 
    if (highrise_phy_socket_test()) {
        cterr('f', 0,
                "[%s]%d: phy socket test failed, PHY is in SFP mode!\n",
                __FUNCTION__, __LINE__);
        prcomplete(testpass, errcount, (char *)0);
    } else {
        rc = PASSED;
        prpass(testpass, "test passed."); 
    }

    prpass(testpass, "revert data "); 
    /* revert to SGMII mode for cpu mac and phy 31.F000.2:0 = 0 */
    phy_config_cpu_mac_port_10g(FALSE); 

    sleep(1);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF000, 0, 16, 0x20cf));
    sleep(1);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x800c));
    sleep(2);
    ATTEMPT(mtdHwSetPhyRegField(pMtdDev,port,31,0xF001, 0, 16, 0x800c));

    /* check T-unit link up? or delay? */
    /* How to know link media? */
 //   mtdWait(pMtdDev, 10000);
    sleep(30); 

    return (rc);
}

void phy_config_cpu_mac_port_10g (uint32_t enable)
{
    uint32_t mac_port_ctrl_reg3 = 0xF2130F1C; /* port0 */
    uint32_t mac_port_ctrl_reg4 = 0xF2130F84;
    uint32_t mac_mode_select = 0;
    uint32_t mac_mode_dma = 0;

    highrise_mem_read32(mac_port_ctrl_reg4, &mac_mode_dma);
    highrise_mem_read32(mac_port_ctrl_reg3, &mac_mode_select); 

    if (enable == TRUE) { 
        mac_mode_dma &= ~0x1000;  /* 10G bit12=0 */
        mac_mode_select |= 0x2000;  /* 10G bit13 = 1*/
    } else { 
        mac_mode_dma |= 0x1000;  /* 1G bit12=1 */
        mac_mode_select &= ~0x2000;  /* 1G bit13 = 0*/
    }

    highrise_mem_write32(mac_port_ctrl_reg4, mac_mode_dma);
    highrise_mem_write32(mac_port_ctrl_reg3, mac_mode_select); 

    if (((NVRAM)->diagflag & D_VERBOSE)) { 
        highrise_mem_read32(mac_port_ctrl_reg4, &mac_mode_dma);
        highrise_mem_read32(mac_port_ctrl_reg3, &mac_mode_select); 
        printf("@0x%x - 0x%x \n", mac_port_ctrl_reg4, mac_mode_dma); 
        printf("@0x%x - 0x%x \n", mac_port_ctrl_reg3, mac_mode_select); 
    }
    return;
}


/*********************************************************************
 * $Log: platform_phy.c,v $
 * Revision 1.2  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.5  2021/01/29 05:55:45  leschen
 * To fix Chrysler P1B MAC link down issue.
 *
 * Revision 1.1.4.4  2020/12/28 09:23:17  leschen
 * Suport MV3310 side band signals utility.
 *
 * Revision 1.1.4.3  2020/12/23 07:58:27  alpeng
 * support phy intr test and check sfp present before lpbk testing
 *
 * Revision 1.1.4.2  2020/09/26 06:21:41  leschen
 * Fix 3310 lpbk issue. Issue could be reproduced if executed 1G and 10G lpbk twice without power cycle.
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

