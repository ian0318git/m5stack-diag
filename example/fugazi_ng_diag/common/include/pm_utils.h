/* $Id: pm_utils.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/pm_utils.h,v $
 *------------------------------------------------------------------
 *
 * Copyright (c) 2012 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Author: Anh Dang
 *------------------------------------------------------------------
 */

/* *------------------------------------------------------------------
 * mars_pm_utils.h
 * Author: Haidung Nguyen
 *------------------------------------------------------------------
 * Revision 1.10  1998/01/28 00:53:32  arwong
 * Add support for Emmett T1DSU WIC
 *
 * Revision 1.9  1997/08/28 16:18:49  dturner
 * Add submenu option to 4- & 8-channel A/S Port Module diags, and to
 * 16- & 32-channel async PM diags.
 *
 * Revision 1.8  1997/08/21 18:13:01  dturner
 * o Include changes to mbri & pri diags by Dale Mize, which, among other
 *   things, facilitate adding submenu capability.
 * o Add submenu capability to the combo board diags.
 * o Calls to msleep(500) moved out of cleanup functions, which hang when
 *   invoked during a break interrupt.
 *
 * Revision 1.7  1997/06/11 21:15:26  aosulliv
 * Added return value to config_pci_controller.
 *
 * Revision 1.6  1997/04/17 19:06:01  fliu
 * Added Mars FE code.
 *
 * Revision 1.5  1997/02/14 04:45:46  sanjeevk
 * Add extern manbri_test() in mars_pm_utils.h
 * Add manbri_test in the table of mars_pm_utils.c file to support Manage Bri..
 *
 * Revision 1.4  1996/12/13 23:27:14  rllewis
 * Add support for the Adtran Sw56 WIC.
 *
 * Revision 1.3  1996/12/10 18:05:19  haidung
 * Fix wic_port_tbl data structure.
 *
 * Revision 1.2  1996/11/05 00:59:06  haidung
 * Add/update Mars PM tests.
 *
 * Revision 1.1  1996/09/10  10:11:07  clev
 * Bring DiagMon for MARs into the world.
 *
 */

typedef struct wan_ds wan_ds_t;
struct wan_ds {
    int         slot;
    ulong       mem_start;
    uchar       *dpr;
    volatile ushort int_flag;
    volatile ushort scc_int_status;
};

/*
 * Combo Wan Interface Card defines.
 */


typedef struct interface_param_t_ interface_t;
struct interface_param_t_ {
    ulong  wic_addr;               /* Wic Addr slot on Platform */
    ulong  nm_addr;                /* Addr of NM if wic installed */
    ulong  dpr;                    /* Quicc/Pquicc dpr          */
    PFT    wic_select;             /* Platform Routine to select/deselct Wic */
    PFT    wic_reset;              /* Platform Routine to reset Wic */
    PFT    put_wic_isr;            /* Platform Routine to setup int handlers */
    PFT    isr_control;            /* Enable/Disable Interrupts   */
    PFT    platform_isr;           /* Platform dependent ISR routine */
    char   *platform_tn;
    int slot;                      
    int port;
    int channel_size;
    int speed;
    int id;
    int daughter_id;
    int menu_display;
    int sys_type;                  /* used by wic_dsu_v2 test only */
    int int_lvl;                   /* Interrupt level */
    PFL test_func;                 /* Pointer to platform specific function */
    int prog_type;                 /* 0=legacy, 1 = new for SHDSL */
    void *hwic_if;                 /* for SHDSL */
    ulong  cable_dpr;              /* Snowshoe cable data stuct ptr */
};
typedef long (*PFLONG)(interface_t *); 

/*
 * This structure is used to map the interface id to the name and
 * diagnostic routine.
 */

struct wic_info {
    char   *name;               /* interface name */
    PFT  wic_diag;            /* the diagnostic for this interface */
    ushort id;                  /* the interface id */
};

struct wic_port_data {
    ushort id;                  /* interface id */ 
    char  *name;                /* interface name */
    PFT diag;                 /* the diagnostic for this port */
};

