#include <error.h>
#include <stdlib.h>

#include "types.h"
#include "queryflags.h"
#include "switzer_common.h"
#include "switzer_manhattan_bcm54194_api.h"

/*Format Rule:
**  Name static func with '_' preceeded
**  Name static global var  with  '_g_' preceeded
**  Name static var  with '_' preceede
*/

manhattan_bcm54194_t g_seahawks;
static manhattan_bcm54194_t *_g_p_seahawks = &g_seahawks;

int manhattan_front_port_to_54194_port_map_show(void)
{
    printf("----------------------+-------------------\n");
    printf("Front Pannel Port     |     BCM54194 Port \n");
    printf("----------------------+-------------------\n");
    printf("Rj45 port 0           |     Copper port 1 \n");
    printf("Rj45 port 1           |     Copper port 0 \n");
    printf("SFP  port 0           |     Fiber  port 1 \n");
    printf("SFP  port 1           |     Fiber  port 0 \n");
    printf("----------------------+-------------------\n");
    return 0;
}

int manhattan_bcm54194_init(void *priv,
    int (*rd)  (void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int *data),
    int (*wr)  (void *ctx, unsigned int mdio_addr, unsigned int reg_addr, unsigned int  data),
    int (*intr)(void *ctx, int act, void *arg),
    int (*rst) (void *ctx, int rst),
    char eth_map[MANHATTAN_BCM54194_PORT_NUMB][IFNAMSIZ])
{
    unsigned int reg = 0;
    unsigned int id  = 0;
    int          idx = 0;

    ERET_COND(!rd || !wr, -(__LINE__), "Invalid argument.\n");

    memset(&g_seahawks, 0, sizeof(g_seahawks));
    _g_p_seahawks->priv    = priv;
    _g_p_seahawks->mdio_rd = rd;
    _g_p_seahawks->mdio_wr = wr;
    _g_p_seahawks->reset   = rst;
    _g_p_seahawks->intr    = intr;
    memcpy(&_g_p_seahawks->eth_map[0][0], &eth_map[0][0], sizeof(_g_p_seahawks->eth_map));

    _g_p_seahawks->init_stage = MANHATTAN_BCM54194_INIT_STAGE_DONE_INIT;

    for(idx = 0; idx < MANHATTAN_BCM54194_PORT_NUMB; idx++) {
        printf("BCM54194 port-%d <-> %s\n", idx, eth_map[idx][0] ? &eth_map[idx][0] : "Unknown");
    }

    ERET_COND(0   != MHT_MDIO_RD(0xf, BCM54194_PHY_IDENTIFIER_MSB, &reg), -(__LINE__), "");
    ERET_COND(reg != BCM54194_ORG_UNIQ_ID, -(__LINE__),
            "Invalid BCM Org Uniq ID:%#x, should be %#x\n", reg, BCM54194_ORG_UNIQ_ID);
    id = reg << 16;

    ERET_COND(0 != MHT_MDIO_RD(0xf, BCM54194_PHY_IDENTIFIER_LSB, &reg), -(__LINE__), "");
    id |= reg;

    printf("%s Bcm54194 ID:%#x\n", __func__, id);

    manhattan_bcm54194_reset(1);

    manhattan_front_port_to_54194_port_map_show();

    return 0;
}

int manhattan_bcm54194_exit(void *priv)
{
    (void)priv;
    if (_g_p_seahawks->init_stage == MANHATTAN_BCM54194_INIT_STAGE_DONE_INIT) {
        manhattan_bcm54194_reset(0);
    }
    memset(&g_seahawks, 0, sizeof(g_seahawks));
    return 0;
}

/*
 * param:
 *    set < 0  - get current
 *    set >=0  - set current
 */
int manhattan_bcm54194_reg_verbose(int set)
{
    static int _verbo = 0;
    if (set >= 0)
        _verbo = !!set;
    return _verbo;
}

/*
 * Function: bcm54194_mdio45_reg_rd
 *
 * Description:
 * Read Broadcom 54194 PHY register.
 *
 * Input:
 * port - The MII phy id
 * dev - MMD
 * reg - Register to read
 *
 * Return: read_value/FAILED
 */
int manhattan_bcm54194_mdio45_reg_rd (int phy_addr, int dev, int reg, ushort *data)
{
    ushort regv = 0;

    /*1, select function addressing */
    regv  = 0;
    regv &= ~(0x3 << 14);         /* bit[15:14] specify function, 2'b00 means addressing. */
    regv |= dev & ((1 << 5) - 1); /* bit[4:0] specify dev addr */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DEVAD_REG, regv), -(__LINE__), "");

    /*2, set reg addr */
    regv  = reg;
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DATA_REG, regv), -(__LINE__), "");

    /*3, select function read */
    regv  = 0;
    regv |= (0x3 << 14);            /* bit[15:14] specify function, 2'b11 means read. */
    regv |= dev & ((1 << 5) - 1);   /* bit[4:0] specify dev addr */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DEVAD_REG, regv), -(__LINE__), "");

    /*4, read reg data */
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, BCM54194_C45_BY_C22_DATA_REG, &regv), -(__LINE__), "");

    *data = regv;
    return 0;
}

int manhattan_bcm54194_mdio45_reg_wr (int phy_addr, int dev, int reg, ushort data)
{
    ushort regv = 0;

    /*1, select function addressing */
    regv  = 0;
    regv &= ~(0x3 << 14);         /* bit[15:14] specify function, 2'b00 means addressing. */
    regv |= dev & ((1 << 5) - 1); /* bit[4:0] specify dev addr */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DEVAD_REG, regv), -(__LINE__), "");

    /*2, set reg addr */
    regv  = reg;
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DATA_REG, regv), -(__LINE__), "");

    /*3, select function write */
    regv  = 0;
    regv &= ~(0x3 << 14);
    regv |= 1 << 14;                /* bit[15:14] specify function, 2'b01 means read. */
    regv |= dev & ((1 << 5) - 1);   /* bit[4:0] specify dev addr */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DEVAD_REG, regv), -(__LINE__), "");

    /*4, write reg data */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_C45_BY_C22_DATA_REG, data), -(__LINE__), "");

    return 0;
}

int manhattan_bcm54194_rdb_access_enable (int phy_addr)
{
    /* Enable RDB access mode */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x17, 0x0F7E), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x15, 0x0000), -(__LINE__), "");
    return 0;
}

int manhattan_bcm54194_rdb_access_disable (int phy_addr)
{
    /* Disable RDB access mode */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1E, 0x0087), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1F, 0x8000), -(__LINE__), ""); /* ported code: why 2 times */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1F, 0x8000), -(__LINE__), "");
    return 0;
}

int manhattan_bcm54194_rdb_read (int phy_addr, int rdb_offset, uint16_t *reg_val)
{
    /* Read the RDB register */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1E, rdb_offset), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, 0x1F, reg_val   ), -(__LINE__), "");
    return 0;
}

int manhattan_bcm54194_rdb_write (int phy_addr, int rdb_offset, uint16_t reg_val)
{
    /* Write the RDB register */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1E, rdb_offset), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x1F, reg_val   ), -(__LINE__), "");
    return 0;
}

/*
 * Function: manhattan_bcm54194_switch_intf_access
 *
 * Description:
 * After reset, the Copper and Fiber registers space are accessible by default.
 * To switch from the Copper/Fiber register space to SGMII register space, the
 * following steps are required.
 * (All writes are to Port 0, but all ports are affected)
 *
 * 1. Switch from Copper/Fiber register space to SGMII register space:
 *     Write RDB 0x234, bits[6:5] = 2'b10
 *     Write RDB 0x234, bits[6:5] = 2'b01
 *     Write RDB 0x021, bits[0]   = 1'b1 (Access SGMII Registers)
 *
 * 2. Switch from SGMII register space to Copper/Fiber register space:
 *     Write RDB 0x234, bits[6:5] = 2'b10
 *     Write RDB 0x234, bits[6:5] = 2'b00
 *     Write RDB 0x021, bits[0]   = 1'b0 (Access Copper Registers)
 *                                = 1'b1 (Access Fiber Registers)
 *
 * Input:
 *        intf - what interface going to switch (0: SMII; 1: Copper; 2: Fiber)
 *
 * Return: PASSED/FAILED
 *
 */
int manhattan_bcm54194_switch_intf_access (manhattan_bcm54194_intf_t intf)
{
    uint16_t reg_val    = 0;
    uint16_t reg_val1   = 0;
    uint16_t reg_val2   = 0;
    uint16_t reg_val3   = 0;
    int      rdb_offset = 0;

    rdb_offset = BCM54194_EXTERNAL_SERDES_CTRL_REG;
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, rdb_offset, &reg_val1), -(__LINE__), "");
    reg_val1 &= ~(0x60); /* clr bit[6:5] */
    reg_val1 |= (0x40);  /* set bit[6:5] = 2'b10 */
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, rdb_offset,  reg_val1), -(__LINE__), "");

    switch(intf)
    {
        case MANHATTAN_BCM54194_INTF_SGMII  : reg_val2 = 0x20; reg_val3 = 0x1; break;
        case MANHATTAN_BCM54194_INTF_COPPER : reg_val2 = 0x00; reg_val3 = 0x0; break;
        case MANHATTAN_BCM54194_INTF_FIBER  : reg_val2 = 0x00; reg_val3 = 0x1; break;
        default: break;
    }

    rdb_offset = 0x234;
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, rdb_offset, &reg_val), -(__LINE__), "");
    reg_val &= ~(0x60);     /* clr bit[6:5] */
    reg_val |= reg_val2;
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, rdb_offset,  reg_val), -(__LINE__), "");

    rdb_offset = 0x021;
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, rdb_offset, &reg_val), -(__LINE__), "");
    reg_val &= ~(0x1);      /* clr bit[0] */
    reg_val |= reg_val3;
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, rdb_offset,  reg_val), -(__LINE__), "");

    /* Add some dealy for the access switching */
    switzer_mdelay(BCM54194_INTF_ACCESS_SWITCH_DELAY);
    return 0;
}

