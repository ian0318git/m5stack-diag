#ifndef __SWITZER_MANHATTAN_BCM54194_TEST_H__
#define __SWITZER_MANHATTAN_BCM54194_TEST_H__

typedef struct bcm54194_regs_t_ {
    const char       *intfname;
    int               phy_intf;
    const reg_info_t *intfregs;
} bcm54194_regs_t;

#define REG_TST_NEP_PATTERN   0x5ADB


#endif