typedef struct vwic_daughter_eeprom_info_ {
    ushort id;
    char   *cookie_name;
    PFT    eeprom_instl;
    PFT    eeprom_rd;
    PFT    eeprom_wr;
} vwic_daughter_eeprom_info;

typedef struct quicc_ds_ {
    int slot;
    ulong base_addr;
    uchar *dpr;
    uchar *spi_tx_data;  /* Host point of view */
    uchar *spi_rx_data;
    uchar *tx_data;      /* Quicc point of view */
    uchar *rx_data;
    uchar *spi_tx_bd;    /* Host point of view */
    uchar *spi_rx_bd;
    uchar *rmd_scc1;
    uchar *tmd_scc1;
    uchar *rmd_scc2;
    uchar *tmd_scc2;
    uchar *rmd_scc3;
    uchar *tmd_scc3;
    uchar *rmd_scc4;
    uchar *tmd_scc4;
    uchar *rmd_scc;
    uchar *tmd_scc;
    uchar *bd_scc;
    uchar *scc_rx_bd;
    uchar *scc_tx_bd;
    uchar *scc1_rx_bd;
    uchar *scc1_tx_bd;
    uchar *scc2_rx_bd;
    uchar *scc2_tx_bd;
    uchar *scc3_rx_bd;
    uchar *scc3_tx_bd;
    uchar *scc4_rx_bd;
    uchar *scc4_tx_bd;
} quicc_ds_t;

extern quicc_ds_t *quicc_ds_ptr;
extern quicc_ds_t quicc_ds;

/*---------------------------------------------------------------------*/
/*  Quicc register defines                                             */
/*---------------------------------------------------------------------*/
#define QUICC_SERIAL_PSMR_SETTING      0x00
#define QUICC_SERIAL_REG_CLEAR         0xffff

#define QUICC_CP_RCCR           0x8000      /* enable risc timer */

#define QUICC_SDMA_ARB_ID       0x0070      /* Slave mode arbitration id */
#define QUICC_SDMA_ISM          0x0700      /* SDMA Int. Service mask */
#define QUICC_SDMA_ERR          0x0002      /* SDMA Bus error enable */
#define QUICC_SDMA_SLAVE_CFG    (QUICC_SDMA_ARB_ID | QUICC_SDMA_ISM) 

#define QUICC_SIM_MCR           0x00000cbf
#define QUICC_SIM_CLKOCR        0x8d        /* enable COM1, dis COM2 */
#define QUICC_SIM_SYPCR_MSK     0x7f
#define QUICC_SIM_DISABLE_WDT   0x70
#define QUICC_SIM_WDT_MASK      0x80
#define QUICC_SIM_PLLCR         0xe2f4
#define QUICC_SIM_RSR           0xff    /* clear all reset status */
#define QUICC_SIM_PEPAR         0x4037

#define QUICC_OR_EXDSACK        0xf0000006 /* use external DSACK */

/* SERIAL related parameters */
#define SERIAL_PRAM_C_PRES      0x0000FFFF; /* 16 bit CRC preset */
#define SERIAL_PRAM_C_MASK      0xF0B8;     /* 16 bit CRC */
#define SERIAL_SCC_RX_BD_IDX    0x10        /* RxBD for the Serial channel */
#define SERIAL_SCC_TX_BD_IDX    0x20        /* TxBD for the Serial channel */

/*
 * QUICC DPRAM area.
 */

#define USER_DATA_BEGIN      0
#define USER_DATA_END        0x6ff
#define PARAM1_BEGIN         0xc00
#define PARAM1_END           0xcbf 
#define PARAM2_BEGIN         0xd00
#define PARAM2_END           0xdbf
#define PARAM3_BEGIN         0xe00
#define PARAM3_END           0xebf
#define PARAM4_BEGIN         0xf00
#define PARAM4_END           0xfbf
#define MAXRANGE             10           /* number of dpram ranges */  

#define WAN_IDSEL            0  /* IDSEL of WAN PM */
#define PCIC_CMD_REG_DEFAULT 0x6

#define WIC_VACCODE     0xfe   /* port is vacant */
#define WIC_ILLCODE     0xfd   /* illegal port code */

#define MAX_WIC_PORT    2      /* 2 interface ports */
#define COMBO_IDSEL     0        /* IDSEL of combo PM */

