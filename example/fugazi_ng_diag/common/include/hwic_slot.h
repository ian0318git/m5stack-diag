/* $Id: hwic_slot.h,v 1.4 2013/10/07 03:40:13 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/hwic_slot.h,v $
 *-----------------------------------------------------------------------------
 * hwic_slot.h - Contains defines for hwic slots. 
 *
 * Jan. 2003 Alan O'Sullivan
 *
 * Copyright (c) 2008-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef __HWIC_SLOT_H_
#define __HWIC_SLOT_H_

#ifndef LINUX_KLM

/*
 * This structure is used to map the HWIC id to the name and
 * diagnostic routine.
 */
struct hwic_slot {
    char    *name;            /* HWIC name                    */
    PFT     diag;             /* the diagnostic for this HWIC */
    ushort  id;               /* the HWIC cookie id           */
    PFT     diag_iface;       /* the interface test for this HWIC */
};

/*
 * Data Structure for the HWIC interface.
 */
typedef struct hwic_interface_param_t_ hwic_iface_t;
struct hwic_interface_param_t_ {
    int    slot;                 /* HWIC Slot Number on Platform        */
    int    port;                 /* PORT ON WIC                         */
    ulong  base_addr;            /* HWIC Addr slot on Platform          */
    ushort cookie_id;            /* HWIC Cookie id                      */
    int    menu_display;         /* Flag set if submenu is invoked      */
    uchar  mode;                 /* HWIC Mode, NMSI, TDM ....           */
    PFV    netio_isr;            /* HWIC netio interrupt ISR            */
    PFV    mgmt_isr;             /* HWIC management interrupt ISR       */
    PFV    err_isr;              /* HWIC error interrupt ISR            */
    int    test_type;            /* Full test or HWIC interface test 
				    Initialize with FULL_TEST or IFACE_TEST*/
    int    prog_type;            /* bit 0 for legacy/new gen         
                                    bit 1 for Kaena/non-kaena           */
    int    protocol;            
    boolean cli;                 /* CLI parameter mode */
    int cmd_id;   /* what type of asic test (ie, hwic_ilpbk, common_intr_test, 
		     or hwic test */
    int subtest;  /*if hwic test, what type of hwic test */

};

#endif /* linux_klm */
/*
 * Defines for prog_type, FIO on SM (previously NM)
 */
#define FIO_ON_NM1              0x00000100
#define FIO_ON_NM2              0x00000200
#define FIO_ON_NM3              0x00000300
#define FIO_ON_NM4              0x00000400
#define FIO_ON_NM_MASK          0x00000700
#define FIO_ON_NM_SHIFT         8

/*
 * EHWIC reference
 */
typedef enum {
    EHWIC_REF_100M = 0,		/* 100 MHz */
    EHWIC_REF_200M,		/* 200 MHz */
} ehwic_ref_clk;

/*
 * Defines used to support HWIC code.
 */
#define NO_HWIC_PRESENT          0xFE

#ifndef OVERLORD
#ifdef INFORMERS
#define NUM_HWIC_SLOTS           3	/* informers supprts only 3 */
#else
#define NUM_HWIC_SLOTS           4	/* informers supprts only 3 */
#endif /* INFORMERS */
#endif

/*
 * Defines used to support the different HWIC Loopback Modes.
 */
#define  MPSC_INTERNAL_LOOPBACK  0x01
#define  MPSC_EXTERNAL_LOOPBACK  0x02
#define  FIO_INTERNAL_LOOPBACK   0x03

/*
 * Defines used to support the different HWIC Modes.
 */
#define  HWIC_MODE_NMSI          0x01
#define  HWIC_MODE_TDM           0x02
#define  HWIC_MODE_NMSI_DTE      0x03
#define  HWIC_MODE_NMSI_ASYNC    0x04 /* e.g Modi */

/*
 * Defines used to support the different HWIC Types.
 */
#define  WIC_TYPE_NONE           0x00
#define  WIC_TYPE_OLD            0x01
#define  WIC_TYPE_VOICE          0x02
/* differentiate between HWIC BRI and WIC BRI */
#define  WIC_TYPE_HWIC           0x03

/*
 * Defines used to support the different MPSC Protocols.
 */
#define  MPSC_PROTOCOL_HDLC      0x01
#define  MPSC_PROTOCOL_UART      0x02
#define  MPSC_PROTOCOL_BISYNC    0x03

/*
 * Generic defines.
 */
#define PORT_A                   0x01
#define PORT_B                   0x02

#define MPSC_PORTA               0x0000000F  /* Swap it */
#define MPSC_PORTB               0x000000F0  /* Swap it */
#define MPSC_PORTC               0x00000F00  /* Swap it */
#define MPSC_PORTD               0x0000F000  /* Swap it */
#define MPSC_PORTE               0x000F0000  /* Swap it */
#define MPSC_PORTF               0x00F00000  /* Swap it */
#define MPSC_PORTG               0x0F000000  /* Swap it */
#define MPSC_PORTH               0xF0000000  /* Swap it */

