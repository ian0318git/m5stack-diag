/* $Id: diag_ext.c,v 1.2 2017/08/02 14:21:44 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/diag_ext.c,v $
 *------------------------------------------------------------------
 *
 * diag_ext.c - diag extended feature functions. 
 *
 * May 2016, Sofian teja.
 *
 * Copyright (c) 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "error.h"
#include "nvsysvars.h"
#include "menu.h"
#include "linux_api.h"
#include "proto.h"
#include "platform_esw.h"
#include "platform_fpga.h"
#include "plat_defs.h"

#define TSN_ESW_RETRY_MAX   100

#define PROFILE_1   1
#define PROFILE_2   2
#define PROFILE_3   3

/*
 * Declare local function
 */

int device_out_of_reset(void);
int show_interface(void);
int bridge_config(boolean);
int iptable_config(boolean);
static int tsn_esw_8port_vlan_config(int);
static int tsn_esw_4port_vlan_config(int);
static int tsn_esw_std_pair_vlan(int);
static int tsn_traffic_test_esw_config(int);
static int tsn_esw_vlan_profile_config(int);
static int lte_traffic_test(int);
static int tsn_m_esw_vlan_config(int);

/*
 * Declare external function
 */
extern int do_all_menu_items(struct menuinfo *);
extern int getdec_answer(char *, uint, uint, uint);
extern int fpga_reset_32_api(uint, uint, uint, uint);
extern void insert_traffic_module (boolean);
extern int tsn_set_esw_port_cap_util(int);

/*
 * Debug Board Test Menu
 */

static submenu_xtable_t diag_extend_feature_table[] = {
    {"Configure IPTable", (type_t(*)())iptable_config, TRUE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())0, FALSE},
    {"Bridge config interface", (type_t(*)())bridge_config, TRUE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())0, FALSE},
    {"Bridge remove config interface", (type_t(*)())bridge_config, FALSE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())0, FALSE},
    {"Configure Switch VLANs",  (type_t(*)())tsn_esw_vlan_profile_config, FALSE,
     MF_CONTINUOUS,             (type_t(*)())0,                           0,
     (type_t(*)())0,            FALSE},
    {"Show Interface", (type_t(*)())show_interface, TRUE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())show_interface, FALSE},
    {"OTA Device out of reset", (type_t(*)())device_out_of_reset, TRUE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())device_out_of_reset, FALSE},
    {"Turn on/off LTE traffic mode", (type_t(*)())lte_traffic_test, TRUE, MM_1,
                             (type_t(*)())0, 0, (type_t(*)())lte_traffic_test, FALSE},
    {"Config LAN port capability",    (type_t(*)())tsn_set_esw_port_cap_util, 0,
     0,                               (type_t(*)())0,                         0,
     (type_t(*)())0,                  0},
};

#define DIAG_EXT_TABLE_SZ \
        (sizeof(diag_extend_feature_table) / sizeof(submenu_xtable_t))

/*
 * primary & secondary submenu items (filled in from xtable)
 */
static mitem_t diag_ext_pri_test_items[DIAG_EXT_TABLE_SZ + MAX_BASE_ITEMS];
static mitem_t diag_ext_sec_test_items[DIAG_EXT_TABLE_SZ + MAX_BASE_ITEMS];

static menuinfo_t diag_ext_test_menu = {
    "Diag Extended Feature Menu",
    0,                            /* mtparam added by init_empty_menu */
    (PFT)menu_show_dflags,        /* shows major flags */
    0,                            /* use generic prompt */
    0,                            /* size (bumped by add_menu_item() */
    diag_ext_pri_test_items,
};
static menuinfo_t *diag_ext_menup = &diag_ext_test_menu;

/*-------------------------------------------------------------------
 *
 * Function: diag_extend_feature()
 * 
 * To support functional team to run QVT testing
 *
 * input : N/A
 *
 * output : PASSED / FAILED 
 *
 *-------------------------------------------------------------------
 */
int diag_extend_feature(boolean db_test_items_executed)
{
    int rc = FAILED;
    char *tname = "Diag Extended Feature";
    testname(tname);

    build_primary_submenu(diag_extend_feature_table,
                          DIAG_EXT_TABLE_SZ, "Diag Extended",
                          &diag_ext_menup);

    build_secondary_submenu(diag_extend_feature_table,
                            DIAG_EXT_TABLE_SZ,
                            diag_ext_sec_test_items);

    if (db_test_items_executed) {
        do_all_menu_items(&diag_ext_test_menu);
    } else {
        menu(&diag_ext_test_menu, diag_ext_sec_test_items, '\0');
    }

    return (rc);
}

/*-------------------------------------------------------------------
 *
 * Function: iptable_config()
 *  
 * Select iptable profile test 
 * 
 * input: none
 *
 * output: PASSED/FAILED
 *
 *--------------------------------------------------------------------*/
