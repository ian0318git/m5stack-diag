#ifndef __SWITZER_MANHATTAN_BCM54194_REG_DETAIL_H__
#define __SWITZER_MANHATTAN_BCM54194_REG_DETAIL_H__

//field parser is orgnized as(take SGMII_CONTROL as an example)
//  bit_comp = "6:6,13:13" which means bit-6 and bit-13 together form a value which bit-6 is msb
//  name     = "SPEED_SELECTION"
//  val_desc = "0x0@@10 Mb/s SGMII;;0x1@@100 Mb/s;;0x2@@1000 Mb/s;;", that is in '(value@@string;;)+' sequence.
struct __fld_parser {
    char *name     ;
    char *attr     ;
    char *bit_comp ;
    char *val_desc ;
};

struct __reg_parser {
    #define __REG_TYPE_MDIO 0
    #define __REG_TYPE_RDB  1
    int type;   //0-reg, 1-mdio
    int offs;
    char *name;
    #define __REG_PARSE_ATTR_SGMII  1
    #define __REG_PARSE_ATTR_COPPER 2
    #define __REG_PARSE_ATTR_FIBER  4
    #define __REG_PARSE_ATTR_GLOBAL 8
    int attr;
    char *note;
    struct __fld_parser fld_info[18]; //actual max 16 elements/bits
};

#define REG_DETAIL__COPPER_PHY_EXTENDED_STATUS                                              \
{                                                                                           \
    __REG_TYPE_RDB,                                                                         \
    0x1,                                                                                    \
    "COPPER_PHY_EXTENDED_STATUS",                                                           \
    __REG_PARSE_ATTR_COPPER,                                                                \
    NULL,                                                                                   \
    {                                                                                       \
        {                                                                                   \
            "AN_BP_MISMATCH",                                                               \
            "RO",                                                                           \
            "15:15",                                                                        \
            "1@@Link Partner Base Page Selector Field mismatched Advertised "               \
                "Selector field since last read.;;"                                         \
            "0@@No mismatch detected since last read.;;"                                    \
        },                                                                                  \
        {                                                                                   \
            "WIRESPD_DWNGRADE",                                                             \
            "RO",                                                                           \
            "14:14",                                                                        \
            "1@@Ethernet@Wirespeed Auto-negotiation advertised speed downgraded.;;"         \
            "0@@No advertised speed downgrade.;;"                                           \
        },                                                                                  \
        {                                                                                   \
            "MDIX_STATE",                                                                   \
            "RO",                                                                           \
            "13:13",                                                                        \
            "1@@Crossover MDI mode.;;"                                                      \
            "0@@Normal MDI mode.;;"                                                         \
        },                                                                                  \
        {                                                                                   \
            "INTR_STATUS",                                                                  \
            "RO",                                                                           \
            "12:12",                                                                        \
            "1@@Unmasked interrupt in RDB_Register, offset = 0x0A currently active.;;"      \
            "0@@Interrupt cleared.;;"                                                       \
        },                                                                                  \
        {                                                                                   \
            "REMOTE_RCVR_STATUS",                                                           \
            "RO",                                                                           \
            "11:11",                                                                        \
            "1@@1000BASE-T Remote receiver status OK.;;"                                    \
            "0@@Remote receiver not OK since last read.;;"                                  \
        },                                                                                  \
        {                                                                                   \
            "LOCAL_RCVR_STATUS",                                                            \
            "RO",                                                                           \
            "10:10",                                                                        \
            "1@@1000BASE-T Local receiver OK.;;"                                            \
            "0@@Local receiver not OK since last read.;;"                                   \
        },                                                                                  \
        {                                                                                   \
            "LOCKED",                                                                       \
            "RO",                                                                           \
            "9:9",                                                                          \
            "1@@100/1000BASE-T Descrambler locked.;;"                                       \
            "0@@Descrambler unlocked.;;"                                                    \
        },                                                                                  \
        {                                                                                   \
            "LINK_STATUS",                                                                  \
            "RO",                                                                           \
            "8:8",                                                                          \
            "1@@Link pass.;;"                                                               \
            "0@@Link fail.;;"                                                               \
        },                                                                                  \
        {                                                                                   \
            "CRC_ERR",                                                                      \
            "RO",                                                                           \
            "7:7",                                                                          \
            "1@@CRC error detected.;;"                                                      \
            "0@@No CRC error since last read.;;"                                            \
        },                                                                                  \
        {                                                                                   \
            "CARRIER_EX_ERR",                                                               \
            "RO",                                                                           \
            "6:6",                                                                          \
            "1@@Carrier extension error detected since last read.;;"                        \
            "0@@No carrier extension error since last read. 01'b;;"                         \
        },                                                                                  \
        {                                                                                   \
            "BAD_SSD",                                                                      \
            "RO",                                                                           \
            "5:5",                                                                          \
            "1@@Bad SSD or False Carrier error detected since last read.;;"                 \
            "0@@No bad SSD or False Carrie error since last read.;;"                        \
        },                                                                                  \
        {                                                                                   \
            "BAD_ESD",                                                                      \
            "RO",                                                                           \
            "4:4",                                                                          \
            "1@@Bad ESD or Premature-End error detected since last read.;;"                 \
            "0@@No bad ESD Premature-End error since last read.;;"                          \
        },                                                                                  \
        {                                                                                   \
            "RX_CODE_ERR",                                                                  \
            "RO",                                                                           \
            "3:3",                                                                          \
            "1@@Receive error (invalid code group) detected since last read.;;"             \
            "0@@No receive error since last read;;"                                         \
        },                                                                                  \
        {                                                                                   \
            "TX_ERR",                                                                       \
            "RO",                                                                           \
            "2:2",                                                                          \
            "1@@Transmit error (invalid code group) received since last read.;;"            \
            "0@@No transmit error code received since last read.;;"                         \
        },                                                                                  \
        {                                                                                   \
            "LOCK_ERR",                                                                     \
            "RO",                                                                           \
            "1:1",                                                                          \
            "1@@100/1000BASE-T Lock error detected since last read.;;"                      \
            "0@@No lock error since last read.;;"                                           \
        },                                                                                  \
        {                                                                                   \
            "MLT3_ERR",                                                                     \
            "RO",                                                                           \
            "0:0",                                                                          \
            "1@@100BASE-TX MLT-3 code error detected since last read.;;"                    \
            "0@@No MLT-3 code error since last read. NOTE: Only valid in 100BASE-TX mode;;" \
        },                                                                                  \
        {NULL, NULL, NULL,},                                                                \
    }                                                                                       \
}

#define REG_DETAIL__COPPER_PHY_CRC_COUNTER \
{                                          \
    __REG_TYPE_RDB,                        \
    0x4,                                   \
    "COPPER_PHY_CRC_COUNTER",              \
    __REG_PARSE_ATTR_COPPER,               \
    NULL,                                  \
    {                                      \
        {                                  \
            "RX_CRC_COUNTER",              \
            "RO CR",                       \
            "15:0",                        \
            "NOTE@@Number of received CRC errors from MDI, freezes at 0xFFFF, " \
            "enabled by RDB 0x00E, bit[15] = 1." \
        },                                 \
        {NULL, NULL, NULL,},               \
    }                                      \
}