#define FLEXTDM_0                0x00000001  /* Swap it */
#define FLEXTDM_0_CLEAR          0x00000007  /* Swap it */
#define FLEXTDM_1                0x00000010  /* Swap it */
#define FLEXTDM_1_CLEAR          0x00000070  /* Swap it */
#define FLEXTDM_2                0x00000300  /* Swap it */
#define FLEXTDM_2_CLEAR          0x00000700  /* Swap it */
#define FLEXTDM_3                0x00003000  /* Swap it */
#define FLEXTDM_3_CLEAR          0x00007000  /* Swap it */
#define FLEXTDM_4                0x00090000  /* Swap it */
#define FLEXTDM_4_CLEAR          0x00070000  /* Swap it */
#define FLEXTDM_5                0x00900000  /* Swap it */
#define FLEXTDM_5_CLEAR          0x00700000  /* Swap it */
#define FLEXTDM_6                0x0B000000  /* Swap it */
#define FLEXTDM_6_CLEAR          0x07000000  /* Swap it */
#define FLEXTDM_7                0xB0000000  /* Swap it */
#define FLEXTDM_7_CLEAR          0x70000000  /* Swap it */

#define DISCONNECT_ALL_ROUTES    0x77777777

#define MPSC_NUM0                0
#define MPSC_NUM1                1
#define MPSC_NUM2                2
#define MPSC_NUM3                3
#define MPSC_NUM4                4
#define MPSC_NUM5                5
#define MPSC_NUM6                6
#define MPSC_NUM7                7

#define HWIC_SLOT0		 0
#define HWIC_SLOT1  	         1
#define HWIC_SLOT2     	         2
#define HWIC_SLOT3     	         3

#define BRG0_NUM                 0

#define MPSC0_PORT_CLOCK         0x2

#define NUM_NTWK_INTS		 4
#define NUM_MGMT_INTS		 4
#define NUM_ERR_INTS		 4
#define HWIC_NETIO_INT		 1
#define HWIC_MGMT_INT		 2
#define HWIC_ERR_INT		 4

#define CS1_0_CS0_0	    0x0 /* DEPENDS ON CARD */
#define CS1_0_CS0_1         0x1 /* INACTIVE MODE   */
#define CS1_1_CS0_0         0x2 /* DEPENDS ON CARD */
#define CS1_1_CS0_1         0x3 /* USED FOR COOKIE */

#ifndef LINUX_KLM
/*
 * Function prototypes for hwic interface.
 */
extern boolean is_hwic_present(int, uchar *);
extern void *get_hwic_base_address(int);
extern int  get_real_hwic_slot(int);
extern void enable_hwic_slot(int, boolean);
extern void hwic_chip_select(int, uchar);
extern int  hwic_neti_handler(int);
extern int  hwic_mgmt_handler(int);
extern int  hwic_err_handler(int);
extern void setup_fio_hwic_intr(int, int, boolean);
extern void setup_hwic_netio_interrupt(hwic_iface_t *, boolean);
extern void setup_hwic_mgmt_interrupt(hwic_iface_t *, boolean);
extern void setup_hwic_err_interrupt(hwic_iface_t *, boolean);
extern void enable_hwic_pin65(int, boolean);
extern void platform_hwic_control_interrupt(hwic_iface_t *hwic_ds, int enable);
extern int plat_mb_slot_is_hwic (int slot);
extern int  plat_realnum_to_chassisnum(int real_num); 
extern int  hwic_slot_start_with(void);
extern int api_set_fio_bri_tdm_te (hwic_iface_t *hwic_ds, int port);
extern void unex_hwic_netio_isr(hwic_iface_t *hwic_iface);
extern int cookie_format_status(uchar *, int , int *, int );

extern struct hwic_slot *get_hwic_table_index (int, void *, ushort *, uchar);
extern struct hwic_slot *get_hwic_table_ptr(void);
extern ushort get_hwic_table_size(void);
extern type_t hwic_notavail(void);
extern type_t hwic_vacant(void);
extern type_t wic_iface_test(void);

extern void goofy_wic_man_intr_hndlr (hwic_iface_t *hwic_iface, int hwic_num);
extern void goofy_wic_net_intr_hndlr (hwic_iface_t *hwic_iface, int hwic_num);
extern void goofy_wic_err_intr_hndlr (hwic_iface_t *hwic_iface, int hwic_num);
extern int set_ehwic_ref_clock(uint speed);

extern hwic_iface_t hwic_params[], *hwic_iface[];

#endif /* linux_klm */

#endif /* __HWIC_SLOT_H_ */
/* ------ End of Module ------ */

/******** History ******** 
$Log: hwic_slot.h,v $
Revision 1.4  2013/10/07 03:40:13  alpeng
introducing a flag early_unreset for ngio to put reset state on early stage

Revision 1.3  2013/09/27 00:34:22  mcharon
add early_unreset variable to allow host code to immeidately bring module out of reset

Revision 1.2  2012/03/28 00:38:11  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