int iptable_config (boolean dummy)
{
    int status = 0;
    char tname[LENGTH100];
    int first_inf = 0;
    int second_inf = 0;
    int profile = 0;
    
    printf("IPtable profile:\n");
    printf("1. Route GE WAN-0 (eth0) and GE Switch / Wifi (eth1)\n");
    printf("2. Route GE WAN-0 (eth0) and GE WAN-1 (eth2)\n");
    printf("3. Route GE Switch / Wifi (eth1) and GE WAN-1 (eth2)\n");
    printf("4. Quit\n");
    profile = getdec_answer("Select: ", 4, 1, 4);

    switch (profile) {
    case 1: /* profile 1*/  
        first_inf = 0;
        second_inf = 1;
        break;
    case 2: /* profile 2*/  
        first_inf = 0;
        second_inf = 2;
        break;
    case 3: /* profile 3*/
        first_inf = 1;
        second_inf = 2;
        break;
    default:
        printf("Not correct profiles\n");
        return(FAILED);  
    }
    printf("iptables Rule setting\n");
    printf("------------------------------------------------------\n");
    printf("iptables -t nat -A POSTROUTING -o eth%d -j MASQUERADE\n", first_inf);
    printf("iptables -A FORWARD -i eth%d -j ACCEPT\n", second_inf);
    printf("------------------------------------------------------\n");
    sprintf (tname, "iptables -t nat -A POSTROUTING -o eth%d -j MASQUERADE", first_inf);
    status = system(tname); 
    sprintf (tname, "iptables -A FORWARD -i eth%d -j ACCEPT", second_inf);
    status = system(tname); 
    printf("------------------------------------------------------\n");
    printf("iptables -t nat -A POSTROUTING -o eth%d -j MASQUERADE\n", second_inf);
    printf("iptables -A FORWARD -i eth%d -j ACCEPT\n", first_inf);
    printf("------------------------------------------------------\n");
    sprintf (tname, "iptables -t nat -A POSTROUTING -o eth%d -j MASQUERADE", second_inf);
    status = system(tname); 
    sprintf (tname, "iptables -A FORWARD -i eth%d -j ACCEPT", first_inf);
    status = system(tname);
    printf("------------------------------------------------------\n");
    printf("iptables Rule setting done.\n");
    return(PASSED);

}

/*-------------------------------------------------------------------
 * Function: Bridge config()
 * Select bridge profile test 
 * input: mode
 * output: PASSED/FAILED
 *--------------------------------------------------------------------*/
int bridge_config (boolean mode)
{
    int rc = FAILED, status = 0;
    char tname[LENGTH100];
    int first_inf = 0;
    int second_inf = 0;
    int profile = 0;
    
    if (mode == TRUE) {
        printf("Bridge profile:\n");
        printf("1. Bridge GE WAN-0 (eth0) and GE Switch / Wifi (eth1)\n");
        printf("2. Bridge GE WAN-0 (eth0) and GE WAN-1 (eth2)\n");
        printf("3. Bridge GE Switch / Wifi (eth1) and GE WAN-1 (eth2)\n");
        printf("4. Quit\n");
        profile = getdec_answer("Select: ", 4, 1, 4);

        switch (profile) {
        case 1: /* profile 1*/  
            first_inf = 0;
            second_inf = 1;
            break;
        case 2: /* profile 2*/  
            first_inf = 0;
            second_inf = 2;
            break;
        case 3: /* profile 3*/
            first_inf = 1;
            second_inf = 2;
            break;
        default:
           printf("Not correct profiles\n");
           return(FAILED);  
        }

        if (first_inf == second_inf) {
            printf("Is Same Interface!! Can't Bridge, Please try again!\n");
            return (FAILED);
        }
        /* ifconfig eth-N up */
        /*brctl addbr br0 */
            sprintf (tname, "brctl addbr br0");
        status = system(tname);    
        /*brctl addif br0 eth-first */
        sprintf (tname, "brctl addif br0 eth%d", first_inf);
        status = system(tname);    
        /*brctl addif br0 eth-second */
        sprintf (tname, "brctl addif br0 eth%d", second_inf);
        status = system(tname);    
        /* ifconfig br0 up */ 
        sprintf (tname, "ifconfig br0 up");
        status = system(tname);    
        /* brctl show */
        sprintf (tname, "brctl show");
            status = system(tname);
        rc = PASSED;    
    } else {
        /* brctl show */
        sprintf (tname, "brctl show");
       	status = system(tname);
        /* ifconfig br0 down */
        first_inf = getdec_answer("Select First Ethernet Interface to remove bridge : eth?", 0, 0, 3);
        second_inf = getdec_answer("Select Second Ethernet Interface to remove bridge : eth?", 0, 0, 3);

        if (first_inf == second_inf) {
            printf("Is Same Interface!! Can't Remove Bridge, Please try again!\n");
            return (FAILED);
        }
        sprintf (tname, "ifconfig br0 down");
        status = system(tname);    
        /*brctl delif br0 eth0 */
        sprintf (tname, "brctl delif br0 eth%d", first_inf);
        status = system(tname);    
        /*brctl delif br0 eth2 */
        sprintf (tname, "brctl delif br0 eth%d", second_inf);
        status = system(tname);    
        /*brctl delbr br0  */
        sprintf (tname, "brctl delbr br0");
        status = system(tname);    
        /* brctl show */
        sprintf (tname, "brctl show");
        status = system(tname);    
        rc = PASSED;    
    }    
    return (rc);
}