#define REG_DETAIL__COPPER_AUXILIARY_STATUS_SUMMARY                                   \
{                                                                                     \
    __REG_TYPE_RDB,                                                                   \
    0x9,                                                                              \
    "COPPER_AUXILIARY_STATUS_SUMMARY",                                                \
    __REG_PARSE_ATTR_COPPER,                                                          \
    NULL,                                                                             \
    {                                                                                 \
        {                                                                             \
            "AUTONEG_COMPLETE",                                                       \
            "RO ",                                                                    \
            "15:15",                                                                  \
            "1@@Auto-negotiation complete.;;"                                         \
            "0@@Auto-negotiation in progress;;"                                       \
        },                                                                            \
        {                                                                             \
            "AUTONEG_HCD",                                                            \
            "RO ",                                                                    \
            "10:8",                                                                   \
            "0x7@@1000BASE-T full-duplex.;;"                                          \
            "0x6@@1000BASE-T half-duplex.;;"                                          \
            "0x5@@100BASE-TX full-duplex.;;"                                          \
            "0x4@@100BASE-T4.;;"                                                      \
            "0x3@@100BASE-TX half-duplex.;;"                                          \
            "0x2@@10BASE-T full-duplex.;;"                                            \
            "0x1@@10BASE-T half-duplex.;;"                                            \
            "0x0@@No highest common denominator or auto-negotiation not complete.;;"  \
        },                                                                            \
        {                                                                             \
            "PARALLEL_DET_FAULT",                                                     \
            "RO ",                                                                    \
            "7:7",                                                                    \
            "1@@Parallel link fault detected.;;"                                      \
            "0@@Parallel link fault not detected.;;"                                  \
        },                                                                            \
        {                                                                             \
            "REMOTE_FAULT",                                                           \
            "RO ",                                                                    \
            "6:6",                                                                    \
            "1@@Link partner has detected remote fault.;;"                            \
            "0@@Link partner has not detected remote fault.;;"                        \
        },                                                                            \
        {                                                                             \
            "PAGE_RECEIVED",                                                          \
            "RO ",                                                                    \
            "5:5",                                                                    \
            "1@@New page has been received from link partner.;;"                      \
            "0@@New page has not been received.;;"                                    \
        },                                                                            \
        {                                                                             \
            "LINK_PARTNER_AN_ABILITY",                                                \
            "RO ",                                                                    \
            "4:4",                                                                    \
            "1@@Link partner has auto-negotiation capability.;;"                      \
            "0@@Link partner does not perform auto-negotiation.;;"                    \
        },                                                                            \
        {                                                                             \
            "LINK_PARTNER_NP_ABILITY",                                                \
            "RO ",                                                                    \
            "3:3",                                                                    \
            "1@@Link partner has Next Page capability.;;"                             \
            "0@@Link partner does not have Next Page capability.;;"                   \
        },                                                                            \
        {                                                                             \
            "LINK_STATUS",                                                            \
            "RO ",                                                                    \
            "2:2",                                                                    \
            "1@@Link is up (link pass state).;;"                                      \
            "0@@Link is down (link fail state).;;"                                    \
        },                                                                            \
        {                                                                             \
            "PAUSE_RESOLUTION_RX",                                                    \
            "RO ",                                                                    \
            "1:1",                                                                    \
            "1@@Enable pause receive.;;"                                              \
            "0@@Disable pause receive.(valid when auto-negotiation has completed);;"  \
        },                                                                            \
        {                                                                             \
            "PAUSE_RESOLUTION_TX",                                                    \
            "RO ",                                                                    \
            "0:0",                                                                    \
            "1@@Enable pause transmit."                                               \
            "0@@Disable pause transmit.(valid when auto-negotiation has completed);;" \
        },                                                                            \
        {NULL, NULL, NULL,},                                                          \
    }                                                                                 \
}

#define REG_DETAIL__TEST_1                                                              \
{                                                                                       \
    __REG_TYPE_RDB,                                                                     \
    0xE,                                                                                \
    "TEST_1",                                                                           \
    __REG_PARSE_ATTR_COPPER,                                                            \
    NULL,                                                                               \
    {                                                                                   \
        {                                                                               \
            "CRC_ERR_CNT",                                                              \
            "RW",                                                                       \
            "15:15",                                                                    \
            "1@@Enables 16-bit CRC error counter (RDB 0x004).;;0@@Normal operation.;;"  \
        },                                                                              \
        {                                                                               \
            "FORCE_LINK",                                                               \
            "RW",                                                                       \
            "12:12",                                                                    \
            "1@@Force Link State machine into link pass state.;;0@@Normal operation.;;" \
        },                                                                              \
        {                                                                               \
            "MANUAL_SWAP_MDI",                                                          \
            "RW",                                                                       \
            "7:7",                                                                      \
            "1@@Manually Swap MDI state.;;0@@Normal operation.;;"                       \
        },                                                                              \
        {NULL, NULL, NULL,},                                                            \
    }                                                                                   \
}

#define REG_DETAIL__MODE_CONTROL                                     \
{                                                                    \
    __REG_TYPE_RDB,                                                  \
    0x21,                                                            \
    "MODE_CONTROL",                                                  \
    __REG_PARSE_ATTR_COPPER | __REG_PARSE_ATTR_FIBER,                \
    NULL,                                                            \
    {                                                                \
        {                                                            \
            "DUAL_SERDES_CAPABLE",                                   \
            "RO",                                                    \
            "9:9",                                                   \
            "1@@PHY supports SGMII-to-fiber mode.;;"                 \
            "0@@PHY does not support SGMII-to-fiber mode.;;"         \
        },                                                           \
        {                                                            \
            "COPPER_LINK",                                           \
            "RO",                                                    \
            "7:7",                                                   \
            "1@@Link-up on copper side.;;"                           \
            "0@@Link-down.;;"                                        \
        },                                                           \
        {                                                            \
            "SERDES_LINK",                                           \
            "RO",                                                    \
            "6:6",                                                   \
            "1@@Fiber link-up.;;"                                    \
            "0@@Link-down.;;"                                        \
        },                                                           \
        {                                                            \
            "COPPER_ENG_DET",                                        \
            "RO",                                                    \
            "5:5",                                                   \
            "1@@Copper energy detected.;;"                           \
            "0@@No copper energy detected.;;"                        \
        },                                                           \
        {                                                            \
            "FIBER_SIGNAL_DET",                                      \
            "RO",                                                    \
            "4:4",                                                   \
            "1@@Fiber signal detect from ball.;;"                    \
            "0@@No fiber signal detect from ball.;;"                 \
        },                                                           \
        {                                                            \
            "SERDES_CAPABLE",                                        \
            "RO",                                                    \
            "3:3",                                                   \
            "1@@SerDes-capable device.;;"                            \
            "0@@Not SerDes-capable device.;;"                        \
        },                                                           \
        {                                                            \
            "MODE_SEL",                                              \
            "RW",                                                    \
            "2:1",                                                   \
            "0@@Copper;;"                                            \
            "1@@Fiber;;"                                             \
            "2@@Reserved.;;"                                         \
            "3@@Reserved;;"                                          \
        },                                                           \
        {                                                            \
            "REG_1000X_EN",                                          \
            "RW",                                                    \
            "0:0",                                                   \
            "1@@Select 1000BASE-X or SGMII registers 0x0 to 0x0F.;;" \
            "0@@Select copper registers 0x0 to 0x0F.;;"              \
        },                                                           \
        {NULL, NULL, NULL,},                                         \
    }                                                                \
}