/*
 * Define  combo  type
 */

#define COMBO_2E2W_ID    0x1e
#define COMBO_1E1R2W_ID  0x1f
#define COMBO_1E2W_ID    0x37

#define FECPM_0FE_ID        0xd6
#define FECPM_1FE_ID        0xd7
#define FECPM_2FE_ID        0xd8
#define FECPM_1FE1R_ID      0xd9
#define FECPM_1FE2W_V2_ID   0x0396
#define FECPM_2FE2W_V2_ID   0x0397
#define FECPM_1FE1R2W_V2_ID 0x0398

#define NOT_COMBO_TYPE   0
#define COMBO_PM_TYPE    1
#define FECPM_PM_TYPE    2

/*
 * Do Not change values of INT_FLAG_SCC1 to INT_FLAG_SCC4. The
 * flag number is also the scc number.
 */
#define DEFAULT              0
#define INT_FLAG_SCC1        1             /* scc 1 interrupt flag */
#define INT_FLAG_SCC2        2 
#define INT_FLAG_SCC3        3 
#define INT_FLAG_SCC4        4
#define INT_FLAG_SPI         5             /* spi interrupt flag */
#define INT_FLAG_SDMA        6             /* sdma interrupt flag */
#define INT_FLAG_TIMER       10            /* timer int occurred */

#define TIMER_CFG_TMR2       0x0012        /* timer clk source, scale */  
#define TIMER_START_COUNT    0             /* init timer counter */
#define TIMER_END_COUNT      0xa00         /* set timer of about 100us */
#define TIMER_2_ENABLE       0x0010        /* enable timer 2 */    
#define TIMER_2_INT_EN       0x00040000    /* enable timer 2 interrupt */

/*
 * WAN Module Interrupt Mask register.
 */

#define WAN_INT_MSK_OFFSET   (CPM_CS_OFFSET + 0x4)

#define WAN_INT_MASK         1
#define WAN_INT_DEFAULT      1
#define WAN_INT_DIAG_MASK    0x6
#define WAN_INT_DIAG_PAT1    0x2
#define WAN_INT_DIAG_PAT2    0x4

/*
 * WAN Module Interrupt Register.
 */

#define WAN_INT_REG_OFFSET   0x40008
#define WIC_PRESENT_MSK      0x30
#define BOTH_WIC_PRESENT     0
#define NO_WIC_PRESENT       (WIC_PRESENT_MSK)
#define ONLY_WIC0_PRESENT    0x20
#define ONLY_WIC1_PRESENT    0x10

/*
 * Addresses offset from the beginning of PM memory space.
 */

#define QUICC_DPRAM_OFFSET   0x000010000 /* Quicc dprambase offset */ 
#define QUICC_MBAR_OFFSET    0x00003ff00 /* Quicc mbar reg */
#define QUICC_DPRAM_OFFSET   0x000010000 /* Quicc dprambase offset */ 
#define CPM_CS_OFFSET        0x000040000 /* Combo PM select offset */
#define WAN0_OFFSET          0x000080000 /* WAN 0 PM select offset */
#define WAN1_OFFSET          0x0000c0000 /* WAN 1 PM select offset */
#define WAN_ADDR_SIZE        0x000040000

#define MARS_NTWK_INT_LVL    0           /* Mars network int at level 0 */
#define QUICC_CICR_INT_LVL   6           /* Quicc internal int level */ 
#define QUICC_CICR_INIT      ((QUICC_CICR_INT_LVL << 13) | 0x1f00 | 0x1b0000)

/*
 * 5in1 define.
 */

#define CPAI_IF_OFFSET       0x04        /* CPAI Interface Reg offset */
#define CPAI_IF0_OFFSET      0x04        /* CPAI Interface Reg offset */
#define CPAI_IF1_OFFSET      0x14        /* CPAI Interface Reg offset */
#define CPAI_MISC_OFFSET     0x08        /* CPAI Misc. Reg offset */
#define CPAI_MISC_MASK       0x06        /* Misc. register mask */
#define CPAI_MISC0_OFFSET    0x08
#define CPAI_MISC1_OFFSET    0x18