/*
 * Function: manhattan_bcm54194_reg_1000x_en
 *
 * Description: Access 1000BASE-X/SGMII register or Copper register
 * RDB_Reg 0x021 bit[0] = 1 (1000BASE-X/SGMII register space selected)
 * RDB_Reg 0x021 bit[0] = 0 (Copper register space selected)
 * Input: phy_addr
 *
 * Need to select Copper register space before bring eth up
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        enable   - 1: 1000BASE-X/SGMII register space; 0: Copper register space
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_reg_1000x_en (int phy_addr, int enable)
{   //wqc CHECK OK
    int      rdb_offset = 0;
    uint16_t reg_val    = 0;

    /* Select 1000BASE-T register space. */
    rdb_offset = 0x21;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    if (enable) {
        reg_val |= BCM54194_REG_1000X_EN_BIT;
    } else {
        reg_val &= ~(BCM54194_REG_1000X_EN_BIT);
    }

    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    return 0;
}

/*
 * Function: manhattan_bcm54194_per_port_reset
 *
 * Description: This reset will set the PHY registers listed below to the
 * default values and hardware strap balls that are labeled sample on reset(SOR)
 * are relatched.
 * IEEE Registers (0x00 to 0x0F)
 * Per-Port RDB Registers (RDB_Reg. 0x00 to offset 0x2FF)
 * Input: phy_addr - PHY mdio address
 *        intf - what side of interface to reset (0: SMII; 1: Copper; 2: Fiber)
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_per_port_reset (int phy_addr, int intf)
{
    ushort reg_val = 0;

    /* Switch to SGMII/Copper/Fiber register space */
    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, !!(intf == MANHATTAN_BCM54194_INTF_FIBER)),
            -(__LINE__), "Failed.\n");

    /* Reset PHY port */
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, BCM54194_CTRL_REG, &reg_val), -(__LINE__), "");
    reg_val |= BCM54194_RESET_BIT;
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_CTRL_REG,  reg_val), -(__LINE__), "");
    switzer_mdelay(5);
    return 0;
}

/*
 * Function: manhattan_bcm54194_global_rdb_reset
 *
 * Description: Global RDB Register reset will reset the Global RDB Registers
 * (RDB_Reg 0x800 to offset 0xAFF) to their default values.
 * This needs to be done to Port 0's PHY address.
 *
 * Input: none
 *
 * Return: PASSED/FAILED
 */
static int manhattan_bcm54194_global_rdb_reset (void)
{
    ushort reg_val = 0;
    int rdb_offset = BCM54194_TOP_MISC_TOP_GBL_RST_REG;
    int phy_addr   = MANHATTAN_BCM54194_PHYAD;

    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
    reg_val     |= BCM54194_TOP_MII_REG_SOFT_RST_BIT;
    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset,  reg_val), -(__LINE__), "");
    switzer_mdelay(5);

    return 0;
}

/*
 * Function: manhattan_bcm54194_clause45_reset
 *
 * Description: Clause 45 Register reset will reset the EEE block and registers
 * to their default values.
 * To enable the reset, set Clause 45 DEVAD 0x1, Address 0x0, bit[15] = 1'b0
 * This needs to be done to Port 0's PHY address.
 *
 * Input: none
 *
 * Return: none
 */
static int manhattan_bcm54194_clause45_reset (void)
{
    ushort mii_val = 0;

    ERET_COND(0 != MHT_MDIO_RD45(MANHATTAN_BCM54194_PHYAD, 0x1, 0, &mii_val), -(__LINE__), "");
    mii_val |= 1 << 15;
    ERET_COND(0 != MHT_MDIO_WR45(MANHATTAN_BCM54194_PHYAD, 0x1, 0,  mii_val), -(__LINE__), "");

    switzer_mdelay(5);

    return 0;
}

/*
 * Function: manhattan_bcm54194_soft_reset
 *
 * Description: This function reset all set of registers
 * The BCM54194 has multiple reset bits as follows:
 * 1. Per-Port Register Reset
 * 2. Global RDB Register Reset
 * 3. Clause 45 Register Reset
 *
 * No one use this function so far.
 *
 * Input: none
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_soft_reset (void)
{
    int phy_addr = 0;
    int port     = 0;
    int intf     = 0;

    for (port = 0; port < MANHATTAN_BCM54194_PORT_NUMB; port++) {
        phy_addr  = MANHATTAN_BCM54194_PHYAD_PORT(port);
        for (intf = MANHATTAN_BCM54194_INTF_SGMII; intf <= MANHATTAN_BCM54194_INTF_FIBER; intf++) {
            ERET_COND(0 != manhattan_bcm54194_per_port_reset(phy_addr, intf), -(__LINE__), "BCM54194 per port reset on port-%d failed.\n", port);
        }
    }

    ERET_COND(0 != manhattan_bcm54194_global_rdb_reset(), -(__LINE__), "Reset Global RDB register failed.\n");
    ERET_COND(0 != manhattan_bcm54194_clause45_reset(),   -(__LINE__), "Reset Clause45 register failed.\n");
    return 0;
}

/*
 * Function: manhattan_bcm54194_init_script
 *
 * Description:
 *
 * Input: none
 *
 * Return: none
 */
int manhattan_bcm54194_init_script (void)
{
    int rdb_reg      = 0;
    int port         = 0;
    int phy_addr     = 0;
    int phy_addr_up  = 0;
    uint16_t reg_val = 0;

    //TODO: disable those unused ports

    /* Workaround for MDIO address issue on BCM54194 B0 silicon.
     * Avoiding touch RDB_reg 0x234 to switch register space.
     *
     * SGMII/Copper:
     * GPHY BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     *
     * SGMII/Fiber:
     * Fiber SerDes BA+0 to BA+3, SGMII SerDes BA+4 to BA+7, MACsec BA+9
     */
    phy_addr = MANHATTAN_BCM54194_PHYAD;
    manhattan_bcm54194_rdb_access_disable(phy_addr);
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x17,  0x0D19  ), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, 0x15,  &reg_val), -(__LINE__), "");
    reg_val |= (0x1 << 3);
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x17,  0x0D19 ), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x15,  reg_val), -(__LINE__), "");
    manhattan_bcm54194_rdb_access_enable(phy_addr);

    /* Reset the PHY by writing to the PHY Reset register in the FPGA
     * The reset is required on A0 silicon, but not on B0 silicon
     * It appears to be harmless on B0, so go ahead and do it for all systems
     *
     * Manhattan: ported code, don't understand, no description about 0x17 in doc
     */
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x17, 0x0D19), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x15, 0x4189), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x17, 0x0D19), -(__LINE__), "");
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, 0x15, 0xC189), -(__LINE__), "");

    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD_SGMII(0), 0x21, 0xFC01), -(__LINE__), "");
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD_SGMII(1), 0x21, 0xFC01), -(__LINE__), "");
    //ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD_SGMII(2), 0x21, 0xFC01), -(__LINE__), "");
    //ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD_SGMII(3), 0x21, 0xFC01), -(__LINE__), "");

    for (port = 0; port < MANHATTAN_BCM54194_PORT_NUMB; port++)
    {
        phy_addr    = MANHATTAN_BCM54194_PHYAD_PORT(port);
        phy_addr_up = MANHATTAN_BCM54194_PHYAD_SGMII(port);
        ERET_COND(0 != MHT_RDB_WR(phy_addr_up, 0x21, 0xFC01), -(__LINE__), "");

        /* Disable SGMII AN */
        /* ERET_COND(0 != MHT_MDIO_WR(phy_addr_up, 0x0, 0x0140), -(__LINE__), ""); */

        /* manhattan_bcm54194_switch_intf_access (manhattan_bcm54194_intf_t intf) */

        /* Set RDB 0x22D.4 to 0 for SGMII auto-negotiation to function property. */
        rdb_reg = 0x22D;
        ERET_COND(0 != MHT_RDB_RD(phy_addr_up, rdb_reg, &reg_val), -(__LINE__), "");
        reg_val &= ~(0x1 << 4);
        ERET_COND(0 != MHT_RDB_WR(phy_addr_up, rdb_reg,  reg_val), -(__LINE__), "");

        /* Disable SUPER_ISOLATE bit.
         * To make 1GE PHY copper interface in normal operation. */
        rdb_reg = 0x2A;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_reg, &reg_val), -(__LINE__), "");
        reg_val &= ~(0x1 << 5);
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_reg,  reg_val), -(__LINE__), "");
    }
    //3.6 SFP Monitoring, SFP_TXFLT_RXLOS_EN,
    //     Configure LED_P[3:0]_2 pins as inputs for TX_FAULT[3:0] pins.
    //     Configure LED_P[3:0]_3 pins as inputs for RX_LOS[3:0] pins
    rdb_reg = 0x811;
    ERET_COND(0 != MHT_RDB_RD(MANHATTAN_BCM54194_PHYAD, rdb_reg, &reg_val), -(__LINE__), "");
    reg_val |= 3 << 14; //SFP_TXDIS_EN | SFP_TXFLT_RXLOS_EN
    ERET_COND(0 != MHT_RDB_WR(MANHATTAN_BCM54194_PHYAD, rdb_reg,  reg_val), -(__LINE__), "");

    return 0;
}

/*
 * Function: manhattan_bcm54194_reset
 *
 * Description: Reset/unreset BCM54194 1G PHY from FPGA, and
 *              Initialize all the BCM54194 1G PHY on Fugazi.
 *
 * Input: print_msg (1: Print Reset msg)
 *
 * Return: none
 */
int manhattan_bcm54194_reset (int print_msg)
{
    /* Reset BCM54194 GE PHY by FPGA bit_13 at FPGA version v0.9.1 */
    /* Reset BCM54194 GE PHY by FPGA bit_0 at latest FPGA version */
    if (print_msg) printf("Reset BCM54194 PHY...\n");
    if (_g_p_seahawks->reset) _g_p_seahawks->reset(_g_p_seahawks->priv, 1);
    switzer_mdelay(20);

    if (print_msg) printf("Unreset BCM54194 PHY...\n");
    if (_g_p_seahawks->reset) _g_p_seahawks->reset(_g_p_seahawks->priv, 0);
    switzer_mdelay(20);

    if (print_msg) printf("Perform soft reset...\n");
    manhattan_bcm54194_soft_reset();

    if (print_msg) printf("Initialize BCM54194 PHY...\n");
    /* Init PHY */
    manhattan_bcm54194_init_script();
    switzer_mdelay(100);

    return 0;
}

