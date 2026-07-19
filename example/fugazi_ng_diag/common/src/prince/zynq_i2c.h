/* $Id: zynq_i2c.h,v 1.1 2013/04/19 07:17:53 xiaoyizh Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/prince/zynq_i2c.h,v $
 *
 * zynq_i2c.h - definitions for i2c driver
 *
 * liwwang -- Dec. 2012
 *
 * Copyright (c) 2012-2013 by Cisco Systems, Inc.
 * All rights reserved.
 *
 */

/* ZC702 test */
#define ZC702_I2C_ADDR             0x54
#define ZC702_I2C_MUX_ADDR         0x74
#define ZC702_EEPROM_START_ADDR    128
#define ZC702_MUX_BASE             0xE000A000
#define ZC702_MUX_LEN              0x400

/* ZYNQ I2C0 BASE ADDRESS */
#define ZYNQ_I2C0_BASE    0xE0004000
#define I2C0_MMAP_LEN     0x1000


/* I2C0 slave device address */
#define ZYNQ_I2C_ADDR_DS4424        0x30    /* Voltage Margin address >> 1 */

/* I2C0 Control register SCL speed set  1,23  */
#define ZYNQ_I2C_APB    105550000    /* Hz */
#define ZYNQ_I2C_SCL    99960    /* Hz */
#define ZYNQ_I2C_SCL_DIVA    1
#define ZYNQ_I2C_SCL_DIVB    23

/* Control register fields */

#define ZYNQ_I2C_CONTROL_RW                     0x00000001
#define ZYNQ_I2C_CONTROL_MS                     0x00000002
#define ZYNQ_I2C_CONTROL_NEA                    0x00000004
#define ZYNQ_I2C_CONTROL_ACKEN                  0x00000008
#define ZYNQ_I2C_CONTROL_HOLD                   0x00000010
#define ZYNQ_I2C_CONTROL_SLVMON                 0x00000020
#define ZYNQ_I2C_CONTROL_CLR_FIFO               0x00000040
#define ZYNQ_I2C_CONTROL_DIV_B_SHIFT            8
#define ZYNQ_I2C_CONTROL_DIV_B_MASK	        0x00003F00
#define ZYNQ_I2C_CONTROL_DIV_A_SHIFT            14
#define ZYNQ_I2C_CONTROL_DIV_A_MASK             0x0000C000
#define ZYNQ_I2C_CONTROL_RESET  0x00000000

/* Status register values */

#define	ZYNQ_I2C_STATUS_RXDV	0x00000020
#define	ZYNQ_I2C_STATUS_TXDV	0x00000040
#define	ZYNQ_I2C_STATUS_RXOVF	0x00000080
#define	ZYNQ_I2C_STATUS_BA	0x00000100

/* Interrupt register fields */
#define	ZYNQ_I2C_INTERRUPT_COMP		0x00000001
#define	ZYNQ_I2C_INTERRUPT_DATA		0x00000002
#define	ZYNQ_I2C_INTERRUPT_NACK		0x00000004
#define	ZYNQ_I2C_INTERRUPT_TO		0x00000008
#define	ZYNQ_I2C_INTERRUPT_SLVRDY	0x00000010
#define	ZYNQ_I2C_INTERRUPT_RXOVF	0x00000020
#define	ZYNQ_I2C_INTERRUPT_TXOVF	0x00000040
#define	ZYNQ_I2C_INTERRUPT_RXUNF	0x00000080
#define	ZYNQ_I2C_INTERRUPT_ARBLOST	0x00000200
#define	ZYNQ_I2C_INTERRUPT_MASK  	0x000002FF

/* Timeout register fields */
#define ZYNQ_I2C_TO_RESET               0x0000001F
/* Slave Monitor Pause Register */
#define ZYNQ_I2C_SLAVEMON_RESET         0x00000000
/* Data Register */
#define ZYNQ_I2C_DATA_RESET             0x00000000
/* Transfer size Register */
#define ZYNQ_I2C_TRANSIZE_RESET         0x00000000
/* Interrupt status Register */
#define ZYNQ_I2C_INTERRSTATUS_RESET     0x00000000

/* Role: read/write */
#define READ_ROLE    1
#define WRITE_ROLE   0

/* Number of bytes of Zync I2C FIFO */
#define ZYNQ_I2C_FIFO_DEPTH    16
/* i2c reg test */
#define SLAVE_REG_TEST_VALUE       0x00000005
#define CONTROL_REG_TEST_VALUE     0x0000571F


/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO                  dmb()

typedef struct zynq_i2c_registers {
    uint control;
    uint status;
    uint address;
    uint data;
    uint interrupt_status;
    uint transfer_size;
    uint slave_mon_pause;
    uint time_out;
    uint interrupt_mask;
    uint interrupt_enable;
    uint interrupt_disable;
} ccsr_zynq_i2c;


/******** History ******** 
$Log: zynq_i2c.h,v $
Revision 1.1  2013/04/19 07:17:53  xiaoyizh


Initial check in for Prince NIM.


$Endlog$
*/
