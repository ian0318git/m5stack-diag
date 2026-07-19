/* $Id: platform_poe_psu.h,v 1.3 2017/10/19 13:41:11 palin2 Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/platform_poe_psu.h,v $ 
 *------------------------------------------------------------------
 *
 * Filename   : platform_poe_psu.h
 * Description: Header file of TSN PoE PSU(TI, TPS2386) Library.
 *
 * Copyright (c) 2016 ~ 2017 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#ifndef __PLATFORM_POE_PSU_H__
#define __PLATFORM_POE_PSU_H__

/* Common defines */
#define TPS2386B_POE_PORTS       4
#define PSE_PORT_ONE             1
#define PSE_PORT_TWO             2
#define PSE_PORT_THREE           3
#define PSE_PORT_FOUR            4
#define PSE_ALL_PORTS            5

#define UNDEFINED_POE_PORTS      0xFFFF
#define TSN_PSE_ACCESS_TIME      1500     /* 1.5 sec */
#define TSN_PSE_CHECK_INTERVAL   50       /* 50ms */
#define PSE_I2C_CHK_INTERVAL     200      /* 200ms */
#define TSN_PSE_RST_WAIT_TIME    10       /* 10ms */
#define PSE_CHKTIME_MAX          5000     /* 5 sec */
#define PSE_CHK_INTVAL           1000     /* 1 sec */
#define WAIT_PD_READY_INTVAL     1000     /* 1 sec */
#define WAIT_PD_READY_MAX        5
#define PSE_CLA_TIME             30       /* 30ms */
#define PSE_PWRON_MAX_MICROSEC   1000000  /* 1sec */
#define PSE_DETCLA_MAX_MICROSEC  1000000  /* 1sec */
#define TSN_PSE_PPWR_CHK_TIME    10000    /* 10sec */

#define TSN_PSE_CURR_CHK_THR     350      /* 350mA */
#define PSE_PWR_POLL_MAX         50

typedef struct pse_pstat {
    int   p_num;
    int   p_opmod;
    int   p_det;
    int   p_class;
    int   p_onoff;
    int   p_pwr_stat;
    double   p_curr;
    double   p_volt;
    double   p_pwr;
} pse_pstat_t;

typedef struct pse_pinfo {
    int         p_num;
    pse_pstat_t p_stat;
} pse_pinfo_t;

typedef struct pse_class_stat {
    char   *str;
} pse_class_stat_t;

typedef struct pse_det_stat {
    char   *str;
} pse_det_stat_t;

typedef struct pse_cmd_info {
    char     *name;
    uint32_t cmd_code;
    int      type;
    char     data_byte;
} pse_cmd_info_t;

/* Register map */
#define TPS2386B_INTR_REG                0x00
#define TPS2386B_INTR_MASK_REG           0x01
#define TPS2386B_PWREVENT_REG            0x02
#define TPS2386B_PWREVENT_COR_REG        0x03
#define TPS2386B_DET_REG                 0x04
#define TPS2386B_DET_COR_REG             0x05
#define TPS2386B_FAULTEVENT_REG          0x06
#define TPS2386B_FAULTEVENT_COR_REG      0x07
#define TPS2386B_STARTEVENT_REG          0x08
#define TPS2386B_STARTEVENT_COR_REG      0x09
#define TPS2386B_SUPPLYEVENT_REG         0x0A
#define TPS2386B_SUPPLYEVENT_COR_REG     0x0B
#define TPS2386B_PSTAT_REG_BASE          0x0C
#define TPS2386B_P1_STATUS_REG           0x0C
#define TPS2386B_P2_STATUS_REG           0x0D
#define TPS2386B_P3_STATUS_REG           0x0E
#define TPS2386B_P4_STATUS_REG           0x0F
#define TPS2386B_PWR_STAT_REG            0x10
#define TPS2386B_I2C_SLVADDR_REG         0x11
#define TPS2386B_OP_MODE_REG             0x12
#define TPS2386B_DISCONN_EN_REG          0x13
#define TPS2386B_DETCLA_EN_REG           0x14
#define TPS2386B_PWRRR_ICUT_DIS_REG      0x15
#define TPS2386B_TIMING_CONF_REG         0x16
#define TPS2386B_GENERAL_MASK_REG        0x17
#define TPS2386B_DETCLA_RESTART_REG      0x18
#define TPS2386B_PWR_EN_REG              0x19
#define TPS2386B_RESET_REG               0x1A
#define TPS2386B_ID_REG                  0x1B
#define TPS2386B_POLICE21_CONF_REG       0x1E
#define TPS2386B_POLICE43_CONF_REG       0x1F
#define TPS2386B_IEEE_PWR_EN_REG         0x23
#define TPS2386B_IEEE_PWRFAULT_REG       0x24
#define TPS2386B_IEEE_PWRFAULT_COR_REG   0x25
#define TPS2386B_TEMP_REG                0x2C
#define TPS2386B_IN_VOLT_REG             0x2E
#define TPS2386B_P1_CURR_REG             0x30
#define TPS2386B_P1_VOLT_REG             0x32
#define TPS2386B_P2_CURR_REG             0x34
#define TPS2386B_P2_VOLT_REG             0x36
#define TPS2386B_P3_CURR_REG             0x38
#define TPS2386B_P3_VOLT_REG             0x3A
#define TPS2386B_P4_CURR_REG             0x3C
#define TPS2386B_P4_VOLT_REG             0x3E
#define TPS2386B_POEPLUS_REG             0x40
#define TPS2386B_FW_REV_REG              0x41
#define TPS2386B_I2C_WD_REG              0x42
#define TPS2386B_DEV_ID_REG              0x43
#define TPS2386B_COOL_DOWN_REG           0x45