/*
 * Function: bcm54194_100base_fx_config
 *
 * Description: Config BCM54194 as 100BASE-FX mode.
 * Input: phy_addr
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_100base_fx_config(int phy_addr, int enable)
{
    uint16_t reg_val    = 0;
    int      rdb_offset = 0;
    int      regnum     = 0;

    if (enable) {
        /* Disable Copper/Fiber Auto-detection */
        rdb_offset   = BCM54194_AUTO_DETECT_MEDIUM_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     &= ~(BCM54194_AUTO_DET_MEDIUM_EN_BIT);
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable Fiber mode and access Copper register */
        rdb_offset   = BCM54194_MODE_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val     |= BCM54194_SGMII_TO_FIBER_MODE;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Power down the Copper interface */
        regnum       = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");
        reg_val     |= BCM54194_POWER_DOWN_BIT;
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, rdb_offset), -(__LINE__), "");

        /* Enable 100BASE-FX mode */
        rdb_offset   = BCM54194_SERDES_100FX_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     |= BCM54194_100BASE_FX_MODE_EN;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Select port */
        rdb_offset   = BCM54194_SGMII_LN_CTRL_1G_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     &= ~(BCM54194_SERFES_PORT_SEL_MASK);
        reg_val     |= MANHATTAN_BCM54194_PORT_BY_PHYAD(phy_addr) << 12;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Set rise/fall time to 142 ns */
        phy_addr     = MANHATTAN_BCM54194_PHYAD;
        rdb_offset   = BCM54194_SGMII_TX_ACTRL_2_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     &= ~(BCM54194_SERDES_RISE_FALL_MASK);
        reg_val     |= BCM54194_SERDES_RISE_FALL_142NS;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Set signal detect threshold = 100 mV */
        rdb_offset   = BCM54194_SGMII_RX_ACTRL_5_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val     &= ~(BCM54194_SD_THRESHOLD_MASK);
        reg_val     |= BCM54194_SD_THRESHOLD_100mV;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");
    } else {
        manhattan_bcm54194_reset(1);
    }

    return 0;
}
/*
 * Function: manhattan_bcm54194_sgmii_slave_mode
 *
 * Description: Config BCM54194 as SGMII-Slave mode.
 *              The mode is used when connecting to a
 *              SFP SGMII-to Copper Transceiver
 *              (10/100/1000BASE-T) module.
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        enable - 1: enable SGMII-Slave mode; 0 disable SGMII-Slave mode
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_sgmii_slave_mode (int phy_addr, int enable)
{
    uint16_t reg_val = 0;
    int      rdb_off = 0;
    int      reg_num = 0;

    if (enable) {
        /* Disable Copper/Fiber Auto-detection */
        rdb_off = BCM54194_AUTO_DETECT_MEDIUM_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_AUTO_DET_MEDIUM_EN_BIT);
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_off,  reg_val), -(__LINE__), "");

        /* Enable Fiber mode and access Copper register */
        rdb_off = BCM54194_MODE_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_off,  reg_val), -(__LINE__), "");

        /* Power down the Copper interface */
        reg_num = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        reg_val |= BCM54194_POWER_DOWN_BIT;
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, reg_num,  reg_val), -(__LINE__), "");

        /* Enable SGMII-Slave mode */
        rdb_off = BCM54194_SGMII_SLAVE_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val |= BCM54194_SGMII_SLAVE_MODE_EN_BIT;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_off,  reg_val), -(__LINE__), "");
    } else {
        /* Enable Copper/Fiber Auto-detection */
        rdb_off = BCM54194_AUTO_DETECT_MEDIUM_REG;
        ERET_COND(0 != MHT_RDB_RD( phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val |= BCM54194_AUTO_DET_MEDIUM_EN_BIT;
        ERET_COND(0 != MHT_RDB_WR( phy_addr, rdb_off,  reg_val), -(__LINE__), "");

        /* Enable Fiber mode and access Copper register */
        rdb_off = BCM54194_MODE_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD( phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_MODE_SEL_MASK | BCM54194_REG_1000X_EN_BIT);
        reg_val |= BCM54194_SGMII_TO_FIBER_MODE;
        ERET_COND(0 != MHT_RDB_WR( phy_addr, rdb_off,  reg_val), -(__LINE__), "");

        /* Power up the Copper interface */
        reg_num = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_POWER_DOWN_BIT);
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, reg_num,  reg_val), -(__LINE__), "");

        /* Disable SGMII-Slave mode */
        rdb_off = BCM54194_SGMII_SLAVE_REG;
        ERET_COND(0 != MHT_RDB_RD( phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_SGMII_SLAVE_MODE_EN_BIT);
        ERET_COND(0 != MHT_RDB_WR( phy_addr, rdb_off,  reg_val), -(__LINE__), "");
    }
    return 0;
}

/*
 * Function: dump_bcm54194_loopback_config
 *
 * Description: dump registers value according to loopback mode.
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        loopback_mode - 1: PHY internal; 2 with ext loopback plug
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_loopback_config_dump (int phy_addr, int loopback_mode)
{
    uint16_t reg_val = 0;
    int      rdb_off = 0;
    int      reg_num = 0;

    switch(loopback_mode) {
    case MANHATTAN_BCM54194_LPBK_INT:
        /* Enable loopback mode. */
        rdb_off = BCM54194_COPPER_AUXILIARY_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x RDB-0x%04x val-0x%04x\n", __func__, phy_addr, rdb_off, reg_val);

        /* Enable loopback mode without loopback plug. */
        rdb_off = 0x2C;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x RDB-0x%04x val-0x%04x\n", __func__, phy_addr, rdb_off, reg_val);

        reg_num = BCM54195_1000BASE_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x reg-0x%04x val-0x%04x\n", __func__, phy_addr, reg_num, reg_val);

        reg_num = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x reg-0x%04x val-0x%04x\n", __func__, phy_addr, reg_num, reg_val);

        rdb_off = BCM54194_AUTO_DETECT_MEDIUM_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x RDB-0x%04x val-0x%04x\n", __func__, phy_addr, rdb_off, reg_val);
        break;

    case MANHATTAN_BCM54194_LPBK_EXT:
        /* Enable loopback mode with loopback plug. */
        rdb_off = BCM54194_COPPER_AUXILIARY_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x RDB-0x%04x val-0x%04x\n", __func__, phy_addr, rdb_off, reg_val);

        reg_num = BCM54195_1000BASE_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x reg-0x%04x val-0x%04x\n", __func__, phy_addr, reg_num, reg_val);

        reg_num = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, reg_num, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x reg-0x%04x val-0x%04x\n", __func__, phy_addr, reg_num, reg_val);

        rdb_off = BCM54194_AUTO_DETECT_MEDIUM_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_off, &reg_val), -(__LINE__), "");
        printf("%-36s phy-addr-0x%02x RDB-0x%04x val-0x%04x\n", __func__, phy_addr, rdb_off, reg_val);
        break;
    }
    return 0;
}

/*
 * Function: bcm54194_cfg_setting
 *
 * Description: BCM54194 control register setting.
 * Input: phy_addr, speed, duplex, interface
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        speed    - Interface speed (100MBPS Only for Fugazi)
 *        auto_neg - 1: enable auto-nego; 0: disable auto-nego
 *        duplex   - 1: full duplex; 0: half duplex
 *        intf     - what interface (0: SMII; 1: Copper; 2: Fiber)
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_cfg_setting (int phy_addr, int speed, int auto_neg, int duplex, manhattan_bcm54194_intf_t intf)
{//wqc CHECK OK
    ushort reg_val = 0;

    /* Switch to SGMII/Copper/Fiber register space */
    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, (intf == MANHATTAN_BCM54194_INTF_FIBER)),
            -(__LINE__), "Failed.\n");

    ERET_COND(0 != MHT_MDIO_RD(phy_addr, BCM54194_CTRL_REG, &reg_val), -(__LINE__), "");

    reg_val &= ~(BCM54194_SPEED_MASK);
    switch(speed) {
    case SPD_1000MBPS : reg_val |= BCM54194_SPEED_1000MBPS; break;
    case SPD_100MBPS  : reg_val |= BCM54194_SPEED_100MBPS ; break;
    case SPD_10MBPS   : reg_val |= BCM54194_SPEED_10MBPS  ; break;
    default           : reg_val |= BCM54194_SPEED_1000MBPS; break;
    }

    if (duplex) {
        reg_val |= BCM54194_DUPLEX_BIT;
    } else {
        reg_val &= ~(BCM54194_DUPLEX_BIT);
    }

    if (auto_neg) {
        reg_val |= BCM54194_AN_ENABLE;
    } else {
        reg_val &= ~(BCM54194_AN_ENABLE);
    }

    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54194_CTRL_REG, reg_val), -(__LINE__), "");

    return 0;
}

/*
 * Function: bcm54194_config_loopback
 *
 * Description: Configurating BCM54194 in loopback mode.
 * Input: phy_addr, speed, interface, loopback mode, enable
 *
 * Input:
 *        phy_addr      - PHY mdio address
 *        speed         - Interface speed (100MBPS Only for Fugazi)
 *        intf          - loop back at what interface (0: SMII; 1: Copper; 2: Fiber)
 *        loopback_mode - 1: PHY internal; 2: with ext loopback plug
 *        enable        - 1: enable; 0: disable
 *
 * Return: TODO
 */