/*-------------------------------------------------------------------
 * Function: show_interface()
 * Show eth interface
 * input: none 
 * output: PASSED
 *--------------------------------------------------------------------*/
int show_interface (void)
{
    printf("Show Interface :\n");
    printf("eth0 = GE WAN-0\n");
    printf("eth1 = GE SWITCH or Wifi\n");
    printf("eth2 = GE WAN-1 or xDSL\n");
    system("ifconfig -a"); 
    return (PASSED);
}


/*-------------------------------------------------------------------
 * Function: device_out_of_reset()
 * Show eth interface
 * input: none 
 * output: PASSED
 *--------------------------------------------------------------------*/
int device_out_of_reset(void)
{
    printf("OTA all device out of reset \n");
   
    if (fpga_reset_32_api(FPGA_EXTER_DEV_RST_REG, FPGA_EMMC_RESET | 
         FPGA_GEWAN1_RESET | FPGA_GEWAN0_RESET | EXT_DSL_CHIP_RESET| 
         EXT_ROMMON_FLASH_RESET | EXT_PRI_LTE_WDIS_1_RESET | 
         EXT_PRI_LTE_WDIS_2_RESET |EXT_PRI_POE_DC_RESET | 
         EXT_PRI_LTE_RESET |EXT_WLAN_RESET |EXT_CPU_SYS_RESET| 
         EXT_ESW_RESET, FALSE, WAITTIME_150_MS ) != PASSED) {
         printf("Failed to Out of reset all devices.");
         return (FAILED);
    } 
    /* use traffic lte driver module */
    insert_traffic_module(TRUE); 
    return (PASSED);
}

/*-------------------------------------------------------------------
 * Function: lte_traffic_test()
 * Show eth interface
 * input: mode on/off 
 * output: PASSED
 *--------------------------------------------------------------------*/