#define REG_DETAIL__COPPER_AUXILIARY_CONTROL                                        \
{                                                                                   \
    __REG_TYPE_RDB,                                                                 \
    0x28,                                                                           \
    "COPPER_AUXILIARY_CONTROL",                                                     \
    __REG_PARSE_ATTR_COPPER,                                                        \
    NULL,                                                                           \
    {                                                                               \
        {                                                                           \
            "EXT_LPBK",                                                             \
            "RW",                                                                   \
            "15:15",                                                                \
            "1@@External loopback enabled.;;"                                       \
            "0@@Normal operation.;;"                                                \
        },                                                                          \
        {                                                                           \
            "100BT_JUMBO",                                                          \
            "RW",                                                                   \
            "14:14",                                                                \
            "1@@Enable support for 100BASE-TX jumbo packets.;;"                     \
            "0@@Normal operation.;;"                                                \
        },                                                                          \
        {                                                                           \
            "ENABLE_DSP_CLOCK",                                                     \
            "RW",                                                                   \
            "11:11",                                                                \
            "1@@Enable DSP clock.;;"                                                \
            "0@@Normal operation.;;"                                                \
        },                                                                          \
        {                                                                           \
            "100BT_RISE_FALL",                                                      \
            "RW",                                                                   \
            "5:4",                                                                  \
            "3@@Fastest rise/fall time.;;"                                          \
            "2@@Decrease rise/fall time approximately 1 ns from default setting.;;" \
            "1@@Decrease rise/fall time approximately 2 ns from default setting.;;" \
            "0@@Decrease rise/fall time approximately 3 ns from default setting.;;" \
        },                                                                          \
        {                                                                           \
            "SHD18_SELECT",                                                         \
            "RW",                                                                   \
            "2:0",                                                                  \
            "0x0@@AUXILIARY_CONTROL Register.;;"                                    \
            "0x1@@10BASE-T Register.;;"                                             \
            "0x2@@POWER/MII_CONTROL Register.;;"                                    \
            "0x4@@MISCELLANEOUS_TEST Register.;;"                                   \
            "0x7@@MISCELLANEOUS_CONTROL Register.;;"                                \
        },                                                                          \
        {NULL, NULL, NULL,},                                                        \
    }                                                                               \
}

#define REG_DETAIL__COPPER_MISCELLANEOUS_CONTROL                                                 \
{                                                                                                \
    __REG_TYPE_RDB,                                                                              \
    0x2F,                                                                                        \
    "COPPER_MISCELLANEOUS_CONTROL",                                                              \
    __REG_PARSE_ATTR_COPPER,                                                                     \
    NULL,                                                                                        \
    {                                                                                            \
        {                                                                                        \
            "PKT_CNTR_ENB",                                                                      \
            "R/W",                                                                               \
            "11:11",                                                                             \
            "1@@Receive packet counter.;;0@@Transmit packet counter."                            \
        },                                                                                       \
        {                                                                                        \
            "BYPASS_WIRESPEED_TIMER",                                                            \
            "R/W",                                                                               \
            "10:10",                                                                             \
            "1@@Link fail counter is cleared as soon as link is up.;;"                           \
            "0@@Link must be up for at least three seconds, else fail counter is incremented."   \
        },                                                                                       \
        {                                                                                        \
            "FORCE_AUTO_MDIX",                                                                   \
            "R/W",                                                                               \
            "9:9",                                                                               \
            "1@@Auto-MDIX is enabled when auto-negotiation is disabled.;;"                       \
            "0@@Auto-MDIX is disabled when auto-negotiation is disabled."                        \
        },                                                                                       \
        {                                                                                        \
            "WIRESPEED_EN",                                                                      \
            "R/W",                                                                               \
            "4:4",                                                                               \
            "1@@Enable Ethernet@Wirespeed.;;0@@Disable Ethernet@Wirespeed."                      \
        },                                                                                       \
        {                                                                                        \
            "MDIO_ALL_PHY_SEL",                                                                  \
            "R/W",                                                                               \
            "3:3",                                                                               \
            "1@@All ports respond to MDIO writes to PHY Address=5'b00000.;;0@@Normal operation." \
        },                                                                                       \
        {NULL, NULL, NULL,},                                                                     \
    }                                                                                            \
}


#define REG_DETAIL__RX_TX_PACKET_COUNTER   \
{                                          \
    __REG_TYPE_RDB,                        \
    0x30,                                  \
    "RX_TX_PACKET_COUNTER",                \
    __REG_PARSE_ATTR_COPPER,               \
    NULL,                                  \
    {                                      \
        {                                  \
            "PKT_CNTR",                    \
            "RO CR",                       \
            "15:0",                        \
            "NOTE@@Copper Transmit or Receive packet counter." \
        },                                 \
        {NULL, NULL, NULL,},               \
    }                                      \
}


#define REG_DETAIL__TOP_MISC_SFP_STS0                                      \
{                                                                          \
    __REG_TYPE_RDB,                                                        \
    0x890,                                                                 \
    "TOP_MISC_SFP_STS0",                                                   \
    __REG_PARSE_ATTR_GLOBAL,                                               \
    NULL,                                                                  \
    {                                                                      \
        {                                                                  \
            "SFP_p3_conn_sts",                                             \
            "RO",                                                          \
            "15:15",                                                       \
            "1@@Connected.;;" "0@@Disconnected.;;"                         \
        },                                                                 \
        {                                                                  \
            "SFP_p2_conn_sts",                                             \
            "RO",                                                          \
            "14:14",                                                       \
            "1@@Connected.;;" "0@@Disconnected.;;"                         \
        },                                                                 \
        {                                                                  \
            "SFP_p1_conn_sts",                                             \
            "RO",                                                          \
            "13:13",                                                       \
            "1@@Connected.;;" "0@@Disconnected.;;"                         \
        },                                                                 \
        {                                                                  \
            "SFP_p0_conn_sts",                                             \
            "RO",                                                          \
            "12:12",                                                       \
            "1@@Connected.;;" "0@@Disconnected.;;"                         \
        },                                                                 \
        {                                                                  \
            "SFP_p3_rxl_chg ",                                             \
            "RO",                                                          \
            "11:11",                                                       \
            "1@@RX loss state change.;;" "0@@No RX loss state change.;;"   \
        },                                                                 \
        {                                                                  \
            "SFP_p3_txf_chg ",                                             \
            "RO",                                                          \
            "10:10",                                                       \
            "1@@TX fault state change.;;" "0@@No TX fault state change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p3_conn_chg",                                             \
            "RO",                                                          \
            "09:09",                                                       \
            "1@@SFP connection change.;;" "0@@No SFP connection change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p2_rxl_chg ",                                             \
            "RO",                                                          \
            "08:08",                                                       \
            "1@@RX loss state change.;;" "0@@No RX loss state change.;;"   \
        },                                                                 \
        {                                                                  \
            "SFP_p2_txf_chg ",                                             \
            "RO",                                                          \
            "07:07",                                                       \
            "1@@TX fault state change.;;" "0@@No TX fault state change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p2_conn_chg",                                             \
            "RO",                                                          \
            "06:06",                                                       \
            "1@@SFP connection change.;;" "0@@No SFP connection change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p1_rxl_chg ",                                             \
            "RO",                                                          \
            "05:05",                                                       \
            "1@@RX loss state change.;;" "0@@No RX loss state change.;;"   \
        },                                                                 \
        {                                                                  \
            "SFP_p1_txf_chg ",                                             \
            "RO",                                                          \
            "04:04",                                                       \
            "1@@TX fault state change.;;" "0@@No TX fault state change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p1_conn_chg",                                             \
            "RO",                                                          \
            "03:03",                                                       \
            "1@@SFP connection change.;;" "0@@No SFP connection change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p0_rxl_chg ",                                             \
            "RO",                                                          \
            "02:02",                                                       \
            "1@@RX loss state change.;;" "0@@No RX loss state change.;;"   \
        },                                                                 \
        {                                                                  \
            "SFP_p0_txf_chg ",                                             \
            "RO",                                                          \
            "01:01",                                                       \
            "1@@TX fault state change.;;" "0@@No TX fault state change.;;" \
        },                                                                 \
        {                                                                  \
            "SFP_p0_conn_chg",                                             \
            "RO",                                                          \
            "00:00",                                                       \
            "1@@SFP connection change.;;" "0@@No SFP connection change.;;" \
        },                                                                 \
        {NULL, NULL, NULL,},                                               \
    }                                                                      \
}