int manhattan_bcm54194_config_loopback (int phy_addr, int speed, int loopback_mode, int enable)
{
    uint16_t reg_val    = 0;
    int      rdb_offset = 0;
    int      regnum     = 0;
    int      duplex     = FULL_DUPLEX;

    /* Switch to SGMII/Copper/Fiber register space */
    //bcm54194_switch_intf_access(intf);

    /* TODO: need to check link status of Fiber register space */

    switch(loopback_mode) {
    case MANHATTAN_BCM54194_LPBK_SGMII:
        if (enable) {
            ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 1, MANHATTAN_BCM54194_INTF_FIBER),
                    -(__LINE__), "Failed.\n");
        }

        /* Select Fiber/SGMII register space. */
        ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, enable), -(__LINE__), "Failed.\n");

        /* Force link when in 10Mbps or 100Mbps mode. Not needed for 1000Mbps mode. */
        if (speed != SPD_1000MBPS) {
            rdb_offset = BCM54194_TEST_1_REG;
            ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
            if (enable) {
                reg_val |= BCM54194_FORCE_LINK_BIT;
            } else {
                reg_val &= ~BCM54194_FORCE_LINK_BIT;
            }
            ERET_COND(0 != MHT_MDIO_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");
        }

        /* Enable SGMII internal loopback */
        regnum = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");
        if (enable) {
            reg_val |= BCM54194_INTERNAL_LOOPBACK;
        } else {
            reg_val &= ~(BCM54194_INTERNAL_LOOPBACK);
        }
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, reg_val), -(__LINE__), "");

        ERET_COND(0 != manhattan_bcm54194_cfg_setting(phy_addr, speed, AUTONEG_ON, duplex, MANHATTAN_BCM54194_INTF_FIBER),
                -(__LINE__), "Failed to config PHY setting. phy addr:0x%x\n", phy_addr);
        break;

    case MANHATTAN_BCM54194_LPBK_INT:
        /* Enable loopback mode. */
        rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
        reg_val    = enable ? 0x8400 : 0x430;

        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable loopback mode without loopback plug. */
        rdb_offset = 0x2C;
        reg_val    = enable ? 0x4014 : 0x4004;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable 1000BASE-T Master mode */
        if (speed == SPD_1000MBPS) {
            regnum = BCM54195_1000BASE_CTRL_REG;
            reg_val= enable ? 0x1800 : 0xF00;
            ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, reg_val), -(__LINE__), "");
        }

        /* Power Down Copper Interface.
         * When power up copper interface,
         * the COPPER_AN_ENABLE bit[12] will reset to 1
         */
        if (enable) {
            manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 0, MANHATTAN_BCM54194_INTF_COPPER);
            switzer_mdelay(10);
            manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 1, MANHATTAN_BCM54194_INTF_COPPER);
        }

        ERET_COND(0 != manhattan_bcm54194_cfg_setting(phy_addr, speed, AUTONEG_OFF, duplex, MANHATTAN_BCM54194_INTF_COPPER),
                -(__LINE__), "Failed.\n");

        /* To exit the Copper Loopback without Loopback Plug,
         * Broadcom recommends a software or hardware reset. */
        if (!enable) {
            manhattan_bcm54194_reset(1);
        }
        break;

    case MANHATTAN_BCM54194_LPBK_EXT:
        /* Enable loopback mode with loopback plug. */
        rdb_offset = BCM54194_COPPER_AUXILIARY_CTRL_REG;
        reg_val    = enable ? 0x8400 : 0x430;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable 1000BASE-T Master mode */
        if (speed == SPD_1000MBPS) {
            regnum = BCM54195_1000BASE_CTRL_REG;
            reg_val= enable ? 0x1800 : 0xF00;
            ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, reg_val), -(__LINE__), "");
        }

        /* Power Down Copper Interface.
         * When power up copper interface,
         * the COPPER_AN_ENABLE bit[12] will reset to 1
         */
        if (enable) {
            manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 0, MANHATTAN_BCM54194_INTF_COPPER);
            switzer_mdelay(10);
            manhattan_bcm54194_sig_pwr_ctrl(phy_addr, 1, MANHATTAN_BCM54194_INTF_COPPER);
        }

        ERET_COND(0 != manhattan_bcm54194_cfg_setting(phy_addr, speed, AUTONEG_OFF, duplex, MANHATTAN_BCM54194_INTF_COPPER),
                -(__LINE__), "Failed.\n");

        /* To exit the Copper Loopback with Loopback Plug,
         * Broadcom recommends a software or hardware reset. */
        if (!enable) {
            manhattan_bcm54194_reset(1);
        }

        break;

    case MANHATTAN_BCM54194_LPBK_SFP_EXT:
        if (speed == SPD_100MBPS) {
            if (0) { //TODO:When slave mode
                manhattan_bcm54194_sgmii_slave_mode(phy_addr, enable);
            } else {
                manhattan_bcm54194_100base_fx_config(phy_addr, enable);
            }
        } else {
            /* From BCM FAE:
             * No phy configuration is needed for fiber SFP loopback with a plug.
             * Just plug fiber tx => rx loopback plug in SFP.
             */
            if (enable) {
                /* Select 1000BASE-T register space and set speed
                 * to make SFP link up.
                 */
                ERET_COND(0 != manhattan_bcm54194_cfg_setting(phy_addr, speed, AUTONEG_ON, duplex, MANHATTAN_BCM54194_INTF_FIBER),
                        -(__LINE__), "Failed.\n");
            } else {
                /* Restore to Copper register space. */
                ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, enable), -(__LINE__), "Failed.\n");
            }
        }

        break;

    default:
        /* Clear the loopback */
        printf("BCM INFO - None loopback\n");

        break;
    }

    return 0;
}

/**********************************************************************
 *
 * Function: bcm54194_is_linkup
 *
 * Description:
 * Check if the BCM54194 SGMII/Copper/Fiber interface link status is up
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        intf - what interface (0: SMII; 1: Copper; 2: Fiber)
 *
 * Return:  > 0     - Link is up
 *          = 0     - Link is not up
 *          < 0     - Error encountered
 */
int manhattan_bcm54194_is_linkup (int phy_addr, manhattan_bcm54194_intf_t intf)
{
    ushort    reg_val   = 0;
    const int wait_max  = 10000; //ms
    const int wait_unit = 100;   //ms
    int       waited    = 0;

    /* Switch to SGMII/Copper/Fiber register space */
    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, (intf == MANHATTAN_BCM54194_INTF_FIBER)), //TODO: ||MANHATTAN_BCM54194_INTF_SGMII
            -(__LINE__), "Failed.\n");

    /* max timeout waiting for Link up 100 sec */
    printf("\nWait for PHY link up");
    for(waited = 0; waited < wait_max; waited += wait_unit) {
        if (waited % 5000 == 0)
            printf("\n");
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, BCM54194_STAT_REG, &reg_val), -(__LINE__), "");
        if (reg_val & BCM54194_LINK_STAT_BIT) {
            printf("OK\n");
            return 1; /* True */
        }
        printf(".");
        fflush(stdout);
        switzer_mdelay(wait_unit);
    }
    printf("\n");
    return 0; /* false */
}

/*
 * Function: bcm54194_sig_pwr_ctrl
 *
 * Description: Configurate BCM54194 in Power Down mode.
 * Input: phy_addr, speed, interface, loopback mode, enable
 *
 * Input:
 *        phy_addr - PHY mdio address
 *        enable   - 1: in Normal operation; 0: in low-power standby mode
 *        intf     - at what interface (0: SMII; 1: Copper; 2: Fiber
 *
 * Return: == 0     - Success
 *         != 0     - Error encountered
 */
int manhattan_bcm54194_sig_pwr_ctrl(int phy_addr, int enable, manhattan_bcm54194_intf_t intf)
{//wqc CHECK OK
    int regnum = BCM54194_CTRL_REG;
    ushort reg_val;

    /* Switch to SGMII/Copper/Fiber register space */
    ERET_COND(0 != manhattan_bcm54194_reg_1000x_en(phy_addr, (intf == MANHATTAN_BCM54194_INTF_FIBER)),
            -(__LINE__), "Failed.\n");

    ERET_COND(0 != MHT_MDIO_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");

    if (enable) {
        /* configure PHY in normal operation */
        reg_val &= ~(BCM54194_POWER_DOWN_BIT);
    } else {
        /* configure PHY in lowe-power standby mode */
        reg_val |= BCM54194_POWER_DOWN_BIT;
    }

    ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, reg_val), -(__LINE__), "");

    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: switch_fiber
 *   Switch between copper and fiber.
 *   ensure the copper is truned off before fiber test.
 *   turn the copper after the test is finished.
 *
 * Input:
 *         onoff - turn on/off advertise reg.
 *         phyad - phy addr
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int manhattan_bcm54194_switch_to_fiber(int phyad, int onoff)
{
    if (onoff) {
        /*enable fiber, disable copper */
        ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phyad, 0, MANHATTAN_BCM54194_INTF_COPPER),
            -(__LINE__), "Phy-addr-0x%02x: disable copper failed", phyad);
        ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phyad, 1, MANHATTAN_BCM54194_INTF_FIBER),
            -(__LINE__), "Phy-addr-0x%02x: enable fiber failed", phyad);
    } else {
        /*enable copper, disable fiber */
        ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phyad, 0, MANHATTAN_BCM54194_INTF_FIBER),
            -(__LINE__), "Phy-addr-0x%02x: disable fiber failed", phyad);
        ERET_COND(0 != manhattan_bcm54194_sig_pwr_ctrl(phyad, 1, MANHATTAN_BCM54194_INTF_COPPER),
            -(__LINE__), "Phy-addr-0x%02x: enable copper failed", phyad);
    }

    return 0;
}

/*
 * Function: check_link
 *
 * Description:
 * Dump the status of system and network side of each port.
 *
 * Input: None
 *
 * Return: None
 */
int manhattan_bcm54194_link_status (void)
{
     ushort  rdb_rdval  = 0;
     ushort  ieee_rdval = 0;
     int     phy_addr   = 0;
     int     port       = 0;

    printf("\n- Link Status -");
    printf("\n------------------------------------------------------\n");
    for (port = 0; port < MANHATTAN_BCM54194_PORT_NUMB; port++) {
        phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);
        ERET_COND(0 != MHT_RDB_RD(phy_addr, 0x21, &rdb_rdval), -(__LINE__), "");
        ERET_COND(0 != MHT_RDB_RD(phy_addr, 0x21, &rdb_rdval), -(__LINE__), "");
        printf("BCM54194 port-%d %-16s: %4s (RDB  reg-0x21 = 0x%04x (%s))\n",
                port, "line-side-fiber", rdb_rdval & 0x40 ? "UP" : "DOWN", rdb_rdval, __binary_dump_16(rdb_rdval, NULL));
        printf("BCM54194 port-%d %-16s: %4s (RDB  reg-0x21 = 0x%04x (%s))\n",
                port, "line-side-copper", rdb_rdval & 0x80 ? "UP" : "DOWN", rdb_rdval, __binary_dump_16(rdb_rdval, NULL));

        phy_addr = MANHATTAN_BCM54194_PHYAD_SGMII(port);
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, 1, &ieee_rdval), -(__LINE__), "");
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, 1, &ieee_rdval), -(__LINE__), "");
        printf("BCM54194 port-%d %-16s: %4s (IEEE reg-0x1  = 0x%04x (%s))\n",
                port, "system-side", ieee_rdval & 0x4 ? "UP" : "DOWN", ieee_rdval, __binary_dump_16(ieee_rdval, NULL));
    }
    return 0;
}