#define SENSE1_MASK          0x20        /* SENSE1 bit mask */
#define BLOOP_MASK           0x40        /* BLOOP bit mask */
#define T_CTRL_MASK          0x1f        /* T_CTRL[4:0] bit mask */

#define BLOOP_LPBK_ON        0           /* No local loopback */
#define BLOOP_LPBK_OFF       BLOOP_MASK  /* local loopback */

#define NORMAL               0             /* normal mode, no loopback */ 
#define SCC_LPBK             1             /* SCC loopback mode */
#define CPAI_LPBK            2             /* CPAI loop back mode */
#define POLL                 0             /* Poll mode */ 
#define INTERRUPT            1             /* Interrupt mode */

#define NO_CABLE             0x01
#define V_35DCE              0x05

/* WIC interrupt mask bits */
#define WIC_INT_MASK		0x1

/* WIC interrupt register bits */
#define DC1_CP			0x20		/* slot 1 WIC card present */
#define DC0_CP			0x10		/* slot 0 WIC card present */
#define DC1_IRQ1		0x8
#define DC1_IRQ0		0x4
#define DC0_IRQ1		0x2
#define DC0_IRQ0		0x1

/* WIC interrupt registers */
#define	WIC_INT_PEND_REG	0x40008
#define	WIC_INT_MASK_REG	0x40004

/*---------------------------------------------------------------------*/
/* Extern Variables & Functions                                        */
/*---------------------------------------------------------------------*/
extern unsigned long wic_serial_num[MAX_WIC_PORT];
extern char wic_serial_num_ascii[MAX_WIC_PORT][12];
extern ushort wic_board_rev[MAX_WIC_PORT];
extern struct wic_port_data wic_port_tbl[MAX_WIC_PORT];
extern boolean is_cookie_4[MAX_WIC_PORT];

extern void   reset_io(int);
extern int    slot_start_with(void);
extern int    get_real_slot(int);
extern int    test_1e_combo_pm(int, ulong);
extern int    test_2e_combo_pm(int, ulong);
extern int    test_1e1r_combo_pm(int, ulong);
extern int    combo_board_test(int);
extern int    wan_test(interface_t *);
extern int    bri_test(interface_t *);
extern int    dsu_4w(interface_t *);
extern void   manbri_test(interface_t *);
extern int    amd79c970_test(interface_t *);
extern int    wic_t1_test(interface_t *);
extern int    scc_wic_t1e1_test(interface_t *);
extern int    tr_test(int);
extern int    quicc_scc_isr(void);
extern int    quicc_timer_isr(void);

extern int    config_pci_controller(int, uchar, ulong);
extern uchar  read_cpai_cable_id(uchar *);
extern void   set_cpai_loopbit(uchar *, int);
extern void   setup_pm_interrupt(uchar);
extern void   setup_pm_mgt_intr(uchar);
extern void   setup_oir_detection(void);

extern uchar  get_pci_bus_no(uint);
extern int    get_pci_dev_num(int, int);
extern void   alt_wic_eeprom(void);
extern void   alt_pas_eeprom(void);
extern int    wic_eeprom_cleanup(void);
extern volatile uchar *get_tstport(int, boolean);

/* wic_sw56.c */

extern volatile uchar *wan_adrs_reg;
extern volatile uchar *wan_data_reg;
extern volatile uchar *wan_port_stat_reg;
extern volatile uchar *wan_intr_pend_reg;
extern volatile uchar *wan_intr_mask_reg;
extern volatile uchar wan_loop_stat_unlatched;
extern volatile uchar wan_loop_stat_latched;

extern int wic_t1e1_rd_ecan(interface_t *iface, PFT, 
                            struct wic_port_data *);
extern int wic_t1e1_wr_ecan(interface_t *iface, PFT, 
                            struct wic_port_data *, void *);
extern vwic_daughter_eeprom_info *is_vwic_multi_cookie(ushort id);
extern int  wic_t1e1_ecan_instl(interface_t *iface); 
extern int identify_lpbk_cable_id(uchar *if_reg_ptr);

/******** History ******** 
$Log: pm_utils.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