int lte_traffic_test(int mode)
{
    /* use traffic lte driver module */
    insert_traffic_module(mode); 
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_std_pair_vlan
 * Description: Wrapped function to set TSN ethernet switch(Marvell 88E6390)
 *              standard pair VLANs(VLAN1: port1 & 2 , VLAN2: port3 & 4, ...)
 *              and enable forwarding feature of configured ports.
 *              Note: all setups in this function are sequentially and
 *                    starts from ESW port1.
 * Inputs     : pair_num - number of pairs that wants to set
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_esw_std_pair_vlan (int pair_num)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0, vlan_id = 0, wr_data = 0;
    int    ctr = 0, retry_ctr = 0, total_ports = 0;

    if (pair_num > (TSN_ESW_PORTS / 2)) {
        printf("Wanted pairs number %d is out of range.\n", pair_num);
        printf("There's only %d eth ports of TSN Switch.\n", TSN_ESW_PORTS);
        return (FAILED);
    }
    total_ports = (pair_num * 2);

    /* Confirm Switch forwarding feature are enabled. */
    reg_addr = (int)ESW_PORTCTR_REG;
    for (ctr = ESW_PORT1; ctr <= total_ports; ctr++) {
        prpass(testpass, "Check ESW port%d forwarding feature.", ctr);

        reg_val = 0;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW port%d port control Reg.(0x%02X)\n",
                   __FUNCTION__, ctr, reg_addr);
            return (FAILED);
        }

        if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
            reg_val |= (ushort)(ESW_PCR_FORWARD);

            if (tsn_esw_reg_wr(ctr, reg_addr, reg_val) != PASSED) {
                printf("%s: Failed to write ESW port%d port control Reg."
                       "(0x%02X)\n",
                       __FUNCTION__, ctr, reg_addr);
                return (FAILED);
            }

            reg_val = 0;
            if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
                printf("%s: Failed to read ESW port%d port control Reg."
                       "(0x%02X)\n",
                       __FUNCTION__, ctr, reg_addr);
                return (FAILED);
            }

            if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
                printf("%s: Failed to enable ESW port%d forwarding feature.",
                       __FUNCTION__, ctr);
                return (FAILED);
            }
        }
    }

    /* Config. 802.1Q mode and VLAN ID */
    for (ctr = ESW_PORT1; ctr <= total_ports; ctr++) {
        prpass(testpass, "Config. ESW port%d 802.1Q mode and VLAN ID", ctr);

        /* Set 802.1Q mode */
        reg_val = 0;
        reg_addr = (int)ESW_PORTCTR2_REG;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW port%d port control2 Reg.(0x%02X)\n",
                   __FUNCTION__, ctr, reg_addr);
            return (FAILED);
        }

        if ((reg_val & ESW_PCR2_8021Q_MODE_MSK) != ESW_PCR2_8021Q_SECURE) {
            reg_val = ((reg_val & (ushort)ESW_PCR2_8021Q_MODE_MSK) |
                       (ushort)ESW_PCR2_8021Q_SECURE);

            if (tsn_esw_reg_wr(ctr, reg_addr, reg_val) != PASSED) {
                printf("%s: Failed to write ESW port%d port control2 Reg."
                       "(0x%02X)\n",
                       __FUNCTION__, ctr, reg_addr);
                return (FAILED);
            }

            reg_val = 0;
            if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
                printf("%s: Failed to read ESW port%d port control2 Reg."
                       "(0x%02X)\n",
                       __FUNCTION__, ctr, reg_addr);
                return (FAILED);
            }

            if ((reg_val & ESW_PCR2_8021Q_MODE_MSK) != ESW_PCR2_8021Q_SECURE) {
                printf("%s: Failed to set ESW port%d to 802.1Q secure mode.",
                       __FUNCTION__, ctr);
                return (FAILED);
            }
        }

        /* Set VLAN ID */
        reg_val = 0;
        reg_addr = (int)ESW_PORTVLAN_ID_REG;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, __LINE__, ctr, reg_addr);
            return (FAILED);
        }

        if (ctr % 2 != 0) {
            vlan_id++;
        }

        wr_data = (reg_val &
                   (ushort)(~(ESW_PVID_FORCE_DVID | ESW_PVID_DVID_MSK)));
        wr_data |= (ushort)vlan_id;

        if (tsn_esw_reg_wr(ctr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW port%d port default VLAN ID Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, ctr, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(ctr, reg_addr, &reg_val) != PASSED) {
            printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, __LINE__, ctr, reg_addr);
            return (FAILED);
        }

        if ((reg_val & ESW_PVID_FORCE_DVID) == ESW_PVID_FORCE_DVID) {
            printf("%s: Failed to clear ESW port%d Force VLAN ID.",
                   __FUNCTION__, ctr);
            return (FAILED);
        }

        if ((reg_val & ESW_PVID_DVID_MSK) != vlan_id) {
            printf("%s: Failed to set ESW port%d VLAN ID to %d.",
                   __FUNCTION__, ctr, vlan_id);
            return (FAILED);
        }
    }

    /* Config. VLANs */
    dev_addr = (int)ESW_SMIDEV_GLOB1;

    for (ctr = ESW_VLAN1; ctr <= pair_num; ctr++) {
        /* Set VLAN ID */
        prpass(testpass, "Config. VLAN%d VID", ctr);

        wr_data = (ushort)(ESW_G1_VID_ENTRY_VALID | ctr);
        reg_addr = (int)ESW_G1_VTUVID_REG;
        if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW Global1 VTU VID Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Global1 VTU VID Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if (reg_val != wr_data) {
            printf("%s: Failed to set VLAN%d VID.\n", __FUNCTION__, ctr);
            return (FAILED);
        }

        /* Config. FID */
        prpass(testpass, "Config. VLAN%d FID", ctr);

        wr_data = (ushort)ctr;
        reg_addr = (int)ESW_G1_VTUFID_REG;
        if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW Global1 VTU FID Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Global1 VTU FID Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if (reg_val != wr_data) {
            printf("%s: Failed to set VLAN%d FID.\n", __FUNCTION__, ctr);
            return (FAILED);
        }

        /* Config. OPs Data (port0~7) */
        prpass(testpass, "Config. VLAN%d OPs Data(port0~7)", ctr);

        wr_data = 0xFFFF;

        if (ctr == ESW_VLAN1) {
            wr_data &= (ushort)(~((ESW_G1_MEMBER_STATE_MSK << 2) |
                                  (ESW_G1_MEMBER_STATE_MSK << 4)));
            wr_data |= (ushort)((ESW_G1_DATA_FRAME_UNTAGGED << 2) |
                                (ESW_G1_DATA_FRAME_UNTAGGED << 4));
        } else if (ctr == ESW_VLAN2) {
            wr_data &= (ushort)(~((ESW_G1_MEMBER_STATE_MSK << 6) |
                                  (ESW_G1_MEMBER_STATE_MSK << 8)));
            wr_data |= (ushort)((ESW_G1_DATA_FRAME_UNTAGGED << 6) |
                                (ESW_G1_DATA_FRAME_UNTAGGED << 8));
        } else if (ctr == ESW_VLAN3) {
            wr_data &= (ushort)(~((ESW_G1_MEMBER_STATE_MSK << 10) |
                                  (ESW_G1_MEMBER_STATE_MSK << 12)));
            wr_data |= (ushort)((ESW_G1_DATA_FRAME_UNTAGGED << 10) |
                                (ESW_G1_DATA_FRAME_UNTAGGED << 12));
        } else if (ctr == ESW_VLAN4) {
            wr_data &= (ushort)(~(ESW_G1_MEMBER_STATE_MSK << 14));
            wr_data |= (ushort)(ESW_G1_DATA_FRAME_UNTAGGED << 14);
        }

        reg_addr = (int)ESW_G1_VTUDATA_0TO7_REG;
        if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW Global1 VTU Data Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Global1 VTU Data Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if (reg_val != wr_data) {
            printf("%s: Failed to set VLAN%d port0~7 Data.\n",
                   __FUNCTION__, ctr);
            return (FAILED);
        }

        /* Config. OPs Data (port8~10) */
        prpass(testpass, "Config. VLAN%d OPs Data(port8~10)", ctr);

        wr_data = 0x3F;
        if (ctr == ESW_VLAN4) {
            wr_data &= (ushort)(~ESW_G1_MEMBER_STATE_MSK);
            wr_data |= (ushort)ESW_G1_DATA_FRAME_UNTAGGED;
        }

        reg_addr = (int)ESW_G1_VTUDATA_8TO10_REG;
        if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW Global1 VTU Data Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Global1 VTU Data Reg.(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if (reg_val != wr_data) {
            printf("%s: Failed to set VLAN%d port8~10 Data.\n",
                   __FUNCTION__, ctr);
            return (FAILED);
        }

        /* Config. VTU Operation */
        prpass(testpass, "Config. VLAN%d VTU Operation", ctr);

        wr_data = (ushort)(ESW_G1_OP_VTUBUSY | ESW_G1_OP_VTULOAD);
        reg_addr = (int)ESW_G1_VTUOP_REG;
        if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
            printf("%s: Failed to write ESW Global1 VTU Operation Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        for (retry_ctr = 0; retry_ctr < TSN_ESW_RETRY_MAX; retry_ctr++) {
            reg_val = 0;
            if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
                printf("%s: Failed to read ESW Global1 VTU Operation Reg."
                       "(0x%02X)\n",
                       __FUNCTION__, reg_addr);
                return (FAILED);
            }

            if ((reg_val & (ushort)ESW_G1_OP_VTUBUSY) != ESW_G1_OP_VTUBUSY) {
                break;
            }

            if (retry_ctr == (TSN_ESW_RETRY_MAX - 1)) {
                printf("%s: Something wrong !! VTU can't finish VLAN%d setup.\n",
                       __FUNCTION__, ctr);
                return (FAILED);
            }
            msleep(10);
        }
    }

    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_8port_vlan_config
 * Description: Function to set TSN ethernet switch(Marvell 88E6390) VLAN and
 *              forwarding for Diag extending feature.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_esw_8port_vlan_config (int opt)
{
    if (tsn_esw_std_pair_vlan(ESW_VLAN4) != PASSED) {
        printf("Failed to configure VLANs on ESW port1 ~ 8.\n");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_4port_vlan_config
 * Description: Function to set TSN ethernet switch(Marvell 88E6390)
 *              4ports VLAN and forwarding for Diag extending feature.
 *              This setup is for Test item 2 in spec.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_esw_4port_vlan_config (int opt)
{
    if (tsn_esw_std_pair_vlan(ESW_VLAN2) != PASSED) {
        printf("Failed to configure VLANs on ESW port1 ~ 4.\n");
        return (FAILED);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_traffic_test_esw_config
 * Description: Function to configure TSN ethernet switch(Marvell 88E6390)
 *              for Diag extending feature: Traffic test.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_traffic_test_esw_config (int opt)
{
    int    dev_addr = 0, reg_addr = 0;
    ushort reg_val = 0, wr_data = 0;
    int    retry_ctr = 0;

    if (tsn_esw_std_pair_vlan(ESW_VLAN4) != PASSED) {
        printf("Failed to configure VLANs on ESW port1 ~ 8 and CPU port.\n");
        return (FAILED);
    }

    /* Configure port0(TSN-H: WiFi) */
    reg_addr = (int)ESW_PORTCTR_REG;

    /* Set port 0 forwarding */
    prpass(testpass, "Check ESW port%d forwarding feature.", ESW_PORT0);

    if (tsn_esw_reg_rd((int)ESW_PORT0, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW port%d port control Reg.(0x%02X)\n",
               __FUNCTION__, ESW_PORT0, reg_addr);
        return (FAILED);
    }

    if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
        reg_val |= (ushort)(ESW_PCR_FORWARD);

        if (tsn_esw_reg_wr((int)ESW_PORT0, reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write ESW port%d port control Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, ESW_PORT0, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd((int)ESW_PORT0, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW port%d port control Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, ESW_PORT0, reg_addr);
            return (FAILED);
        }

        if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
            printf("%s: Failed to enable ESW port%d forwarding feature.",
                   __FUNCTION__, ESW_PORT0);
            return (FAILED);
        }
    }

    /* Set VLAN ID */
    reg_val = 0;
    reg_addr = (int)ESW_PORTVLAN_ID_REG;
    if (tsn_esw_reg_rd((int)ESW_PORT0, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, __LINE__, ESW_PORT0, reg_addr);
        return (FAILED);
    }

    wr_data = (reg_val &
               (ushort)(~(ESW_PVID_FORCE_DVID | ESW_PVID_DVID_MSK)));
    wr_data |= (ushort)ESW_VLAN4;

    if (tsn_esw_reg_wr((int)ESW_PORT0, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW port%d port default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, ESW_PORT0, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd((int)ESW_PORT0, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, __LINE__, ESW_PORT0, reg_addr);
        return (FAILED);
    }

    if ((reg_val & ESW_PVID_FORCE_DVID) == ESW_PVID_FORCE_DVID) {
        printf("%s: Failed to clear ESW port%d Force VLAN ID.",
               __FUNCTION__, ESW_PORT0);
        return (FAILED);
    }

    if ((reg_val & ESW_PVID_DVID_MSK) != ESW_VLAN4) {
        printf("%s: Failed to set ESW port%d VLAN ID to %d.",
               __FUNCTION__, ESW_PORT0, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. CPU port */
    /* Set CPU port(TSN-H: port 9) forwarding */
    prpass(testpass, "Check ESW port%d forwarding feature.", ESW_PORT9);

    if (tsn_esw_reg_rd((int)ESW_PORT9, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW port%d port control Reg.(0x%02X)\n",
               __FUNCTION__, ESW_PORT9, reg_addr);
        return (FAILED);
    }

    if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
        reg_val |= (ushort)(ESW_PCR_FORWARD);

        if (tsn_esw_reg_wr((int)ESW_PORT9, reg_addr, reg_val) != PASSED) {
            printf("%s: Failed to write ESW port%d port control Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, ESW_PORT9, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd((int)ESW_PORT9, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW port%d port control Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, ESW_PORT9, reg_addr);
            return (FAILED);
        }

        if ((reg_val & ESW_PCR_PSTAT_MSK) != ESW_PCR_FORWARD) {
            printf("%s: Failed to enable ESW port%d forwarding feature.",
                   __FUNCTION__, ESW_PORT9);
            return (FAILED);
        }
    }

    /* Set 802.1Q mode and VLAN ID */
    prpass(testpass, "Config. ESW port%d 802.1Q mode and VLAN ID", ESW_PORT9);

    /* Set VLAN ID */
    reg_val = 0;
    reg_addr = (int)ESW_PORTVLAN_ID_REG;
    if (tsn_esw_reg_rd((int)ESW_PORT9, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, __LINE__, ESW_PORT9, reg_addr);
        return (FAILED);
    }

    wr_data = (reg_val &
               (ushort)(~(ESW_PVID_FORCE_DVID | ESW_PVID_DVID_MSK)));
    wr_data |= (ushort)ESW_VLAN4;

    if (tsn_esw_reg_wr((int)ESW_PORT9, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW port%d port default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, ESW_PORT9, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd((int)ESW_PORT9, reg_addr, &reg_val) != PASSED) {
        printf("%s:%d Failed to read ESW port%d default VLAN ID Reg."
               "(0x%02X)\n",
               __FUNCTION__, __LINE__, ESW_PORT9, reg_addr);
        return (FAILED);
    }

    if ((reg_val & ESW_PVID_FORCE_DVID) == ESW_PVID_FORCE_DVID) {
        printf("%s: Failed to clear ESW port%d Force VLAN ID.",
               __FUNCTION__, ESW_PORT9);
        return (FAILED);
    }

    if ((reg_val & ESW_PVID_DVID_MSK) != ESW_VLAN4) {
        printf("%s: Failed to set ESW port%d VLAN ID to %d.",
               __FUNCTION__, ESW_PORT9, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. VLANs */
    dev_addr = (int)ESW_SMIDEV_GLOB1;

    /* Set VLAN ID */
    prpass(testpass, "Config. VLAN%d VID", ESW_VLAN4);

    wr_data = (ushort)(ESW_G1_VID_ENTRY_VALID | ESW_VLAN4);
    reg_addr = (int)ESW_G1_VTUVID_REG;
    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW Global1 VTU VID Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW Global1 VTU VID Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed to set VLAN%d VID.\n", __FUNCTION__, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. FID */
    prpass(testpass, "Config. VLAN%d FID", ESW_VLAN4);

    wr_data = (ushort)ESW_VLAN4;
    reg_addr = (int)ESW_G1_VTUFID_REG;
    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW Global1 VTU FID Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW Global1 VTU FID Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed to set VLAN%d FID.\n", __FUNCTION__, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. OPs Data (port0~7) */
    prpass(testpass, "Config. VLAN%d OPs Data(port0~7)", ESW_VLAN4);

    wr_data = 0xFFFF;
    wr_data &= (ushort)(~((ESW_G1_MEMBER_STATE_MSK << 14) |
                          (ESW_G1_MEMBER_STATE_MSK)));
    wr_data |= (ushort)((ESW_G1_DATA_FRAME_UNTAGGED << 14) |
                        (ESW_G1_DATA_FRAME_UNTAGGED));

    reg_addr = (int)ESW_G1_VTUDATA_0TO7_REG;
    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW Global1 VTU Data Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW Global1 VTU Data Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed to set VLAN%d port0~7 Data.\n",
               __FUNCTION__, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. OPs Data (port8~10) */
    prpass(testpass, "Config. VLAN%d OPs Data(port8~10)", ESW_VLAN4);
    wr_data = 0x35;

    reg_addr = (int)ESW_G1_VTUDATA_8TO10_REG;
    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW Global1 VTU Data Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    reg_val = 0;
    if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
        printf("%s: Failed to read ESW Global1 VTU Data Reg.(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    if (reg_val != wr_data) {
        printf("%s: Failed to set VLAN%d port8~10 Data.\n",
               __FUNCTION__, ESW_VLAN4);
        return (FAILED);
    }

    /* Config. VTU Operation */
    prpass(testpass, "Config. VLAN%d VTU Operation", ESW_VLAN4);

    wr_data = (ushort)(ESW_G1_OP_VTUBUSY | ESW_G1_OP_VTULOAD);
    reg_addr = (int)ESW_G1_VTUOP_REG;
    if (tsn_esw_reg_wr(dev_addr, reg_addr, wr_data) != PASSED) {
        printf("%s: Failed to write ESW Global1 VTU Operation Reg."
               "(0x%02X)\n",
               __FUNCTION__, reg_addr);
        return (FAILED);
    }

    for (retry_ctr = 0; retry_ctr < TSN_ESW_RETRY_MAX; retry_ctr++) {
        reg_val = 0;
        if (tsn_esw_reg_rd(dev_addr, reg_addr, &reg_val) != PASSED) {
            printf("%s: Failed to read ESW Global1 VTU Operation Reg."
                   "(0x%02X)\n",
                   __FUNCTION__, reg_addr);
            return (FAILED);
        }

        if ((reg_val & (ushort)ESW_G1_OP_VTUBUSY) != ESW_G1_OP_VTUBUSY) {
            break;
        }

        if (retry_ctr == (TSN_ESW_RETRY_MAX - 1)) {
            printf("%s: Something wrong !! VTU can't finish VLAN%d setup.\n",
                   __FUNCTION__, ESW_VLAN4);
            return (FAILED);
        }
        msleep(10);
    }
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_esw_vlan_profile_config
 * Description: Wrapped function for Diag extended feature.
 *              It provides some profiles for user to set 802.1Q VLANs in
 *              TSN ethernet switch(Marvell 88E6390) and enable all configured
 *              ports forwarding feature.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_esw_vlan_profile_config (int opt)
{
    int p_num = 1, ret_val = FAILED;

    printf("\nEach provides below will do following things:\n");
    printf("1. Enable Switch forwarding feature for all configured ports.\n");

    if (this_is_tsn_h_sku() == TRUE) {
        printf("2. Configure Switch VLANs(802.1q).\n\n");

        printf("Profile 1:\n");
        printf("  VLAN1: Port1 and Port2.\n");
        printf("  VLAN2: Port3 and Port4.\n");
        printf("  VLAN3: Port5 and Port6.\n");
        printf("  VLAN4: Port7, Port8, CPU port and WiFi port.\n\n");

        printf("Profile 2:\n");
        printf("  VLAN1: Port1 and Port2.\n");
        printf("  VLAN2: Port3 and Port4.\n");
        printf("  VLAN3: Port5 and Port6.\n");
        printf("  VLAN4: Port7 and Port8.\n\n");

        printf("Profile 3:\n");
        printf("  VLAN1: Port1 and Port2.\n");
        printf("  VLAN2: Port3 and Port4.\n\n");

        p_num = getdec_answer("Enter Profile number(1 ~ 3): ", 1, 1, 3);

        switch(p_num) {
        case PROFILE_1:
            ret_val = tsn_traffic_test_esw_config(0);
        break;
        case PROFILE_2:
            ret_val = tsn_esw_8port_vlan_config(0);
        break;
        case PROFILE_3:
            ret_val = tsn_esw_4port_vlan_config(0);
        break;
        default:  
            printf("Invalid profile ID(%d).\n", p_num);
            return (FAILED);
        break;
        }

        if (ret_val != PASSED) {
            printf("Failed to config. Switch VLANs as profile%d shown.\n",
                   p_num);
            return (FAILED);
        }
    } else {
        /* TSN-M Switch VLAN utility */
        printf("2. Configure Switch VLANs.\n\n");

        if (tsn_m_esw_vlan_config(0) != PASSED) {
            printf("Failed to config. TSN-M Switch VLANs.\n");
            return (FAILED);
        }
    }

    printf("Done config. Switch VLANs successfully.\n");
    return (PASSED);
}

/*******************************************************************************
 *
 * Function   : tsn_m_esw_vlan_config
 * Description: Function to config TSN-M Switch VLANs for Compliance team.
 * Inputs     : opt - reserved for future use
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
static int tsn_m_esw_vlan_config (int opt) {
    int    ctr = 0, smi_addr = 0;
    int    reg_addr = (int)ESW_PORT_VLAN_REG;
    ushort reg_val = 0, wr_in = 0;

    printf("VLAN setups:\n");
    printf(" -VLAN0: LAN0 and LAN1.\n");
    printf(" -VLAN1: LAN2 and LAN3.\n\n");

    for (ctr = ESW_PORT0; ctr <= ESW_PORT3; ctr++) {
        smi_addr = (int)(ctr + TSN_M_ESW_PORT_REG_BASE);
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("smi_addr = 0x%02X.\n", smi_addr);
        }

        if (tsn_esw_reg_rd(smi_addr, reg_addr, &reg_val) != PASSED) {
            printf("Failed to read LAN%d Port Based VLAN Map Reg.(0x%02X)\n",
                   ctr, reg_addr);
            return (FAILED);
        }

        reg_val &= (ushort)(~ESW_PBVM_VLAN_TBL_MSK);

        switch (ctr) {
        case ESW_PORT0:
            reg_val |= (ushort)(ESW_PBVM_VLAN_TBL(ESW_PORT1));
        break;
        case ESW_PORT1:
            reg_val |= (ushort)(ESW_PBVM_VLAN_TBL(ESW_PORT0));
        break;
        case ESW_PORT2:
            reg_val |= (ushort)(ESW_PBVM_VLAN_TBL(ESW_PORT3) |
                                ESW_PBVM_VLAN_TBL(ESW_PORT5));
        break;
        case ESW_PORT3:
            reg_val |= (ushort)(ESW_PBVM_VLAN_TBL(ESW_PORT2) |
                                ESW_PBVM_VLAN_TBL(ESW_PORT5));
        break;
        default:  
            printf("Invalid ESW port ID(%d).\n", ctr);
            return (FAILED);
        break;
        }
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("reg_val = 0x%04X.\n", reg_val);
        }

        wr_in = reg_val;
        if (tsn_esw_reg_wr(smi_addr, reg_addr, wr_in) != PASSED) {
            printf("Failed to write LAN%d Port Based VLAN Map Reg.(0x%02X)\n",
                   ctr, reg_addr);
            return (FAILED);
        }

        reg_val = 0;
        if (tsn_esw_reg_rd(smi_addr, reg_addr, &reg_val) != PASSED) {
            printf("Failed to read LAN%d Port Based VLAN Map Reg."
                   "(0x%02X) for confirm.\n",
                   ctr, reg_addr);
            return (FAILED);
        }

        if (reg_val != wr_in) {
            printf("Failed to set LAN%d VLAN Table.\n", ctr);
            return (FAILED);
        }
    }
    return (PASSED);
}


/*-------------------------------------------------
$Log: diag_ext.c,v $
Revision 1.2  2017/08/02 14:21:44  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:02  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.2  2017/07/20 13:38:03  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.8.2.1  2017/07/18 03:53:00  steja
Code cleanup

Revision 1.1.4.8  2016/09/28 04:36:15  palin2
Added CPU to ESW PHY MAC loopback test.

Revision 1.1.4.7  2016/09/23 07:09:37  palin2
Added TSN-M Switch VLANs config. profile for compliance team.

Revision 1.1.4.6  2016/09/09 03:02:05  steja
Fix VLAN setup configuration

Revision 1.1.4.5  2016/07/29 14:27:47  palin2
Added utility and function to config. Switch port to specific speed and mode.

Revision 1.1.4.4  2016/07/26 16:05:24  palin2
Fixed ESW VLANs setup function.

Revision 1.1.4.3  2016/07/21 14:26:32  steja
1. Update Temperature sensor function
2. Add Wifi Temperature sensor to basic utilities
3. Add LTE Extended Feature Test
4. Display Chasis Temperature When Boot up
5. Update I2C scan function
6. Update POE cookie check card present

Revision 1.1.4.2  2016/06/30 06:22:47  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.7  2016/06/21 12:58:52  steja
Add OTA out of reset utility

Revision 1.1.2.6  2016/06/17 15:26:25  palin2
Added WLAN module diags and utilities.

Revision 1.1.2.5  2016/06/16 05:51:30  steja
Update software bridge and iptable profiles

Revision 1.1.2.4  2016/06/16 03:00:44  palin2
Added utility to config Switch VLANs automatically by choosing profiles.

Revision 1.1.2.3  2016/06/15 14:46:16  palin2
Added utilities to config Switch VLAN and forwarding for extended feature.

Revision 1.1.2.2  2016/06/13 16:55:21  palin2
Added Diag extending feature, Switch forwarding and VLAN for 8 ports snake test.

Revision 1.1.2.1  2016/05/16 09:26:08  steja
Checkin diag extended feature

*/