/*
 * Function: set_line_side_config
 *
 * Description: Enable 1000BASE-x Line-Side Loopback
 * RDB_Reg 0x023E        = 0x78E0 (Disable Copper/Fiber Auto Switching)
 * RDB_Reg 0x021         = 0x7C30 (Enable 1000BASE-X mode and 1000BASE-X Register space)
 * RDB_Reg 0x02C bit[15] = 1'b1   (1000BASE-X Line-Side Loopback Enable)
 * IEEE_ Reg 0x0 bit[9]  = 1'b1   (Restart 1000BASE-X autonegotiation)
 *
 * Input:
 *        port - BCM54194 port
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_line_side_config (int port)
{
     uint16_t  reg_val    = 0;
     uint16_t  ieee_rdval = 0;
     int       rdb_offset = 0;
     int       phy_addr   = 0;

    phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);

    /*1, Disable Copper/Fiber Auto Switching */
    rdb_offset = 0x23E  ;
    reg_val    = 0x78E0 ;
    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    /*2, Enable 1000BASE-X mode and 1000BASE-X Register space */
    rdb_offset = 0x21;
    reg_val    = 0x7C30;
    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    /*3, 1000BASE-X Line-Side Loopback Enable */
    rdb_offset = 0x2C;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    reg_val |= 1<<15;
    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    /*4, Restart 1000BASE-X autonegotiation */
    rdb_offset = 0x0;
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, rdb_offset, &ieee_rdval), -(__LINE__), "");

    ieee_rdval |= 1<<9;
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, rdb_offset, ieee_rdval), -(__LINE__), "");

    return 0;
}


/*
 * Function: bcm54194_config_prbs
 *
 * Description: Config BCM54194 PRBS from utility.
 *
 * Inputs      :
 *      action  - Check:0(default), Enable:1, Disable:2, Inject errors:3, Clear:4.
 *      invert  - 1(yes) or no(0), used when 'action == enable'.
 *      pattern - PRBS pattern: 7, 15 or 23, used when 'action == enable'.
 * Outputs     : PASSED / FAILED
 */
int manhattan_bcm54194_config_prbs (int port, int action, int invert, int pattern)
{
    ushort wr_val     = 0;
    ushort reg_val    = 0;
    int    phy_addr   = 0;
    int    rdb_offset = 0;

    phy_addr = MANHATTAN_BCM54194_PHYAD_PORT(port);

    switch (action) {
    default:
    case 0:
        rdb_offset = 0x201;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

        printf("\nPRBS %slocked"      , reg_val & BCM54194_PRBS_LOCKED    ? "" : "not "    );
        printf("\nPRBS %slost locked" , reg_val & BCM54194_PRBS_LOST_LOCK ? "" : "hasn't " );
        printf("\nPRBS errors: %d\n"  , reg_val & (~0xf800));
        break;

    case 1:
        rdb_offset = 0x200;
        /* Enable PRBS */
        wr_val = BCM54194_PRBS_ENABLE;
        wr_val &= ~(BCM54194_CLR_PRBS_ERR_CNTR);

        /* PRBS Config */
        if (invert) {
            wr_val |= BCM54194_PRBS_INVERT;
        } else {
            wr_val &= ~(BCM54194_PRBS_INVERT);
        }

        switch (pattern) {
        case PRBS_PATTERN_7  : wr_val |= BCM54194_PRBS_7 ; break;
        case PRBS_PATTERN_15 : wr_val |= BCM54194_PRBS_15; break;
        case PRBS_PATTERN_23 : wr_val |= BCM54194_PRBS_23; break;
        default              : wr_val |= BCM54194_PRBS_23; break;
        }

        /* Clear PRBS error counter first */
        wr_val |= BCM54194_CLR_PRBS_ERR_CNTR;
        wr_val |= BCM54194_PRBS_ENABLE;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, wr_val), -(__LINE__), "");

        wr_val &= ~(BCM54194_CLR_PRBS_ERR_CNTR);
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, wr_val), -(__LINE__), "");

        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        break;

    case 2:
        rdb_offset = 0x200;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        reg_val &= ~(BCM54194_PRBS_ENABLE);
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");
        break;

    case 3:
        /* to inject error, user must do Enable PRBS first */
        rdb_offset = 0x200;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

        /* Enable PRBS21 and Star Inject error */
        reg_val |= BCM54194_TEST_PRBS_ERR_CNTR;
        reg_val |= BCM54194_PRBS_ENABLE;

        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable PRBS21 and Stop Inject error */
        reg_val &= ~(BCM54194_TEST_PRBS_ERR_CNTR);

        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* read error counter */
        rdb_offset = 0x201;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
        printf("\nPRBS %slocked"      , reg_val & BCM54194_PRBS_LOCKED    ? "" : "not "    );
        printf("\nPRBS %slost locked" , reg_val & BCM54194_PRBS_LOST_LOCK ? "" : "hasn't " );
        printf("\nPRBS errors: %d\n"  , reg_val & (~0xf800));
        break;
    case 4:
        /* clear error counter, user must do Enable PRBS first */
        rdb_offset = 0x200;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

        /* Clear PRBS error counter */
        reg_val |= BCM54194_CLR_PRBS_ERR_CNTR;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* back to Normal operation */
        reg_val &= ~BCM54194_CLR_PRBS_ERR_CNTR;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        break;

    }

    return 0;
}


/*
 * Function: packet_counter_util
 *
 * Description:
 * Utility to show or modify packet counter of RDB register.
 *
 * Input:
 *      action - 0: Enable RX PKT counter
 *               1: Enable TX PKT Counter
 *               2: Show PKT Counter
 * Return: None
 */
int manhattan_bcm54194_packet_counter_util (int port, int action)
{
    ushort  rdb_up_rdval   = 0;
    ushort  rdb_down_rdval = 0;
    int     rdb_offset     = 0;
    int     phy_addr_up    = 0;
    int     phy_addr_down  = 0;

    /* Set up phy address*/
    phy_addr_up   = MANHATTAN_BCM54194_PHYAD_PORT(port);
    phy_addr_down = MANHATTAN_BCM54194_PHYAD_SGMII(port);

    manhattan_bcm54194_rdb_access_enable(phy_addr_up);
    manhattan_bcm54194_rdb_access_enable(phy_addr_down);

    rdb_offset = 0x2F;
    MHT_RDB_RD(phy_addr_down, rdb_offset, &rdb_down_rdval);
    MHT_RDB_RD(phy_addr_up  , rdb_offset, &rdb_up_rdval);

    if (action == 0) {
        printf("\nRX packet Counter Enable at Port-%d", port);
        rdb_up_rdval |= 0x800;
        MHT_RDB_WR(phy_addr_up, rdb_offset, rdb_up_rdval);

        rdb_down_rdval |= 0x800;
        MHT_RDB_WR(phy_addr_down, rdb_offset, rdb_down_rdval);
    }
    else if (action == 1) {
        printf("\nTX packet Counter Enable at Port-%d", port);
        rdb_up_rdval &= ~0x800;
        MHT_RDB_WR(phy_addr_up, rdb_offset, rdb_up_rdval);

        rdb_down_rdval &= ~0x800;
        MHT_RDB_WR(phy_addr_down, rdb_offset, rdb_down_rdval);
    }
    else {
        uint16_t rdb_reg[] = {
            0x1, 0x4, 0x2f, 0x30
        };
        int i = 0;

        printf("\nPacket Count of Port-%d : ", port);
        printf("\n------------------------------------------------------");
        printf("\nNetwork - downstream port");
        for(i = 0; i < sizeof(rdb_reg) / sizeof(uint16_t); i++) {
            MHT_RDB_RD(phy_addr_down, rdb_reg[i], &rdb_down_rdval);
            printf("\nRDB reg 0x%04x = 0x%04x", rdb_reg[i], rdb_down_rdval);
        }

        printf("\nSystem - upstream port");
        for(i = 0; i < sizeof(rdb_reg) / sizeof(uint16_t); i++) {
            MHT_RDB_RD(phy_addr_up, rdb_reg[i], &rdb_up_rdval);
            printf("\nRDB reg 0x%04x = 0x%04x", rdb_reg[i], rdb_up_rdval);
        }
    }
    printf("\n\n");
    return 0;
}


/*
 * Function: bcm54194_config_interrupt
 *
 * Description:
 *     configure the phy Code interrupts on the INTRP.
 * Input:
 *   port   - PHY port number, two ports per PHY in Fugazi (0, 1)
 *   enable - 1: enable; 0: disable
 * Note:
 *   Since RDB reg 0x82D is in Global Register space and needs to be accessed
 *   using first port PHY address.
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_config_interrupt (int port, int enable)
{
     int       phy_addr      = 0      ;
     int       rdb_offset    = 0      ;
     uint16_t  reg_val       = 0      ;
     uint16_t  port_int_mask = 0x00   ;


    /* configure the phy Code interrupts on the INTRP base on PHY port */
    port_int_mask |= (1 << (4 + port));

    phy_addr = MANHATTAN_BCM54194_PHYAD; /* Use the base addr */

    rdb_offset = 0x82D;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    /* disable all the Ports interrupt first */
    reg_val |= 0x00F0;
    if (enable) {
        /* interrupt output enabled on INTRP ball */
        reg_val &= ~port_int_mask;
    }
    else {
        /* interrupt output disable on INTRP ball */
        reg_val |= port_int_mask;
    }

    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    return 0;
}


/*
 * Function: bcm54194_interrupt_generate
 *
 * Description:
 *     Configure the phy loopback to make link change condition to
 *     generate link change interrupt.
 * Input:
 *   phy_addr - PHY MDIO address
 *   phy_num  - 1G PHY number in Fugazi (2,3,4,5)
 *   enable_flag - 1: enable; 0: disable
 * Note:
 *   Since RDB reg 0x82D is in Global Register space and needs to be accessed
 *   using first port PHY address.
 *
 * Return: PASSED/FAILED
 */