#define REG_DETAIL__SGMII_CTRL                                                                 \
{                                                                                          \
    __REG_TYPE_MDIO ,                                                                      \
    0               ,                                                                      \
    "SGMII_CTRL"    ,                                                                      \
    __REG_PARSE_ATTR_SGMII,                                                                \
    NULL,                                                                                  \
    {                                                                                      \
        {                                                                                  \
            "SPEED_SELECTION",                                                             \
            "R/W",                                                                         \
            "6:6,13:13",                                                                   \
            "0x0@@10 Mb/s SGMII;;0x1@@100 Mb/s;;0x2@@1000 Mb/s;;"                          \
        },                                                                                 \
        {                                                                                  \
            "DUPLEX",                                                                      \
            "R/W",                                                                         \
            "8:8",                                                                         \
            "1@@Full duplex;;0@@Haf duplex;;"                                              \
        },                                                                                 \
        {                                                                                  \
            "RESTART_AN",                                                                  \
            "R/W SC",                                                                      \
            "9:9",                                                                         \
            "1@@Restart auto-negotiation;;0@@Auto-negotiation restart complete;;"          \
        },                                                                                 \
        {                                                                                  \
            "POWER_DOWN",                                                                  \
            "R/W",                                                                         \
            "11:11",                                                                       \
            "1@@PHY is in low-power standby mode, "                                        \
            "SGMII interface is powered down, twisted pair interface remains powered-up;;" \
            "0@@Normal operation.;;"                                                       \
        },                                                                                 \
        {                                                                                  \
            "SGMII_AN_ENABLE",                                                             \
            "R/W",                                                                         \
            "12:12",                                                                       \
            "1@@Auto-negotiation enabled;;0@@Auto-negotiation disabled.;;"                 \
        },                                                                                 \
        {                                                                                  \
            "LOOPBACK",                                                                    \
            "R/W",                                                                         \
            "14:14",                                                                       \
            "1@@SGMII loopback enabled;;0@@Normal operation.;;"                            \
        },                                                                                 \
        {                                                                                  \
            "RESET",                                                                       \
            "R/W SC",                                                                      \
            "15:15",                                                                       \
            "1@@Reset in progress;;0@@Normal operation."                                   \
        },                                                                                 \
        {NULL, NULL, NULL},                                                                \
    }                                                                                      \
}

#define REG_DETAIL__SGMII_STATUS                                                          \
{                                                                                         \
    __REG_TYPE_MDIO ,                                                                     \
    1               ,                                                                     \
    "SGMII_STATUS"  ,                                                                     \
    __REG_PARSE_ATTR_SGMII,                                                               \
    NULL,                                                                                 \
    {                                                                                     \
        {                                                                                 \
            "SGMII_LINK_STATUS",                                                          \
            "RO",                                                                         \
            "2:2",                                                                        \
            "1@@SGMII Link is up;;0@@SGMII Link is down"                                  \
        },                                                                                \
        {                                                                                 \
            "SGMI_AN_COMPLETE",                                                           \
            "RO",                                                                         \
            "5:5",                                                                        \
            "1@@SGMII Auto-negotiation complete.;;0@@SGMII Auto-negotiation in progress." \
        },                                                                                \
        {NULL, NULL, NULL},                                                               \
    }                                                                                     \
}

#define REG_DETAIL__SGMII_AN_ADVERTISEMENT                                                     \
{                                                                                              \
    __REG_TYPE_MDIO ,                                                                          \
    4               ,                                                                          \
    "SGMII_AN_ADVERTISEMENT"  ,                                                                \
    __REG_PARSE_ATTR_SGMII,                                                                    \
    NULL,                                                                                      \
    {                                                                                          \
        {                                                                                      \
            "SGMII_SELECTOR",                                                                  \
            "R/W",                                                                             \
            "0:0",                                                                             \
            "1@@SGMII mode"                                                                    \
        },                                                                                     \
        {                                                                                      \
            "EEE_CAP",                                                                         \
            "R/W",                                                                             \
            "9:9",                                                                             \
            "1@@Remote device is Energy Efficient Ethernet capable;;"                          \
            "0@@Remote device is not Energy Efficient Ethernet capable."                       \
        },                                                                                     \
        {                                                                                      \
            "COPPER_SPEED",                                                                    \
            "R/W",                                                                             \
            "11:10",                                                                           \
            "0@@10BASE-T;;1@@100BASE-TX;;2@@1000BASE-T"                                        \
        },                                                                                     \
        {                                                                                      \
            "COPPER_DUPLEX",                                                                   \
            "R/W",                                                                             \
            "12:12",                                                                           \
            "1@@Full-duplex;;0@@Half-duplex"                                                   \
        },                                                                                     \
        {                                                                                      \
            "COPPER_LINK",                                                                     \
            "R/W",                                                                             \
            "15:15",                                                                           \
            "1@@Copper link. PHY has established a 10BASE-T, 100BASE-TX, or 1000BASE-T link;;" \
            "0@@No link"                                                                       \
        },                                                                                     \
        {NULL, NULL, NULL},                                                                    \
    }                                                                                          \
}

#define REG_DETAIL__SGMII_AN_LINK_PARTNER_ABILITY             \
{                                                             \
    __REG_TYPE_MDIO,                                          \
    5,                                                        \
    "SGMII_AN_LINK_PARTNER_ABILITY",                          \
    __REG_PARSE_ATTR_SGMII,                                   \
    NULL,                                                     \
    {                                                         \
        {                                                     \
            "SGMII_SELECTOR",                                 \
            "RO",                                             \
            "0:0",                                            \
            "1@@SGMII mode;;"                                 \
        },                                                    \
        {                                                     \
            "ACKNOWLEDGE",                                    \
            "RO",                                             \
            "14:14",                                          \
            "1@@Link partner has received link code word;;"   \
            "0@@Link partner has not received link code word" \
        },                                                    \
        {NULL, NULL, NULL},                                   \
    }                                                         \
}