#define PSE_DETECT_MSK           0xF
#define PSE_DET_VALID            0x4
#define PSE_CLASS_MSK            0xF
#define PSE_CLASS_SHIFT          4

#define PSE_PORT_ONOFF_MSK       0x1
#define PSE_PWR_STAT_MSK         0x1
#define PSE_PWR_STAT_SHIFT       4

/* Interrupt Mask Reg(0x1) */
#define PSE_INTR_MSK_MASKED_ALL  0

/* Power Event Reg(0x02) */
#define PSE_PWRE_PEC             0x01
#define PSE_PWRE_PGC             0x10
#define PSE_PWRE_PGCPEC          (PSE_PWRE_PGC | PSE_PWRE_PEC)
#define PSE_PWRE_PORT_PGCPEC(x)  (PSE_PWRE_PGCPEC << (x - 1))

/* Detection Event Reg(0x04) */
#define PSE_DET_RST_VAL          0
#define PSE_DETE_CLADET          0x11
#define PSE_DETE_PORT_CLADET(x)  (PSE_DETE_CLADET << (x - 1))

/* Detection Event CoR Reg(0x5) */
#define PSE_DET_CLSC_MSK         0xf0
#define PSE_DET_CLSC_OFFSET      4
#define PSE_DET_DETC_MSK         0x0f

/* Port Status Reg(0x0C ~ 0x0F) */
#define PSE_PSTAT_REG(x)         (0x0C + (x - 1))
#define PSE_PSTAT_RST_VALUE      0
#define PSE_PSTAT_DET_MSK        0xF
#define PSE_PSTAT_DET_UNKNOWN    0x0
#define PSE_PSTAT_DET_TOO_LOW    0x3
#define PSE_PSTAT_DET_VALID      0x4
#define PSE_PSTAT_DET_TOO_HIGH   0x5
#define PSE_PSTAT_DET_OPEN_CIRC  0x6
#define PSE_PSTAT_DET_MOS_FAULT  0xF
#define PSE_PSTAT_CLA_MSK        0xF0
#define PSE_PSTAT_UNKNOWN_CLA    0x0
#define PSE_PSTAT_CLA1           0x1
#define PSE_PSTAT_CLA2           0x2
#define PSE_PSTAT_CLA3           0x3
#define PSE_PSTAT_CLA4           0x4
#define PSE_PSTAT_CLA0           0x6
#define PSE_PSTAT_CLA_OC         0x7
#define PSE_PSTAT_CLA_SHIFT      4

/* Power Status Reg(0x10) */
#define PSE_PWRSTAT_PE           0x01
#define PSE_PWRSTAT_PG           0x10
#define PSE_PWRSTAT_PGPE         (PSE_PWRSTAT_PG | PSE_PWRSTAT_PE)
#define PSE_PWRSTAT_PORT_PGPE(x) (PSE_PWRSTAT_PGPE << (x - 1))

/* Operating mode Reg(0x12) */
#define PSE_OPMODE_MSK           0x3
#define PSE_OP_OFF               0x0
#define PSE_OP_MANUAL            0x1
#define PSE_OP_SEMIAUTO          0x2
#define PSE_PORT_SEMIAUTO(x)   (PSE_OP_SEMIAUTO << (2 * (x - 1)))
#define PSE_PORT_MANUAL(x)     (PSE_OP_MANUAL << (2 * (x - 1)))
#define PSE_ALL_PORTS_SEMIAUTO   0xAA
#define PSE_PORT_OPMODE_MSK(x)   (PSE_OPMODE_MSK << (2 * (x - 1)))
#define PSE_PORT_OP_CHG_TIME     1000 /* 1sec */

/* Disconnect Enable Reg(0x13) */
#define PSE_ALL_DISCONN_EN       0x0F

/* Detect/Class Reg(0x14) */
#define PSE_DETCLA_EN_CLEDETE    0x11
#define PSE_DETCLA_DET_EN(x)     (1 << (x - 1))
#define PSE_DETCLA_CLA_EN(x)     (1 << (4 * (x - 1)))
#define PSE_DETCLA_EN(x)         (PSE_DETCLA_DET_EN(x) | PSE_DETCLA_CLA_EN(x))
#define PSE_DETCLA_EN_MSK(x)     (PSE_DETCLA_EN(x))
#define PSE_ALL_PORTS_DETCLA     0xFF

