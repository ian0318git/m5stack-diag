/* $Id: platform_phy.h,v 1.2 2021/06/02 02:56:21 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/hightower-mmwv/platform_phy.h,v $
 *********************************************************************
 *
 * platform_phy.h -
 *
 * Copyright (c) 2020-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *********************************************************************
 */

#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPIInternal.h"
#include "mtdFwDownload.h"
#include "mtdAPI.h"
#include "mtdHXunit.h"
#include "mtdApiRegs.h"
#include "mtdInitialization.h"
#include "mtdDiagnostics.h"
#include "mtdDiagnosticsRegDumpData.h"
#include "mtdIntr.h"
#include "mtdCunit.h"


/* Set default debug log level ref by sdk */
extern MTD_DBG_LEVEL mtd_debug_level;

enum {
    MTD_PORT_0 = 0x1c,
    MAX_MTD_PORTS,
};

enum {
    MARVELL_MTD_MODE_NONE,
    MARVELL_MTD_MODE_10M,
    MARVELL_MTD_MODE_100M,
    MARVELL_MTD_MODE_1G,
    MARVELL_MTD_MODE_2P5G,
    MARVELL_MTD_MODE_5G,
    MARVELL_MTD_MODE_10G,
    MARVELL_MTD_MODE_MAX_NR,
};

enum {
    MARVELL_MTD_PORT_SPEED_UNKNOWN,
    MARVELL_MTD_PORT_SPEED_10M,
    MARVELL_MTD_PORT_SPEED_100M,
    MARVELL_MTD_PORT_SPEED_1GB,
    MARVELL_MTD_PORT_SPEED_2P5GB,
    MARVELL_MTD_PORT_SPEED_5GB,
    MARVELL_MTD_PORT_SPEED_10GB,
    MARVELL_MTD_PORT_SPEED_MAX_NR,
};


enum {
    MARVELL_MTD_PORT_LINK_UP,
    MARVELL_MTD_PORT_LINK_DOWN,
    MARVELL_MTD_PORT_LINK_UNKNOWN,
};


int mtd_sgmii_speed[MARVELL_MTD_MODE_MAX_NR] = {
        [MARVELL_MTD_MODE_10M]    = MTD_SGMII_SPEED_10M,
        [MARVELL_MTD_MODE_100M]   = MTD_SGMII_SPEED_100M,
        [MARVELL_MTD_MODE_1G]     = MTD_SGMII_SPEED_1G,
        [MARVELL_MTD_MODE_2P5G]     = MTD_SGMII_SPEED_1G,
};

int mtd_t_speed[MARVELL_MTD_MODE_MAX_NR] = {
        [MARVELL_MTD_MODE_10M]    = MTD_SPEED_10M_FD,
        [MARVELL_MTD_MODE_100M]   = MTD_SPEED_100M_FD,
        [MARVELL_MTD_MODE_1G]     = MTD_SPEED_1GIG_FD,
        [MARVELL_MTD_MODE_2P5G]   = MTD_SPEED_2P5GIG_FD,
        [MARVELL_MTD_MODE_5G]     = MTD_SPEED_5GIG_FD,
        [MARVELL_MTD_MODE_10G]    = MTD_SPEED_10GIG_FD,
};

char* mtd_speed_desc[MTD_ALL_SPEEDS_AVAILABLE] = {
    [MTD_SPEED_10M_HD]      = "10M_HD",
    [MTD_SPEED_10M_FD]      = "10M_FD",
    [MTD_SPEED_100M_HD]     = "100M_HD",
    [MTD_SPEED_100M_FD]     = "100M_FD",
    [MTD_SPEED_1GIG_HD]     = "1GIG_HD",
    [MTD_SPEED_1GIG_FD]     = "1GIG_FD",
    [MTD_SPEED_2P5GIG_FD]   = "2P5GIG_FD",
    [MTD_SPEED_5GIG_FD]     = "5GIG_FD",
    [MTD_SPEED_10GIG_FD]    = "10GIG_FD",
    [MTD_SPEED_10M_HD_AN_DIS] = "10M HD AN DIS",
    [MTD_SPEED_10M_FD_AN_DIS]  = "10M FD AN DIS",
    [MTD_SPEED_100M_HD_AN_DIS] = "100M HD AN DIS",
    [MTD_SPEED_100M_FD_AN_DIS] = "100M FD AN DIS",
};


enum {
    MTD_COPPER_ONLY = 0,
    MTD_FIBER_ONLY = 1,
    MTD_AUTO_MEDIA_COPPER_PRE = 2,
    MTD_AUTO_MEDIA_FIBER_PRE = 3,
    MTD_REDUNDANT = 4,
    MTD_RESERVED_1 = 5,
    MTD_RESERVED_2 = 6,
    MTD_AUTO_MEDIA = 7,
};

/*********************************************************************
 * $Log: platform_phy.h,v $
 * Revision 1.2  2021/06/02 02:56:21  alpeng
 * merge sears into trunk
 *
 * Revision 1.1.4.1  2020/08/27 07:19:33  alpeng
 * apply cvs header
 *
 *
 * $Endlog$
 */