#define REG_DETAIL__COPPER_MII_CONTROL                                                     \
{                                                                                          \
    __REG_TYPE_MDIO,                                                                       \
    0,                                                                                     \
    "COPPER_MII_CONTROL",                                                                  \
    __REG_PARSE_ATTR_COPPER,                                                               \
    NULL,                                                                                  \
    {                                                                                      \
        {                                                                                  \
            "UNIDIRECTIONAL_EN",                                                           \
            "R/W",                                                                         \
            "5:5",                                                                         \
            "1@@Trans from MII does not depends on link staus.;;"                          \
            "0@@Trans from MII only valid link established."                               \
        },                                                                                 \
        {                                                                                  \
            "SPEED_SELECTION",                                                             \
            "R/W",                                                                         \
            "6:6,13:13",                                                                   \
            "0x0@@10BASE-T;;0x1@@100BASE-T;;0x2@@1000BASE-T;;0x3@@Reserved"                \
        },                                                                                 \
        {                                                                                  \
            "DUPLEX_MODE",                                                                 \
            "R/W",                                                                         \
            "8:8",                                                                         \
            "1@@Full duplex;;0@@Half duplex"                                               \
        },                                                                                 \
        {                                                                                  \
            "RESTART_AN",                                                                  \
            "R/W SC",                                                                      \
            "9:9",                                                                         \
            "1@@Restart auto-negotiation;;0@@Auto-negotiation restart complete;;"          \
        },                                                                                 \
        {                                                                                  \
            "POWER_DOWN",                                                                  \
            "R/W",                                                                         \
            "11:11",                                                                       \
            "1@@PHY is in low-power standby mode, "                                        \
            "SGMII interface is powered down, twisted pair interface remains powered-up;;" \
            "0@@Normal operation.;;"                                                       \
        },                                                                                 \
        {                                                                                  \
            "COPPER_AN_ENABLE",                                                            \
            "R/W",                                                                         \
            "12:12",                                                                       \
            "1@@Auto-negotiation enabled;;0@@Auto-negotiation disabled.;;"                 \
        },                                                                                 \
        {                                                                                  \
            "INTERNAL_LOOPBACK",                                                           \
            "R/W",                                                                         \
            "14:14",                                                                       \
            "1@@Loopback enabled;;0@@Normal operation.;;"                                  \
        },                                                                                 \
        {                                                                                  \
            "RESET",                                                                       \
            "R/W SC",                                                                      \
            "15:15",                                                                       \
            "1@@Reset in progress;;0@@Normal operation."                                   \
        },                                                                                 \
        {NULL, NULL, NULL},                                                                \
    }                                                                                      \
}