/* Power Enable Reg(0x19) */
#define PSE_PWR_ON(x)   (1 << (x - 1))
#define PSE_PWR_OFF(x)  (1 << (x + 3))
#define PSE_ALL_PORTS_PWR_ON     0x0F

/* Reset Reg(0x1A) */
#define PSE_RESET_ALL_PORTS      0x10
#define PSE_RESET_PORT_BIT(x)    (1 << (x - 1))

#define PSE_P_CURR_REG_BASE      0x30
#define PSE_CURR_BASE            61.035
#define PSE_CURR_REG_BASE(x)     (PSE_P_CURR_REG_BASE + (4 * (x - 1)))

#define PSE_P_VOLT_REG_BASE      0x32
#define PSE_VOLT_BASE            3.662

/* Police 21 Configuration Reg(0x1E) */
/* Police 43 Configuration Reg(0x1F) */
#define PSE_POLICE_MAX           0xF
#define PSE_POLICE_PORT_MAX(x)   (PSE_POLICE_MAX << (4 * (1 - (x % 2))))
#define PSE_POLICE_MAX_ALL       0xFF
#define TSN_PSE_POLICE_VAL       0x8
#define PSE_POLICE_PORT_MSK(x)   (0xF << (4 * (1 - (x % 2))))
#define TSN_PSE_PORT_POL_VAL(x)  (TSN_PSE_POLICE_VAL << (4 * (1 - (x % 2))))
#define PSE_POL_ODD_MSK          0xF
#define PSE_POL_EVEN_MSK         0xF0
#define PSE_POL_EVEN_SHIFT       4
#define PSE_ICUT_MAX             0xF
#define PSE_ICUT_320MA           0xF
#define PSE_ICUT_640MA           0x8

/* IEEE Power Enable Reg(0x23) */
#define PSE_IEEE_TP2_SHIFTBASE   4
#define PSE_IEEE_TP2_ALL_ON      0xF0
#define PSE_IEEE_TP2_ON          0x10
#define PSE_IEEE_TP1_ON          0x1
#define PSE_IEEE_TP2_EN(x)       (PSE_IEEE_TP2_ON << (x - 1))
#define PSE_IEEE_TP1_EN(x)       (PSE_IEEE_TP1_ON << (x - 1))

/* PoE Plus Reg(0x40) */
#define PSE_POEPLUS              0x10
#define PSE_PORT_POEPLUS(x)      (PSE_POEPLUS << (x - 1))
#define PSE_TPON                 0x01
#define PSE_PWR_IEEE_TP1         1
#define PSE_PWR_IEEE_TP2         2

/* Externs */
extern int plat_psu_utils(int);
extern int tsn_psu_reg_rd(uint32_t, char *);
extern int tsn_psu_reg_wr(uint32_t, uchar);

#endif /* __PLATFORM_POE_PSU_H__ */

/*------------------------------------------------------------------
$Log: platform_poe_psu.h,v $
Revision 1.3  2017/10/19 13:41:11  palin2
Fixed CSCvg23616: TSN PoE link down intermittently when connect to iPorter PoE tester.

Revision 1.2  2017/08/02 14:21:49  steja
Support TSN-H/M platform code

Revision 1.1.8.2  2017/07/29 03:41:20  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.1.6.4  2017/07/24 14:14:11  palin2
1. To improve code readability.
2. All changes are verified before check-in.

Revision 1.1.6.3  2017/07/21 10:46:03  steja
Update based on code review comment

Revision 1.1.6.2  2017/07/20 13:38:07  steja
tsn-branch4 merge with maintrunk

Revision 1.1.4.5.2.1  2017/05/17 02:19:42  palin2
Updated function of triggering PoE PSE side interrupt.

Revision 1.1.4.5  2016/11/15 01:20:02  palin2
Added PoE PSU to FPGA interrupt test.

Revision 1.1.4.4  2016/10/03 00:38:49  palin2
Added utility to dump PSE registers.

Revision 1.1.4.3  2016/06/30 14:06:32  steja
Pick up the latest from tsn-branch1

Revision 1.1.4.2  2016/06/30 06:22:51  steja
tsn-branch2 sync with main trunk

Revision 1.1.2.4  2016/06/29 14:14:51  palin2
1. Updated code to support TSN-M.
2. Added utility to set LAN PHY 1000Base-T Test mode.

Revision 1.1.2.3  2016/06/16 07:51:11  palin2
Updated PoE related utilities and code.

Revision 1.1.2.2  2016/05/10 06:17:33  palin2
Updated PoE PSE related diag code after bring up.

Revision 1.1.2.1  2016/03/22 22:19:12  palin2
Added PoE PSU Diag.

$Endlog$
*/