int manhattan_bcm54194_interrupt_generate (int phy_addr, int enable, int ext_lpbk)
{
    int      rdb_offset = 0;
  //int      regnum     = 0;
    uint16_t reg_val    = 0;
  //int      idx        = 0;

    printf("External loopback is %senabled.\n", ext_lpbk ? "" : "not ");

    if (!ext_lpbk) {
        /* Ext. loopback diag flag is off. config host local loopback */
        #if 0
        rdb_offset = 0x21;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

        if (enable)
            reg_val |= 0x1;
        else
            reg_val &= ~0x1;

        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        regnum = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");
        printf("%-40s phy addr:0x%02x, reg offset:0x%04x, value=0x%04x\n",
                __func__, phy_addr, regnum, reg_val);

        reg_val = (0x4140);
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, reg_val), -(__LINE__), "");
        printf("%-40s phy addr:0x%02x, reg offset:0x%04x, value=0x%04x\n",
                __func__, phy_addr, regnum, reg_val);
        #elif 0

        //wqc Write RDB Register 0x028 = 0x8400 Enable 1000BASE-T Loopback mode.
        //wqc Write RDB Register 0x02C = 0x0410 Enable 1000BASE-T Loopback mode without loopback plug.
        //wqc Write Register     0x09  = 0x1800 Enable 1000BASE-T Master mode.
        //wqc Write Register     0x00  = 0x0800 Power down copper interface.
        //wqc Write Register     0x00  = 0x0040 Enable Force 1000BASE-T.
        struct {
            int      type    ; //0-reg, 1-rdb, -1-invalid
            uint16_t reg_rdb ;
            uint16_t value   ;
        } configs [] = {
            {1  , 0x028 , 0x8400},
            {1  , 0x02C , 0x0410},
            {0  , 0x09  , 0x1800},
            {0  , 0x00  , 0x0800},
            {0  , 0x00  , 0x0040},
            {-1 , 0     , 0     },
        };
        if (enable) {
            for(idx = 0; configs[idx].type >= 0; idx++) {
                if (configs[idx].type == 0) {
                    ERET_COND(0 != MHT_MDIO_WR(phy_addr, configs[idx].reg_rdb, configs[idx].value), -(__LINE__), "");
                } else {
                    ERET_COND(0 != MHT_RDB_WR(phy_addr, configs[idx].reg_rdb, configs[idx].value), -(__LINE__), "");
                }
            }
        } else {
            manhattan_bcm54194_reset(0);
        }
        #else
        ERET_COND(0 != manhattan_bcm54194_config_loopback(phy_addr, BCM54194_SPEED_1000MBPS, MANHATTAN_BCM54194_LPBK_INT, 1),
                -(__LINE__), "Failed.\n");
        #endif
    } /* << Ext. loopback diag flag is off. config host local loopback */
    else {
        /* Ext. loopback diag flag is on. config line loopback */
        /* Disable Copper/Fiber Auto Switching */
        rdb_offset = 0x23E;
        reg_val = 0x78E0;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* Enable 1000BASE-X mode and 1000BASE-X Register spac */
        rdb_offset = 0x21;
        reg_val = 0x7C30;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

        /* 1000BASE-X Line-Side Loopback Enable */
        rdb_offset = 0x2C;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

        if (enable)
            reg_val |= 1<<15;
        else
            reg_val &= ~(1<<15);

        ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");
    }  /* Ext. loopback diag flag is on. config line loopback */

    return 0;
}

/*
 * Function: bcm54194_interrupt_set
 *
 * Description:
 *   to Disable/Enable/clear BCM54194 Link change Interrupt .
 *
 * Input:
 *   phy_addr - PHY address
 *   phy_num  - PHY number in Fugazi (2,3,4,5)
 *   action   - 0: disable, 1: enable, 2: clear link change interrupt
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_interrupt_set(int phy_addr, int action)
{
    int      rdb_offset           = 0     ;
    uint16_t reg_val              = 0     ;
    uint16_t link_change_int_mask = 0x0040;

    rdb_offset = 0x032;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    if (action == 1) {
        reg_val &= ~link_change_int_mask;/* Enable BCM54194 Link change Interrupt */
    }
    else {
        reg_val |= link_change_int_mask; /* Disable BCM54194 Link change Interrupt */
    }

    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");


    rdb_offset = 0xB;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    if (action == 1) {
        reg_val &= ~(1 << 1);/* Unmask BCM54194 LINK_STAT_CHANGE */
    }
    else {
        reg_val |=  (1 << 1); /* Mask  BCM54194 LINK_STAT_CHANGE */
    }

    ERET_COND(0 != MHT_RDB_WR(phy_addr, rdb_offset, reg_val), -(__LINE__), "");

    return 0;
}

/*
 * Function: bcm54194_interrupt_clear
 *
 * Description:
 *   to Disable/Enable/clear BCM54194 Link change Interrupt .
 *
 * Input:
 *   phy_addr - PHY address
 *
 * Return: PASSED/FAILED
 */
int manhattan_bcm54194_interrupt_clear (int phy_addr)
{
    int      rdb_offset = 0;
    uint16_t reg_val    = 0;

    rdb_offset = 0x031;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    rdb_offset = 0xA;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");

    return 0;
}

/*
 * Function: bcm54194_interrupt_get
 *
 * Description:
 *   to get which port that generated the interrupt.
 *
 * Input:
 *   phy_addr - PHY address
 *
 *  Output:
 *   int_status - interrupt status info
 *
 * Return: PASSED/FAILED
  */
int manhattan_bcm54194_interrupt_get (int phy_addr, uint16_t *int_status)
{
    int      rdb_offset = 0;
    uint16_t reg_val    = 0;

    rdb_offset = 0x03B;
    ERET_COND(0 != MHT_RDB_RD(phy_addr, rdb_offset, &reg_val), -(__LINE__), "");
    *int_status = reg_val;
    return 0;
}


/*
 * Function: bcm54194_mspu_bcast_write
 *
 * Description: BCM54194 MSPU register broadcast write.
 * The broadcast feature allows writing to all MSPU ports/slices at the the same time
 * and can be used for the write command only.
 * Write access to a register in all eight slices of the MSPU can be achieved by doing
 * a single broadcast write.
 * Input
 *
 * Return: none
 */