#define REG_DETAIL__COPPER_MII_STATUS                                                                               \
{                                                                                                                   \
    __REG_TYPE_MDIO,                                                                                                \
    1,                                                                                                              \
    "COPPER_MII_STATUS",                                                                                            \
    __REG_PARSE_ATTR_COPPER,                                                                                        \
    NULL,                                                                                                           \
    {                                                                                                               \
        {                                                                                                           \
            "100BASE-T4_CAP"    ,                                                                                   \
            "RO",                                                                                                   \
            "15:15",                                                                                                \
            "1@@Capable.;;0@@Not capable."                                                                          \
        },                                                                                                          \
        {                                                                                                           \
            "100BASE-X_FD_CAP"  ,                                                                                   \
            "RO",                                                                                                   \
            "14:14",                                                                                                \
            "1@@Full-duplex capable.;;0@@Not full-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "100BASE-X_HD_CAP"  ,                                                                                   \
            "RO",                                                                                                   \
            "13:13",                                                                                                \
            "1@@Half-duplex capable.;;0@@Not half-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "10BASE-T_FD_CAP"   ,                                                                                   \
            "RO",                                                                                                   \
            "12:12",                                                                                                \
            "1@@Full-duplex capable.;;0@@Not full-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "10BASE-T_HD_CAP"   ,                                                                                   \
            "RO",                                                                                                   \
            "11:11",                                                                                                \
            "1@@Half-duplex capable.;;0@@Not half-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "100BASE-T2_FD_CAP" ,                                                                                   \
            "RO",                                                                                                   \
            "10:10",                                                                                                \
            "1@@Full-duplex capable.;;0@@Not full-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "100BASE-T2_HD_CAP" ,                                                                                   \
            "RO",                                                                                                   \
            "9:9"  ,                                                                                                \
            "1@@Half-duplex capable.;;0@@Not half-duplex capable."                                                  \
        },                                                                                                          \
        {                                                                                                           \
            "EXTENDED_STATUS"       ,                                                                               \
            "RO",                                                                                                   \
            "8:8"  ,                                                                                                \
            "1@@Extended status information in Register 0x0F.;;0@@No extended status information in Register 0x0F." \
        },                                                                                                          \
        {                                                                                                           \
            "UNIDIRECTIONAL_CAP",                                                                                   \
            "RO",                                                                                                   \
            "7:7"  ,                                                                                                \
            "1@@Transmit from MII regardless of valid link or not.;;0@@Transmit from MII only when valid link."     \
        },                                                                                                          \
        {                                                                                                           \
            "MF_PREAMBLE_SUPPRESS"  ,                                                                               \
            "RO",                                                                                                   \
            "6:6"  ,                                                                                                \
            "1@@MDIO preamble can be suppressed.;;0@@MDIO preamble always required."                                \
        },                                                                                                          \
        {                                                                                                           \
            "AN_COMPLETE"           ,                                                                               \
            "RO",                                                                                                   \
            "5:5"  ,                                                                                                \
            "1@@Auto-negotiation complete.;;0@@Auto-negotiation in progress."                                       \
        },                                                                                                          \
        {                                                                                                           \
            "REMOTE_FAULT"          ,                                                                               \
            "RO",                                                                                                   \
            "4:4"  ,                                                                                                \
            "1@@Remote fault detected(Remains until remote fault cleared and the register is read).;;"              \
            "0@@No remote fault detected."                                                                          \
        },                                                                                                          \
        {                                                                                                           \
            "AN_ABILITY"            ,                                                                               \
            "RO",                                                                                                   \
            "3:3"  ,                                                                                                \
            "1@@Auto-negotiation capable.;;0@@Not auto-negotiation capable."                                        \
        },                                                                                                          \
        {                                                                                                           \
            "COPPER_LINK_STATUS"    ,                                                                               \
            "RO",                                                                                                   \
            "2:2"  ,                                                                                                \
            "1@@Link is up;;0@@Link is down(latched until read)"                                                    \
        },                                                                                                          \
        {                                                                                                           \
            "JABBER_DETECT"         ,                                                                               \
            "RO",                                                                                                   \
            "1:1"  ,                                                                                                \
            "1@@Detected.;;0@@No detected."                                                                         \
        },                                                                                                          \
        {                                                                                                           \
            "EXTENDED_CAPABILITY"   ,                                                                               \
            "RO",                                                                                                   \
            "0:0"  ,                                                                                                \
            "1@@Supports Extended Capability registers;;0@@No extended register capabilities."                      \
        },                                                                                                          \
    }                                                                                                               \
}

#define REG_DETAIL__COPPER_AN_ADVERTISEMENT                                        \
{                                                                                  \
    __REG_TYPE_MDIO,                                                               \
    4,                                                                             \
    "COPPER_AN_ADVERTISEMENT",                                                     \
    __REG_PARSE_ATTR_COPPER,                                                       \
    NULL,                                                                          \
    {                                                                              \
        {                                                                          \
            "PROTOCOL_SELECTOR",                                                   \
            "R/W",                                                                 \
            "4:0",                                                                 \
            "1@@IEEE 802.3 CSMA/CD(See doc);;"                                     \
        },                                                                         \
        {                                                                          \
            "10BASE-T_HD_CAP",                                                     \
            "R/W",                                                                 \
            "5:5",                                                                 \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "10BASE-T_FD_CAP",                                                     \
            "R/W",                                                                 \
            "6:6",                                                                 \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "100BASE-TX_HD_CAP",                                                   \
            "R/W",                                                                 \
            "7:7",                                                                 \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "100BASE-TX_FD_CAP",                                                   \
            "R/W",                                                                 \
            "8:8",                                                                 \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "100BASE-T4_FD_CAP",                                                   \
            "R/W",                                                                 \
            "9:9",                                                                 \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "PAUSE_CAP",                                                           \
            "R/W",                                                                 \
            "10:10",                                                               \
            "1@@Capable;;0@@Not Capable"                                           \
        },                                                                         \
        {                                                                          \
            "ASYMMETRIC_PAUSE",                                                    \
            "R/W",                                                                 \
            "10:10,11:11",                                                         \
            "0@@No pause;;"                                                        \
            "1@@Asymmetric PAUSE toward link partner;;"                            \
            "2@@Symmetric PAUSE;;"                                                 \
            "3@@Both Symmetric PAUSE and Asymmetric PAUSE toward local device."    \
        },                                                                         \
        {                                                                          \
            "REMOTE_FAULT",                                                        \
            "R/W",                                                                 \
            "13:13",                                                               \
            "1@@Advertise remote fault;;0@@Not advertise remote fault."            \
        },                                                                         \
        {                                                                          \
            "NEXT_PAGE",                                                           \
            "R/W",                                                                 \
            "15:15",                                                               \
            "1@@Next Page ability supported.;;0@@Next Page ability not supported." \
        },                                                                         \
        {NULL, NULL, NULL},                                                        \
    }                                                                              \
}

#define REG_DETAIL__COPPER_AN_LINK_PARTNER_ABILITY                              \
{                                                                               \
    __REG_TYPE_MDIO,                                                            \
    5,                                                                          \
    "COPPER_AN_LINK_PARTNER_ABILITY",                                           \
    __REG_PARSE_ATTR_COPPER,                                                    \
    NULL,                                                                       \
    {                                                                           \
        {                                                                       \
            "PROTOCOL_SELECTOR",                                                \
            "RO",                                                               \
            "4:0",                                                              \
            "0@@See doc"                                                        \
        },                                                                      \
        {                                                                       \
            "10BASE-T_HD_CA",                                                   \
            "RO",                                                               \
            "5:5",                                                              \
            "1@@Capable;;0@@Not capable."                                       \
        },                                                                      \
        {                                                                       \
            "10BASE-T_FD_CAP",                                                  \
            "RO",                                                               \
            "6:6",                                                              \
            "1@@Capable;;0@@Not capable."                                       \
        },                                                                      \
        {                                                                       \
            "100BASE-TX_HD_CAP",                                                \
            "RO",                                                               \
            "7:7",                                                              \
            "1@@Capable;;0@@Not capable."                                       \
        },                                                                      \
        {                                                                       \
            "100BASE-TX_FD_CAP",                                                \
            "RO",                                                               \
            "8:8",                                                              \
            "1@@Capable;;0@@Not capable."                                       \
        },                                                                      \
        {                                                                       \
            "100BASE-T4_CAP",                                                   \
            "RO",                                                               \
            "9:9",                                                              \
            "1@@Capable;;0@@Not capable."                                       \
        },                                                                      \
        {                                                                       \
            "PAUSE_CAP",                                                        \
            "RO",                                                               \
            "10:10",                                                            \
            "1@@Capable;;0@@Not Capable"                                        \
        },                                                                      \
        {                                                                       \
            "ASYMMETRIC_PAUSE",                                                 \
            "RO",                                                               \
            "10:10,11:11",                                                      \
            "0@@No pause;;"                                                     \
            "1@@Asymmetric PAUSE toward link partner;;"                         \
            "2@@Symmetric PAUSE;;"                                              \
            "3@@Both Symmetric PAUSE and Asymmetric PAUSE toward local device." \
        },                                                                      \
        {                                                                       \
            "REMOTE_FAULT",                                                     \
            "RO",                                                               \
            "13:13",                                                            \
            "1@@Link partner has detected remote fault;;"                       \
            "0@@Link partner has not detected remote fault."                    \
        },                                                                      \
        {                                                                       \
            "ACKNOWLEDGE",                                                      \
            "RO",                                                               \
            "14:14",                                                            \
            "1@@Link partner has received link code word;;"                     \
            "0@@Link partner has not received link code word."                  \
        },                                                                      \
        {NULL, NULL, NULL},                                                     \
    }                                                                           \
}

#define REG_DETAIL__COPPER_AN_EXPANSION                                                                              \
{                                                                                                                    \
    __REG_TYPE_MDIO,                                                                                                 \
    6,                                                                                                               \
    "COPPER_AN_EXPANSION",                                                                                           \
    __REG_PARSE_ATTR_COPPER,                                                                                         \
    NULL,                                                                                                            \
    {                                                                                                                \
        {                                                                                                            \
            "LP_AN_ABILITY",                                                                                         \
            "RO",                                                                                                    \
            "0:0",                                                                                                   \
            "1@@Capable;;0@@Not capable."                                                                            \
        },                                                                                                           \
        {                                                                                                            \
            "NP_RECEIVED",                                                                                           \
            "RO",                                                                                                    \
            "1:1",                                                                                                   \
            "1@@New page has been received from link partner;;0@@New page has not been received."                    \
        },                                                                                                           \
        {                                                                                                            \
            "NP_CAPABLE",                                                                                            \
            "RO",                                                                                                    \
            "2:2",                                                                                                   \
            "1@@PHY is Next Page capable;;0@@PHY is not Next Page capable."                                          \
        },                                                                                                           \
        {                                                                                                            \
            "LP_NEXT_PAGE_ABILITY",                                                                                  \
            "RO",                                                                                                    \
            "3:3",                                                                                                   \
            "1@@Link partner has Next Page capability;;0@@Link partner does not have Next Page capability."          \
        },                                                                                                           \
        {                                                                                                            \
            "PARALLEL_DETECTION_FAULT",                                                                              \
            "RO",                                                                                                    \
            "4:4",                                                                                                   \
            "1@@Parallel detection fault detected(Latched until read);;0@@Parallel detection fault not detected."    \
        },                                                                                                           \
        {                                                                                                            \
            "NP_RECEIVE_LOCATION",                                                                                   \
            "R/W",                                                                                                   \
            "5:5",                                                                                                   \
            "1@@Next Pages stored in Register 0x08;;0@@Next Pages stored in Register 0x05."                          \
        },                                                                                                           \
        {                                                                                                            \
            "NP_RECEIVE_LOCATION_ABLE",                                                                              \
            "R/W",                                                                                                   \
            "5:5",                                                                                                   \
            "1@@Bit-5 determines Next Page receive location;;0@@Bit-5 does not determine Next Page receive location" \
        },                                                                                                           \
        {NULL, NULL, NULL},                                                                                          \
    }                                                                                                                \
}

#define REG_DETAIL__1000BASET_CONTROL                                                                                     \
{                                                                                                                         \
    __REG_TYPE_MDIO,                                                                                                      \
    9,                                                                                                                    \
    "1000BASET_CONTROL",                                                                                                  \
    __REG_PARSE_ATTR_COPPER,                                                                                              \
    NULL,                                                                                                                 \
    {                                                                                                                     \
        {                                                                                                                 \
            "1000BASE-T_HD_CAP",                                                                                          \
            "R/W",                                                                                                        \
            "8:8",                                                                                                        \
            "1@@Advertise 1000BASE-T half-duplex capability;;0@@Advertise no 1000BASE-T half-duplex capability"           \
        },                                                                                                                \
        {                                                                                                                 \
            "1000BASE-T_FD_CAP",                                                                                          \
            "R/W",                                                                                                        \
            "9:9",                                                                                                        \
            "1@@Advertise 1000BASE-T full-duplex capability;;0@@Advertise no 1000BASE-T full-duplex capability"           \
        },                                                                                                                \
        {                                                                                                                 \
            "REPEATER_DTE",                                                                                               \
            "R/W",                                                                                                        \
            "10:10",                                                                                                      \
            "1@@Repeater/switch device port(see doc);;0@@DTE deivce(see doc)"                                             \
        },                                                                                                                \
        {                                                                                                                 \
            "MS_CONFIG_VALUE",                                                                                            \
            "R/W",                                                                                                        \
            "11:11",                                                                                                      \
            "1@@Config PHY as master;;0@@Config PHY as slave."                                                            \
        },                                                                                                                \
        {                                                                                                                 \
            "MS_CONFIG_EN",                                                                                               \
            "R/W",                                                                                                        \
            "12:12",                                                                                                      \
            "1@@Enable Master/Slave manual configuration value.;;0@@Automatic Master/Slave configuration."                \
        },                                                                                                                \
        {                                                                                                                 \
            "TEST_MODE",                                                                                                  \
            "R/W",                                                                                                        \
            "15:13",                                                                                                      \
            "0@@Normal operation;;1@@Test mode 1: Transmit Waveform Test;;2@@Test mode 2: Master Transmit Jitter Test.;;" \
            "3@@Test mode 3: Slave Transmit Jitter Test;;4@@Test mode 4: Transmitter Distortion Test;;"                   \
            "5@@Test mode 4: Transmitter Distortion Test;;6@@Test mode 4: Transmitter Distortion Test;;"                  \
            "7@@Test mode 4: Transmitter Distortion Test"                                                                 \
        },                                                                                                                \
        {NULL, NULL, NULL},                                                                                               \
    }                                                                                                                     \
}

#define REG_DETAIL__1000BASE_T_STATUS                                                                                \
{                                                                                                                    \
    __REG_TYPE_MDIO,                                                                                                 \
    0xA,                                                                                                             \
    "1000BASE-T_STATUS",                                                                                             \
    __REG_PARSE_ATTR_COPPER,                                                                                         \
    NULL,                                                                                                            \
    {                                                                                                                \
        {                                                                                                            \
            "IDLE_ERROR_COUNT",                                                                                      \
            "RO CR",                                                                                                 \
            "7:0",                                                                                                   \
            "0@@No error;;"                                                                                          \
        },                                                                                                           \
        {                                                                                                            \
            "LP_1000BASE-T_FD_CAP",                                                                                  \
            "RO",                                                                                                    \
            "10:10",                                                                                                 \
            "1@@Link partner is 1000BASE-T half-duplex capable;;0@@Link partner not 1000BASE-T half-duplex capable." \
        },                                                                                                           \
        {                                                                                                            \
            "LP_1000BASE-T_FD_CAP",                                                                                  \
            "RO",                                                                                                    \
            "11:11",                                                                                                 \
            "1@@Link partner is 1000BASE-T full-duplex capable;;0@@Link partner not 1000BASE-T full-duplex capable." \
        },                                                                                                           \
        {                                                                                                            \
            "REMOTE_RECEIVER_STATUS",                                                                                \
            "RO",                                                                                                    \
            "12:12",                                                                                                 \
            "1@@Remote receiver status is good;;0@@Remote receiver status is not good."                              \
        },                                                                                                           \
        {                                                                                                            \
            "LOCAL_RECEIVER_STATUS",                                                                                 \
            "RO",                                                                                                    \
            "13:13",                                                                                                 \
            "1@@Local receiver status is good.;;0@@Local receiver status is not good."                               \
        },                                                                                                           \
        {                                                                                                            \
            "MS_CONFIG_RESOLUTION",                                                                                  \
            "RO",                                                                                                    \
            "14:14",                                                                                                 \
            "1@@Local transmitter is master;;0@@Local transmitter is slave."                                         \
        },                                                                                                           \
        {                                                                                                            \
            "MS_CONFIG_FAULT",                                                                                       \
            "RO",                                                                                                    \
            "15:15",                                                                                                 \
            "1@@Master/Slave configuration fault detected;;0@@No Master/Slave configuration fault detected"          \
        },                                                                                                           \
        {NULL, NULL, NULL},                                                                                          \
    }                                                                                                                \
}

#define REG_DETAIL__IEEE_EXTENDED_STATUS                                                 \
{                                                                                        \
    __REG_TYPE_MDIO,                                                                     \
    0xF,                                                                                 \
    "IEEE_EXTENDED_STATUS",                                                              \
    __REG_PARSE_ATTR_COPPER,                                                             \
    NULL,                                                                                \
    {                                                                                    \
        {                                                                                \
            "1000BASE-T_HD_CAP",                                                         \
            "RO",                                                                        \
            "12:12",                                                                     \
            "1@@1000BASE-T half-duplex capable.;;0@@Not 1000BASE-T half-duplex capable." \
        },                                                                               \
        {                                                                                \
            "1000BASE-T_FD_CAP",                                                         \
            "RO",                                                                        \
            "13:13",                                                                     \
            "1@@1000BASE-T full-duplex capable.;;0@@Not 1000BASE-T full-duplex capable." \
        },                                                                               \
        {                                                                                \
            "1000BASE-X_HD_CAP",                                                         \
            "RO",                                                                        \
            "14:14",                                                                     \
            "1@@1000BASE-X half-duplex capable.;;0@@Not 1000BASE-X half-duplex capable." \
        },                                                                               \
        {                                                                                \
            "1000BASE-X_FD_CAP",                                                         \
            "RO",                                                                        \
            "15:15",                                                                     \
            "1@@1000BASE-X full-duplex capable.;;0@@Not 1000BASE-X full-duplex capable." \
        },                                                                               \
        {NULL, NULL, NULL},                                                              \
    }                                                                                    \
}

#define REG_DETAIL__FIBER_CONTROL                                                          \
{                                                                                          \
    __REG_TYPE_MDIO,                                                                       \
    0x0,                                                                                   \
    "FIBER_CONTROL",                                                                       \
    __REG_PARSE_ATTR_FIBER,                                                                \
    NULL,                                                                                  \
    {                                                                                      \
        {                                                                                  \
            "UNIDIRECTIONAL_EN",                                                           \
            "R/W",                                                                         \
            "5:5",                                                                         \
            "1@@Enable Unidirectional mode for 1000BASE-X or 100BASE-FX(See doc);;"        \
            "0@@Disable Unidirectional mode"                                               \
        },                                                                                 \
        {                                                                                  \
            "SGMII-Slave speed",                                                           \
            "R/W",                                                                         \
            "6:6,13:13",                                                                   \
            "0@@SGMII slave 10Mb/s;;1@@SGMII slave 100Mb/s;;2@@SGMII slave 1000Mb/s."      \
        },                                                                                 \
        {                                                                                  \
            "DUPLEX",                                                                      \
            "R/W",                                                                         \
            "8:8",                                                                         \
            "1@@Full duplex;;0@@Half duplex."                                              \
        },                                                                                 \
        {                                                                                  \
            "RESTART_AN",                                                                  \
            "R/W",                                                                         \
            "9:9",                                                                         \
            "1@@Restart auto-negotiation.;;0@@Auto negotiation restart complete."          \
        },                                                                                 \
        {                                                                                  \
            "POWER_DOWN",                                                                  \
            "R/W",                                                                         \
            "11:11",                                                                       \
            "1@@PHY is in low-power standby mode, "                                        \
            "Fiber interface is powered down, twisted pair interface remains powered-up;;" \
            "0@@Normal operation.;;"                                                       \
        },                                                                                 \
        {                                                                                  \
            "1000BASE-X_AN_ENABLE",                                                        \
            "R/W",                                                                         \
            "12:12",                                                                       \
            "1@@Enabled;;0@@Disabled."                                                     \
        },                                                                                 \
        {                                                                                  \
            "LOOPBACK",                                                                    \
            "R/W",                                                                         \
            "14:14",                                                                       \
            "1@@Enabled.;;0@@Normal operation."                                            \
        },                                                                                 \
        {NULL, NULL, NULL},                                                                \
    }                                                                                      \
}

#define REG_DETAIL__1000BASE_X_STATUS         \
{                                             \
    __REG_TYPE_MDIO,                          \
    0x1,                                      \
    "1000BASE-X_STATUS",                      \
    __REG_PARSE_ATTR_FIBER,                   \
    NULL,                                     \
    {                                         \
        {                                     \
            "1000BASE-X_LINK_ STATUS",        \
            "RO",                             \
            "2:2",                            \
            "1@@Link is up;;0@@Link is down." \
        },                                    \
        {                                     \
            "1000BASE-X_AN_COMPLETE",         \
            "RO",                             \
            "5:5",                            \
            "1@@Complete.;;0@@In progress."   \
        },                                    \
        {NULL, NULL, NULL},                   \
    }                                         \
}

#define REG_DETAIL__1000BASE_X_AN_ADVERTISEMENT                                    \
{                                                                                  \
    __REG_TYPE_MDIO,                                                               \
    0x4,                                                                           \
    "1000BASE-X_AN_ADVERTISEMENT",                                                 \
    __REG_PARSE_ATTR_FIBER,                                                        \
    NULL,                                                                          \
    {                                                                              \
        {                                                                          \
            "FULL_DUPLEX",                                                         \
            "R/W",                                                                 \
            "5:5",                                                                 \
            "1@@Advertise full duplex;;0@@Do not advertise full duplex."           \
        },                                                                         \
        {                                                                          \
            "HALF_DUPLEX",                                                         \
            "R/W",                                                                 \
            "6:6",                                                                 \
            "1@@Advertise half duplex;;0@@Do not advertise half duplex."           \
        },                                                                         \
        {                                                                          \
            "PAUSE",                                                               \
            "R/W",                                                                 \
            "8:7",                                                                 \
            "0x0@@No pause.;;"                                                     \
            "0x1@@Asymmetric pause toward link partner.;;"                         \
            "0x2@@Symmetric pause.;;"                                              \
            "0x3@@Both symmetric pause and asymmetric pause toward local device;;" \
        },                                                                         \
                                                                                   \
        {                                                                          \
            "REMOTE_FAULT",                                                        \
            "R/W",                                                                 \
            "13:12",                                                               \
            "0x0@@No remote fault;;"                                               \
            "0x1@@Link failure;;"                                                  \
            "0x2@@Off line;;"                                                      \
            "0x3@@Auto-negotiation error;;"                                        \
        },                                                                         \
        {                                                                          \
            "NEXT_PAGE",                                                           \
            "R/W",                                                                 \
            "15:15",                                                               \
            "1@@advertise next page ability.;;"                                    \
            "0@@next page ability not supported;;"                                 \
        },                                                                         \
        {NULL, NULL, NULL},                                                        \
    }                                                                              \
}

#define REG_DETAIL__1000BASE_X_AN_LINK_PARTNER_ABILITY                                             \
{                                                                                                  \
    __REG_TYPE_MDIO,                                                                               \
    0x5,                                                                                           \
    "1000BASE-X_AN_LINK_PARTNER_ABILITY",                                                          \
    __REG_PARSE_ATTR_FIBER,                                                                        \
    NULL,                                                                                          \
    {                                                                                              \
        {                                                                                          \
            "FULL_DUPLEX",                                                                         \
            "RO",                                                                                  \
            "5:5",                                                                                 \
            "1@@Link Partner is full-duplex capable.;;"                                            \
            "0@@Link Partner is not full-duplex capable;;"                                         \
        },                                                                                         \
        {                                                                                          \
            "HALF_DUPLEX",                                                                         \
            "RO",                                                                                  \
            "6:6",                                                                                 \
            "1@@Link Partner is half-duplex capable.;;"                                            \
            "0@@Link Partner is not half-duplex capable;;"                                         \
        },                                                                                         \
        {                                                                                          \
            "PAUSE",                                                                               \
            "RO",                                                                                  \
            "8:7",                                                                                 \
            "0@@Link Partner sends no pause.;;"                                                    \
            "1@@Link Partner sends asymmetric pause toward link partner.;;"                        \
            "2@@Link Partner sends symmetric pause.;;"                                             \
            "3@@Link Partner sends both symmetric pause and asymmetric pausetoward local device;;" \
        },                                                                                         \
                                                                                                   \
        {                                                                                          \
            "REMOTE_FAULT",                                                                        \
            "RO",                                                                                  \
            "13:12",                                                                               \
            "0x0@@No remote fault;;"                                                               \
            "0x1@@Link failure;;"                                                                  \
            "0x2@@Off line;;"                                                                      \
            "0x3@@Auto-negotiation error;;"                                                        \
        },                                                                                         \
        {                                                                                          \
            "ACKNOWLEDGE",                                                                         \
            "RO",                                                                                  \
            "14:14",                                                                               \
            "1@@Link partner has received link code word.;;"                                       \
            "0@@Link partner has not received link code word;;"                                    \
        },                                                                                         \
        {                                                                                          \
            "NEXT_PAGE",                                                                           \
            "RO",                                                                                  \
            "15:15",                                                                               \
            "1@@Link Partner supports Next Page ability.;;"                                        \
            "0@@Link Partner does not support Next Page ability;;"                                 \
        },                                                                                         \
        {NULL, NULL, NULL},                                                                        \
    }                                                                                              \
}

#endif
