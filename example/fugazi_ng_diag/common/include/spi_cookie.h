/* $Id: spi_cookie.h,v 1.2 2012/03/28 00:38:12 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/spi_cookie.h,v $
 *------------------------------------------------------------------
 * spi_cookie.h - SPI cookie interface header file
 *
 * Mar. 2002, 	Alan Hsu
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

/* start bit[8] | opcode field[7~6] | address field [5~0] */
#define SPI_EEPROM_READ       0x00000180
#define SPI_EEPROM_WRITE      0x00000140
#define SPI_EEPROM_WEN        0x00000130
#define SPI_EEPROM_WDS        0x00000100

#define SPI_WAIT              0x400

/*
 * Bit definitions for WIC SPI Control register ( 0x1FB0 0006 ) 
 */
#define SPI_CLK_2600KHZ       0x0030
#define SPI_CLK_1300KHZ       0x0020
#define SPI_CLK_650KHZ        0x0010
#define SPI_CLK_325KHZ        0x0000
/* Moved from sys_regs.h */
#if defined(GIOVE)
#define SPI_OP_DONE           0x0400
#define SPI_OP_START          0x0200
#define SPI_WRT_WIC_DATA      0x0100
#define SPI_RD_WIC_DATA       0x0000
#else
#define SPI_OP_DONE           0x0004
#define SPI_OP_START          0x0002
#define SPI_WRT_WIC_DATA      0x0001
#define SPI_RD_WIC_DATA       0x0000
#endif

/* bit defs for fio local bus offset 0x2c */
#define WIC0_SCS_REG_CS0      0x1 
#define WIC0_SCS_REG_CS1      0x2
#define WIC1_SCS_REG_CS0      0x4
#define WIC1_SCS_REG_CS1      0x8
#define WIC2_SCS_REG_CS0      0x10
#define WIC2_SCS_REG_CS1      0x20
#define WIC3_SCS_REG_CS0      0x40
#define WIC3_SCS_REG_CS1      0x80

/* Quack Test Bus Timing (in micro second) */
#define QUACK_TSTBUS_CS_TO_CLK_DELAY_MIN       100 //in us
#define QUACK_TSTBUS_CLK_TO_CS_DELAY_MIN        50
#define QUACK_TSTBUS_CS_DELAY_BETWEEN_CYCLES   260

/* ************************************************************ *
 * NOTE: the cookie_select_reg and spi_cntl_reg registers could
 *       point to the same register
 * ************************************************************ */
typedef struct spi_ushort_interface_ds {
					/* for two bits cookie access */
    volatile ushort *cookie_select_reg;
    ushort enable_select_cookie_bits;	/* 00b, 10b, 11b */
    ushort disable_select_cookie_bits;  /* 01b */
    ushort select_cookie_mask;		/* 11b */
    ushort disable_all_cookie_mask;	/* cookie 0, cookie 1, cookie 2, ... */ 
    					/*   01b   |  01b   |    01b   | ... */ 
    volatile ushort *spi_cntl_reg;
    ushort spi_clock_select_bits;
    ushort spi_clock_select_mask;
    ushort spi_complete_bit;
    ushort spi_start_bit;
    volatile uint *spi_status_reg;
    volatile ushort *spi_data_port;

    uint  eeprom_size_in_byte;
    uchar *eeprom;

} spi_ushort_interface_t;

typedef struct spi_uchar_interface_ds {
					/* for two bits cookie access */
    uchar *cookie_select_reg;
    uchar enable_select_cookie_bits;	/* 00b, 10b, 11b */
    uchar disable_select_cookie_bits;	/* 01b */
    uchar select_cookie_mask;            /* 11b */
    uchar disable_all_cookie_mask;	/* cookie 0, cookie 1, cookie 2, ... */
                                        /*   01b   |  01b   |    01b   | ... */
    uchar *spi_cntl_reg;
    uchar spi_clock_select_bits;
    uchar spi_clock_select_mask;
    uchar spi_complete_bit;
    uchar spi_start_bit;

    volatile ushort *spi_data_port;

    uint  eeprom_size_in_byte;
    uchar *eeprom;

} spi_uchar_interface_t; 

typedef struct spi_uint_interface_ds {
                                        /* for two bits cookie access */
    volatile uint *cookie_select_reg;
    uint enable_select_cookie_bits;     /* 00b, 10b, 11b */
    uint disable_select_cookie_bits;    /* 01b */
    uint select_cookie_mask;            /* 11b */
    uint disable_all_cookie_mask;       /* cookie 0, cookie 1, cookie 2, ... */
                                        /*   01b   |  01b   |    01b   | ... */
    volatile uint *spi_cntl_reg;
    volatile uint *spi_status_reg;
    uint spi_clock_select_bits;
    uint spi_clock_select_mask;
    uint spi_complete_bit;
    uint spi_start_bit;

    volatile uint *spi_data_port;
    uint spi_data_port_mask;
    uint spi_data_port_offset;          /* staring bit position */

    uint  eeprom_size_in_byte;
    uchar *eeprom;
    uint  cntl_reg_shad;

} spi_uint_interface_t;

extern int  read_eeprom_data(int , void *, uchar *);
extern int  spi_ushort_read_eeprom(spi_ushort_interface_t *);
extern int  spi_ushort_write_eeprom(spi_ushort_interface_t *);
extern int  spi_uchar_read_eeprom(spi_uchar_interface_t *);
extern int  spi_uchar_write_eeprom(spi_uchar_interface_t *);
extern int  spi_uint_read_eeprom(spi_uint_interface_t *);
extern int  spi_uint_write_eeprom(spi_uint_interface_t *);
extern int  spi_uchar_rw_op(spi_uchar_interface_t *, ushort *, ushort *, uint);
extern int  spi_ushort_rw_op(spi_ushort_interface_t *, ushort *, 
			     ushort *, uint);
extern int  spi_uint_rw_op (spi_uint_interface_t* , ushort *, ushort *, uint);
extern void init_uint_spi_cookie_p (spi_uint_interface_t *, int ,
                          uint *, uint *, uint *, uchar *);
extern int spi_uint_read_eeprom_r2(spi_uint_interface_t *);
extern int spi_uint_write_eeprom_r2(spi_uint_interface_t *);




/******** History ******** 
$Log: spi_cookie.h,v $
Revision 1.2  2012/03/28 00:38:12  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:04  ptong
Initial archive of ng_diag module


$Endlog$
*/