int manhattan_bcm54194_mspu_write(int mspu_sector, int mspu_reg, uint32_t reg_val)
{
    int            regnum     = 0x0;
    int            mspu_phyad = MANHATTAN_BCM54194_PHYAD + 9;
    unsigned short wrval_lsb  = 0;
    unsigned short wrval_msb  = 0;

    wrval_lsb = (reg_val & 0xFFFF);
    wrval_msb = (reg_val >> 16);
    /* Write the RDB register */
    regnum = 0x1B;
    ERET_COND(0 != MHT_MDIO_WR(mspu_phyad, regnum, mspu_sector), -(__LINE__), "");

    regnum = 0x18;
    ERET_COND(0 != MHT_MDIO_WR(mspu_phyad, regnum, mspu_reg), -(__LINE__), "");

    regnum = 0x19;
    ERET_COND(0 != MHT_MDIO_WR(mspu_phyad, regnum, wrval_lsb), -(__LINE__), "");

    regnum = 0x1A;
    ERET_COND(0 != MHT_MDIO_WR(mspu_phyad, regnum, wrval_msb), -(__LINE__), "");

    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: init_bcm54194_macsec
 *
 * Description:
 * This is a sample script for Encryption and decryption on packets.
 * The TCAM Secure Association table is setup ony to look at MAC SA =
 * BBBBBBBBBBBBh for decision making;
 * For encryption send plaintext packets with enough IPG to support
 * 32byte MACsec overhead addition
 * Input:  phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
int manhattan_bcm54194_macsec_init(uint phy_addr)
{
    int mspu_sector = 0x00;

    /******************************
     * BCM54194 top configuration
     ******************************/

    /* Configure datapath select to bring 1588 close to Line
     * and enabling MSPU (MacSec) path */
    MHT_MDIO_WR45(phy_addr, 7, 0x984D, 0x40FF);
    ERET_COND(0 != MHT_RDB_WR(phy_addr, 0x084A, 0x0), -(__LINE__), "");

    /* Power up all MSPU's by enabling clock */
    ERET_COND(0 != MHT_RDB_WR(phy_addr, 0x084B, 0x8000), -(__LINE__), "");

    /* Enable MSPU Switch MAC to take SW side speed setting */
    MHT_MDIO_WR45(phy_addr, 7, 0x9870, 0x00FF);

    /* Set MSPU SW side speed setting to 1G */
    MHT_MDIO_WR45(phy_addr, 7, 0x9871, 0xAAAA);

    /******************************
     * MSPU configuration: UniMAC
     ******************************/
    /* Setting Line side and Switch side UniMAC
     * Enable RX and TX, ETH_SPEED = 1G,
     * enable RX and TX SW programmed pause capability*/
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7808, 0x0146009B);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7C08, 0x0146009B);

    /* Setting TX IPG length to be 8 on Line side */
    manhattan_bcm54194_mspu_write(mspu_sector, 0x785C, 0x00000008);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7C5C, 0x00000008);

    /* Enable TX CRC corruption when system signals
     * corrupt CRC on Line and Switch side */
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7B14, 0x00000002);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7F14, 0x00000002);

    /* Setting Frame length to support Jumbo packets */
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7814, 0x00003FFF);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7C14, 0x00003FFF);

    /* Disable MAC statistics clear on read */
    //printf("mfix: script typo? %d\n", __LINE__);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0000, 0x00000002);

    /**************************************
     * MSPU configuration: EIP165 generic
     **************************************/
    /* SAF/cut-through mode egress
     * enabling SAF mode */
    /* 0xAAAA ---> sector 3 (egress, ===> 0x2FE00) */
    mspu_sector = 0xAAAA;
    //printf("mfix: script typo? %d\n", __LINE__);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xFE00, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xFE04, 0x06000400);

    /* SAF/cut-through mode ingress
     * enabling SAF mode */
    /* 0x5555 ---> sector 2 (ingress, ===> 0x1FE00) */
    mspu_sector = 0x5555;
    manhattan_bcm54194_mspu_write(mspu_sector, 0xFE00, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xFE04, 0x06000400);

    /**********************************************
     * MSPU configuration:
     * Encryption and Decryption configuration
     **********************************************/
    /* Encryption configuration */
    // EIP165.IgEIP160.OutPostProc: CC_Configure
    //printf("mfix: script typo(mspu_sector is 0x08 or 0x00)? %d\n", __LINE__);
    mspu_sector = 0x5555;
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE844, 0x05FF0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE840, 0x0000C000);

    // EIP165.IgEIP160.StatTCAM: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000C);

    // EIP165.IgEIP160.StatRXCAM: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0x5410, 0x0000000C);

    // EIP165.IgEIP160.StatSA: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE210, 0x0000000C);

    // EIP165.IgEIP160.StatSECY: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE410, 0x0000000C);

    // EIP165.IgEIP160.StatIFC: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE610, 0x0000000C);

    // Program PE transform err detection
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF124, 0x80FE0000);

    // EIP165.IgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    manhattan_bcm54194_mspu_write(mspu_sector, 0x797C, 0x02000000);

    // Program packet engine to fetch only
    // 5 x 128-bit words of the context
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880214);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.IgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880614);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // EIP165.EgEIP160: DefaultMode
    // EIP165.EgEIP160.StatTCAM: DefaultMode
    mspu_sector = 0xAAAA;
    manhattan_bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000C);

    // EIP165.EgEIP160.StatSA: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE210, 0x0000000C);

    // EIP165.EgEIP160.StatSECY: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE410, 0x0000000C);

    // EIP165.EgEIP160.StatIFC: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xE610, 0x0000000C);

    // Program PE transform err detection
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF124, 0x80FE0000);

    // EIP165.EgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    manhattan_bcm54194_mspu_write(mspu_sector, 0x797C, 0x02000000);

    // Program packet engine to fetch only
    // 6 x 128-bit words of the context
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.EgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xE5880618);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // Program EIP-160 latency compensation FIFO
    // EIP165.EgEIP160.Port: Set
    // 'non_vlan_mtu_check': 32760(0x7ff8)
    // 'non_vlan_mtu_check_drop': 1(0x1)
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF200, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF204, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF208, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF20C, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF210, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF214, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF218, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF21C, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF220, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF224, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF228, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF22C, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF230, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF234, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF238, 0x0000FFF8);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF23C, 0x0000FFF8);

    // PICS-CSA-2/3/4: Cipher suite
    // Set Header bypass length to 0 bytes
    // EIP165.EgEIP160.Flow: Configure
    // 'hb_size': 0(0x0)
    // Installing flows to exercise confidentialityOffset
    // EIP165.EgEIP160.Flow: FlowCreate(Descr='MACsec out, confOffset=0 bytes')
    // Configure array of size 96 to address starting &H10000
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0000, 0x924bc066);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0004, 0x00c8a707);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0008, 0x651ead73);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x000C, 0x37f20a35);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0010, 0x8b603225);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0014, 0x25fbfd26);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0018, 0x1e63dc20);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x001C, 0x23d96efc);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0020, 0x9bb538a3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0024, 0x72445d05);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0028, 0x00000007);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x002C, 0x5b6ec642);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0030, 0xe7b3dd10);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0034, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0038, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x003C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0040, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0044, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0048, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x004C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0050, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0054, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0058, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x005C, 0x00000000);

    // WriteBlock to 65632 completed
    // Configure array of size 8 to address starting &H17000
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7000, 0x00270003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7004, 0x000c0c00);

    // WriteBlock to 94216 completed
    // Configure array of size 4 to address starting &H13800
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3800, 0x0000c000);

    // WriteBlock to 79876 completed
    // Writing the match rules for RULE0
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H12000
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2000, 0x00002003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2004, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2008, 0xc2d80000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x200C, 0x6782426b);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2008, 0xbbbb0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x200C, 0xbbbbbbbb);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2010, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2014, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2018, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x201C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2020, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2024, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2028, 0xffff0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x202C, 0xffffffff);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2030, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2034, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2038, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x203C, 0x00000000);

    // WriteBlock to 73792 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3000, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004000);

    // EIP165.EgEIP160.Flow:
    // FlowCreate(Descr='MACsec out, confOffset=4 bytes')
    // Configure array of size 96 to address starting &H10080
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0080, 0x924bc066);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0084, 0x00de6fed);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0088, 0x630c96f0);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x008C, 0x0e2f153b);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0090, 0x9c9c2ca3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0094, 0xe04883b4);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0098, 0x07a4a51a);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x009C, 0xa1e8bc48);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A0, 0x4f1e5a44);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A4, 0xaa7af0b3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A8, 0x00000007);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00AC, 0x63ca71e9);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B0, 0x323863ce);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B4, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00BC, 0x00010000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C0, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C4, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00CC, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D0, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D4, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00DC, 0x00000000);

    // WriteBlock to 65760 completed
    // Configure array of size 8 to address starting &H17008
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7008, 0x00270003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x700C, 0x040c0c00);

    // WriteBlock to 94224 completed
    // Configure array of size 4 to address starting &H13808
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3808, 0x0000c001);

    // WriteBlock to 79884 completed
    // Writing the match rules for RULE1
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H12040
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2040, 0x00002003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2044, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2048, 0x95d60000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x204C, 0x1510ceb7);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2050, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2054, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2058, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x205C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2060, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2064, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2068, 0xffff0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x206C, 0xffffffff);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2070, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2074, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2078, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x207C, 0x00000000);

    // WriteBlock to 73856 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3004, 0x00000001);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004001);

    /* Decryption configuration */
    // EIP165.IgEIP160.OutPostProc: CC_Configure
    mspu_sector = 0x5555;
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe844, 0x05ff0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe840, 0x0000c000);

    // EIP165.IgEIP160.StatTCAM: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000c);

    // EIP165.IgEIP160.StatRXCAM: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0x5410, 0x0000000c);

    // EIP165.IgEIP160.StatSA: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe210, 0x0000000c);

    // EIP165.IgEIP160.StatSECY: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe410, 0x0000000c);

    // EIP165.IgEIP160.StatIFC: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe610, 0x0000000c);

    // Program PE transform err detection
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF124, 0x80fe0000);

    // EIP165.IgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    manhattan_bcm54194_mspu_write(mspu_sector, 0x797C, 0x02080000);

    // Program packet engine to fetch only
    // 5 x 128-bit words of the context
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.IgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880618);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // EIP165.EgEIP160: DefaultMode
    // EIP165.EgEIP160.StatTCAM: DefaultMode
    mspu_sector = 0xAAAA;
    manhattan_bcm54194_mspu_write(mspu_sector, 0x4410, 0x0000000c);

    // EIP165.EgEIP160.StatSA: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe210, 0x0000000c);

    // EIP165.EgEIP160.StatSECY: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe410, 0x0000000c);

    // EIP165.EgEIP160.StatIFC: DefaultMode
    manhattan_bcm54194_mspu_write(mspu_sector, 0xe610, 0x0000000c);

    // Program PE transform err detection
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF124, 0x80fe0000);

    // EIP165.EgEIP160.Flow: Configure
    // 'context_size': 2(0x2)
    manhattan_bcm54194_mspu_write(mspu_sector, 0x797C, 0x02080000);

    // Program packet engine to fetch only
    // 6 x 128-bit words of the context
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880218);

    // Program PE to disabled context update for externally bad packets
    // Program PE to perform a strict comparison for packet number
    // EIP165.EgEIP160.EIP62: Configure
    // 'ctx_upd_ctrl': 3(0x3)
    // 'pn_thr_mode': 1(0x1)
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF408, 0xe5880618);
    manhattan_bcm54194_mspu_write(mspu_sector, 0xF430, 0x00000003);

    // Program EIP-160 latency compensation FIFO
    // EIP165.IgEIP160.Port: Set
    // 'non_vlan_mtu_check': 32760(0x7ff8)
    // 'non_vlan_mtu_check_drop': 1(0x1)
    // EIP165.IgEIP160.Flow: Configure
    // 'hb_size': 0(0x0)
    // PICS-CSA-2/3/4: Cipher suite
    // Installing flows to exercise confidentialityOffset
    // EIP165.IgEIP160.Flow: FlowCreate(Descr='MACsec in, confOffset=0 bytes, key_len=16')
    // Configure array of size 96 to address starting &H00
    mspu_sector = 0x5555;
    //printf("mfix Swaraj edited, %d\n", __LINE__);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0000, 0xd24bc06f);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0004, 0x00c8a707);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0008, 0x651ead73);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x000C, 0x37f20a35);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0010, 0x8b603225);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0014, 0x25fbfd26);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0018, 0x1e63dc20);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x001C, 0x23d96efc);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0020, 0x9bb538a3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0024, 0x72445d05);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0028, 0x00000001);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x002C, 0x00000080);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0030, 0x5b6ec642);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0034, 0xe7b3dd10);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0038, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x003C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0040, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0044, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0048, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x004C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0050, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0054, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0058, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x005C, 0x00000000);

    // WriteBlock to 96 completed
    // Configure array of size 8 to address starting &H7000
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7000, 0x00170002);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7004, 0x00000c00);

    // WriteBlock to 28680 completed
    // Configure array of size 8 to address starting &H3800
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3800, 0xc000c000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3804, 0xc000c000);

    // WriteBlock to 14344 completed
    // Configure array of size 12 to address starting &H3400
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3400, 0x5b6ec642);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3404, 0xe7b3dd10);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3408, 0x00000000);

    // WriteBlock to 13324 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3700, 0x00004000);

    // Writing the match rules for RULE0
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H2000
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2000, 0x00002003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2004, 0x00000000);
    //printf("mfix: commented out by Swaraj for screening. %d\n", __LINE__);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2008, 0xbbbb0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x200C, 0xbbbbbbbb);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2010, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2014, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2018, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x201C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2020, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2024, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2028, 0xffff0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x202C, 0xffffffff);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2030, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2034, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2038, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x203C, 0x00000000);

    // WriteBlock to 8256 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3000, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004000);

    // EIP165.IgEIP160.Flow:
    // FlowCreate(Descr='MACsec in, confOffset=4 bytes, key_len=16')
    // Configure array of size 96 to address starting &H80
    //printf("mfix: Swaraj edited. %d\n", __LINE__);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0080, 0xd24bc06f);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0084, 0x00de6fed);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0088, 0x630c96f0);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x008C, 0x0e2f153b);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0090, 0x9c9c2ca3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0094, 0xe04883b4);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x0098, 0x07a4a51a);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x009C, 0xa1e8bc48);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A0, 0x4f1e5a44);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A4, 0xaa7af0b3);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00A8, 0x00000001);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00AC, 0x00000080);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B0, 0x63ca71e9);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B4, 0x323863ce);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00B8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00BC, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C0, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C4, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00C8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00CC, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D0, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D4, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00D8, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x00DC, 0x00000000);

    // WriteBlock to 224 completed
    // Configure array of size 8 to address starting &H7008
    manhattan_bcm54194_mspu_write(mspu_sector, 0x7008, 0x00170002);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x700C, 0x04000C00);

    // WriteBlock to 28688 completed
    // Configure array of size 8 to address starting &H3808
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3808, 0xc001c001);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x380C, 0xc001c001);

    // WriteBlock to 14352 completed
    // Configure array of size 12 to address starting &H3410
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3410, 0x63ca71e9);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3414, 0x323863ce);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3418, 0x00000001);

    // WriteBlock to 13340 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3700, 0x00004001);

    // Writing the match rules for RULE1
    // Packet type is not specified for the rules (untagged&, tagged etc.).
    // Enabling all of them
    // Configure array of size 64 to address starting &H2040
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2040, 0x00002003);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2044, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2048, 0x95d60000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x204C, 0x1510ceb7);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2050, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2054, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2058, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x205C, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2060, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2064, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2068, 0xffff0000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x206C, 0xffffffff);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2070, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2074, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x2078, 0x00000000);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x207C, 0x00000000);

    // WriteBlock to 8320 completed
    manhattan_bcm54194_mspu_write(mspu_sector, 0x3004, 0x00000001);
    manhattan_bcm54194_mspu_write(mspu_sector, 0x6100, 0x00004001);

    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: disable_bcm54194_macsec
 *
 * Description:
 * Input:  phy_addr - phy address
 *
 * Output: NONE
 *
 *------------------------------------------------------------------
 */
int disable_bcm54194_macsec(uint phy_addr)
{
    /* Power down all MSPU's */
    ERET_COND(0 != MHT_RDB_WR(phy_addr, 0x084B, 0x03FF), -(__LINE__), "");
    return 0;
}

/***********************************************************************
 *
 * Function: enable_bcm54194_i2c_access
 *
 * Description: Enable BCM54194 I2C master mode, enable SDA/SCL line de-glitch.
 *              Need to enable it before I2C access.
 * Inputs: TRUE/FALSE
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int manhattan_bcm54194_i2c_access_enable (int port, int enable)
{
    int    phy_addr = MANHATTAN_BCM54194_PHYAD;
    ushort reg_val  = 0;
    int    offset   = BCM54194_I2C_MASTER_CTRL_REG;

    ERET_COND(0 != MHT_RDB_RD(phy_addr, offset, &reg_val), -(__LINE__), "");

    if (enable) {
        reg_val &= ~(BCM54194_I2C_CMD_MASK); /* clear operation */
        reg_val &= ~(0xf << 5); /* clear port select to 0 */
        reg_val &= ~(BCM54194_I2C_MASTER_EN_BIT);
        reg_val |= (BCM54194_I2C_SDA_DEGL_EN_BIT |
                    BCM54194_I2C_SCL_DEGL_EN_BIT |
                   (port << 5));
        reg_val &= ~BCM54194_I2C_SOFT_RST_BIT; /* TODO ?? */
    } else {
        reg_val &= ~(0xf << 5); /* clear port select to 0 */
        reg_val &= ~(BCM54194_I2C_MASTER_EN_BIT);
        reg_val |= (BCM54194_I2C_DISABLE_ALL_PORT << 5);
        reg_val |= BCM54194_I2C_SOFT_RST_BIT; /* TODO ?? */
    }

    ERET_COND(0 != MHT_RDB_WR(phy_addr, offset, reg_val), -(__LINE__), "");
    return 0;
}

/***********************************************************************
 *
 * Function: bcm54194_issue_i2c_cmd
 *
 * Description: Issuing the I2C command, then check the command execution status.
 *
 * Inputs: i2c_cmd : BCM54194_I2C_READ_CURR_CMD
 *                   BCM54194_I2C_WRITE_CMD
 *                   BCM54194_I2C_READ_CURR_CMD
 *                   BCM54194_I2C_FLUSH_CURR_CMD
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
static int bcm54194_issue_i2c_cmd (int i2c_cmd)
{
    int    phy_addr = MANHATTAN_BCM54194_PHYAD;
    int    repeat   = 100;
    ushort reg_val  = 0;

    ERET_COND(0 != MHT_RDB_RD(phy_addr, BCM54194_I2C_MASTER_CTRL_REG, &reg_val), -(__LINE__), "");
    reg_val &= ~(BCM54194_I2C_CMD_MASK);
    reg_val |= i2c_cmd;
    reg_val |= BCM54194_I2C_MASTER_EN_BIT;

    ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_CTRL_REG, reg_val), -(__LINE__), "");

    while (repeat--) {
        ERET_COND(0 != MHT_RDB_RD(phy_addr, BCM54194_I2C_MASTER_STS_REG, &reg_val), -(__LINE__), "");

        if (reg_val & BCM54194_I2C_CMD_DONE_BIT) {
            return 0;
        }
        switzer_mdelay(10);
    }

    ERET_COND(1, -(__LINE__), "I2C command execution failed. I2CM_CMD = 0x#.03%x, I2CM_STS = 0x#.04%x\n",
        i2c_cmd, reg_val);

    return 0;
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_read
 *
 * Description: Read 1 byte of data from a given address at selected I2C slave.
 *              Enable I2C master mode before I2C access.
 * Inputs:
 * slave_addr : external I2C slave devices address
 * offset : external I2C slave register offset
 * rd_val : read data
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_read (int port, int slave_addr, int offset, ushort *buf, int len)
{
    int phy_addr = MANHATTAN_BCM54194_PHYAD;
    int i        = 0;

    for(i = 0; i < len; i++) {
        ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 1), -(__LINE__), "Failed.\n");

        ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_DEV_ADDR_REG, slave_addr), -(__LINE__), "");
        ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset + i), -(__LINE__), "");

        ERET_COND(0 != bcm54194_issue_i2c_cmd(BCM54194_I2C_READ_CMD), -(__LINE__), "Failed.\n");

        ERET_COND(0 != MHT_RDB_RD(phy_addr, BCM54194_I2C_MASTER_RDAT_REG, &buf[i]), -(__LINE__), "");

        ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 0), -(__LINE__), "Failed.\n");
    }

    return 0;
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_write
 *
 * Description: Write 1 byte of data to a given address at selected I2C slave.
 *              Enable I2C master mode before I2C access.
 * Inputs:
 * slave_addr : external I2C slave devices address
 * offset : external I2C slave register offset
 * wr_val : write data
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_write (int port, int slave_addr, int offset, ushort wrval)
{
    int phy_addr = MANHATTAN_BCM54194_PHYAD;

    ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 1), -(__LINE__), "Failed.\n");

    ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_DEV_ADDR_REG, slave_addr), -(__LINE__), "");

    ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_REG_ADDR_REG, offset), -(__LINE__), "");

    ERET_COND(0 != MHT_RDB_WR(phy_addr, BCM54194_I2C_MASTER_WDAT_REG, wrval), -(__LINE__), "");

    ERET_COND(0 != bcm54194_issue_i2c_cmd(BCM54194_I2C_WRITE_CMD), -(__LINE__), "Failed.\n");

    ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 0), -(__LINE__), "Failed.\n");

    return 0;
}

/***********************************************************************
 *
 * Function: bcm54194_i2c_slave_flush
 *
 * Description: Reset a selected or all slaves.
 *
 * Inputs:
 * slave_addr : external I2C slave devices address.
 *
 * Outputs: PASSED/FAILED
 *
 *************************************************************************/
int bcm54194_i2c_slave_flush (int port, int slave_addr)
{
    ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 1), -(__LINE__), "Failed.\n");
    ERET_COND(0 != bcm54194_issue_i2c_cmd(BCM54194_I2C_FLUSH_CMD), -(__LINE__), "Failed.\n");
    ERET_COND(0 != bcm54194_issue_i2c_cmd(BCM54194_I2C_NO_OP_CMD), -(__LINE__), "Failed.\n");
    ERET_COND(0 != manhattan_bcm54194_i2c_access_enable(port, 0), -(__LINE__), "Failed.\n");

    return 0;
}

/*
 * Function: bcm54194_transmit_test_pattern
 *
 * Description: Enable BCM54194 RDB access mode.
 *
 * Hardware or software resets to the chip will enable RDB Access mode.
 * Input: none
 *
 * Return: none
 */
int bcm54194_transmit_test_pattern(int phy_addr, int mode)
{
    int    regnum  = 0x0;
    ushort reg_val;

    if ((mode == BCM54194_TEST_MODE_1) || (mode == BCM54194_TEST_MODE_2) ||
        (mode == BCM54194_TEST_MODE_4)) {
        /* Disable auto-negotiation and force to 1000BASE-T mode */
        regnum = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, 0x0040), -(__LINE__), "");

        /* Disable Auto-MDIX */
        regnum = BCM54194_COPPER_MISCEL_CTRL_REG;
        ERET_COND(0 != MHT_RDB_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");
        reg_val &= ~BCM54194_FORCE_AUTO_MDIX_BIT;
        ERET_COND(0 != MHT_RDB_WR(phy_addr, regnum, reg_val), -(__LINE__), "");
    } else {
        /* Enable Auto-Negotiation */
        regnum = BCM54194_CTRL_REG;
        ERET_COND(0 != MHT_MDIO_WR(phy_addr, regnum, 0x1140), -(__LINE__), "");
    }

    regnum = BCM54195_1000BASE_CTRL_REG;
    ERET_COND(0 != MHT_MDIO_RD(phy_addr, regnum, &reg_val), -(__LINE__), "");
    reg_val &= ~BCM54194_TEST_MODE_MASK;

    switch(mode)
    {
        case BCM54194_TEST_MODE_1:
            reg_val |= BCM54194_TRANSMIT_WAVE_TEST;
            break;
        case BCM54194_TEST_MODE_2:
            reg_val |= BCM54194_MS_TRANSMIT_JITTER_TEST;
            break;
        case BCM54194_TEST_MODE_3:
            reg_val |= BCM54194_SL_TRANSMIT_JITTER_TEST;
            break;
        case BCM54194_TEST_MODE_4:
            reg_val |= BCM54194_TRANSMIT_DIST_TEST;
            break;
        default:
            reg_val |= BCM54194_NORMAL_MODE;
            break;
    }
    ERET_COND(0 != MHT_MDIO_WR(phy_addr, BCM54195_1000BASE_CTRL_REG, reg_val), -(__LINE__), "");

    return 0;
}
