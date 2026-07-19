/* $Id: patriot_fpga.c,v 1.1 2014/03/25 02:12:43 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: patriot_fpga.c
 *
 * Description: FPGA module to handle interrupts and other utilities
 *
 *      
 * Author: Huan Ngo, port from IOS
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */



#include <linux/io.h>
#include <asm/uaccess.h> /* for copy_from_user & copy_to_user */
#include <linux/cdev.h>  /* for cdev struct */
#include <linux/delay.h>
#include "klm_defs.h"
#include "patriot_fpga.h"
#include "../apps/patriot_intr.h"


#define SAMPLE_OUTPUT "this is from fpga"
struct patriot_fpga *glob_patriot_fpga;
extern patriot_intr_dev_t *glob_intr_dev;

/*
 * function to read and write to fpga
 */


static int
patriot_fpga_xfer (int offset,
		   unsigned char value, int operation)
{
    int ret = -1;
    if (!glob_patriot_fpga)
    {
      goto out;
    } 
    
    if (operation == PATRIOT_XFER_READ)
    {
        asm volatile ("msync");
        ret = i2c_smbus_read_byte_data (glob_patriot_fpga->client, offset);
    } else {
        mutex_lock (&glob_patriot_fpga->lock);
        ret = i2c_smbus_write_byte_data (glob_patriot_fpga->client,
                offset, value);
        mutex_unlock (&glob_patriot_fpga->lock);
        asm volatile ("msync");
    }
    return ret; 
out:
  return -1;
}

int
patriot_fpga_read_led (void)
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, led);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_led);

int
patriot_fpga_write_led (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, led);
  glob_patriot_fpga->fpga_reg.led |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_led);

int
patriot_fpga_read_porttype (void)
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, port_type);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_porttype);

int
patriot_fpga_write_porttype (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, port_type);
  setbits8 (&glob_patriot_fpga->fpga_reg.port_type, value);
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_porttype);

int
patriot_fpga_read_frmr_gpio_reg ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, gpio_reg);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_frmr_gpio_reg);

int
patriot_fpga_write_frmr_gpio_reg (unsigned char value)
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, gpio_reg);
  glob_patriot_fpga->fpga_reg.gpio_reg |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_frmr_gpio_reg);

int
patriot_fpga_read_read_frmr_gpio_oe_reg ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, gpio_oe_reg);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_read_frmr_gpio_oe_reg);

int
patriot_fpga_read_write_frmr_gpio_oe_reg (unsigned char value)
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, gpio_oe_reg);
  glob_patriot_fpga->fpga_reg.gpio_oe_reg |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_write_frmr_gpio_oe_reg);

int
patriot_fpga_read_te3_status ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, status);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_te3_status);

int
patriot_fpga_write_te3_status (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, status);
  glob_patriot_fpga->fpga_reg.status |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_te3_status);

int
patriot_fpga_read_line_config ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, t3e3_liu);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_line_config);

int
patriot_fpga_write_line_config (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3e3_liu);
  glob_patriot_fpga->fpga_reg.t3e3_liu |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_line_config);

int
patriot_fpga_read_T3_subrate_sel ()
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3_mode);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_T3_subrate_sel);

int
patriot_fpga_write_T3_subrate_sel (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3_mode);
  glob_patriot_fpga->fpga_reg.t3_mode |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_T3_subrate_sel);

int
patriot_fpga_read_T3_subrate_bw_sel1 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, t3_bw_1);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_T3_subrate_bw_sel1);

int
patriot_fpga_write_T3_subrate_bw_sel1 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3_bw_1);
  glob_patriot_fpga->fpga_reg.t3_bw_1 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_T3_subrate_bw_sel1);

int
patriot_fpga_read_T3_subrate_bw_sel2 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, t3_bw_2);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_T3_subrate_bw_sel2);

int
patriot_fpga_write_T3_subrate_bw_sel2 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3_bw_2);
  glob_patriot_fpga->fpga_reg.t3_bw_2 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_T3_subrate_bw_sel2);

int
patriot_fpga_read_T3_subrate_bw_sel3 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, t3_bw_3);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_T3_subrate_bw_sel3);

int
patriot_fpga_write_T3_subrate_bw_sel3 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, t3_bw_3);
  glob_patriot_fpga->fpga_reg.t3_bw_3 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_T3_subrate_bw_sel3);

int
patriot_fpga_read_E3_subrate_sel ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, e3_mode);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_E3_subrate_sel);

int
patriot_fpga_write_E3_subrate_sel (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, e3_mode);
  glob_patriot_fpga->fpga_reg.e3_mode |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_E3_subrate_sel);

int
patriot_fpga_read_E3_subrate_bw_sel1 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, e3_bw_1);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_E3_subrate_bw_sel1);

int
patriot_fpga_write_E3_subrate_bw_sel1 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, e3_bw_1);
  glob_patriot_fpga->fpga_reg.e3_bw_1 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_E3_subrate_bw_sel1);

int
patriot_fpga_read_E3_subrate_bw_sel2 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, e3_bw_2);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_E3_subrate_bw_sel2);

int
patriot_fpga_write_E3_subrate_bw_sel2 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, e3_bw_2);
  glob_patriot_fpga->fpga_reg.e3_bw_2 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_E3_subrate_bw_sel2);

int
patriot_fpga_read_E3_subrate_bw_sel3 ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, e3_bw_3);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_E3_subrate_bw_sel3);

int
patriot_fpga_write_E3_subrate_bw_sel3 (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, e3_bw_3);
  glob_patriot_fpga->fpga_reg.e3_bw_3 |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_E3_subrate_bw_sel3);

int
patriot_fpga_read_fpga_ver ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, fpga_ver);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_fpga_ver);

int
patriot_fpga_read_serial_fifo ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, serial_fifo);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_serial_fifo);

int
patriot_fpga_write_serial_fifo (unsigned char value)
{
  int offset = 0;

  offset = offsetof (patriot_fpga_reg_t, serial_fifo);
  glob_patriot_fpga->fpga_reg.serial_fifo |= (uchar)value;
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_serial_fifo);

int
patriot_fpga_read_icr ()
{
  int offset = 0;
  offset = offsetof (patriot_fpga_reg_t, icr);
  return patriot_fpga_xfer (offset, 0, PATRIOT_XFER_READ);
}
EXPORT_SYMBOL_GPL (patriot_fpga_read_icr);

int
patriot_fpga_write_icr (unsigned char value)
{
  int offset = 0;
#ifdef DEBUG
  printk("\n%s %02x", __func__, value);
#endif  
  offset = offsetof (patriot_fpga_reg_t, icr);
  setbits8 (&glob_patriot_fpga->fpga_reg.icr, value);
  return patriot_fpga_xfer (offset, value, PATRIOT_XFER_WRITE);
}
EXPORT_SYMBOL_GPL (patriot_fpga_write_icr);

static void
patriot_read_fpga_settings (void)
{
  setbits8 (&glob_patriot_fpga->fpga_reg.led,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t, led)));
  setbits8 (&glob_patriot_fpga->fpga_reg.port_type,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						port_type)));
  setbits8 (&glob_patriot_fpga->fpga_reg.gpio_reg,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						gpio_reg)));
  setbits8 (&glob_patriot_fpga->fpga_reg.gpio_oe_reg,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						gpio_oe_reg)));
  setbits8 (&glob_patriot_fpga->fpga_reg.status,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t, status)));
  setbits8 (&glob_patriot_fpga->fpga_reg.t3e3_liu,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						t3e3_liu)));
  setbits8 (&glob_patriot_fpga->fpga_reg.t3_mode,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						t3_mode)));
  setbits8 (&glob_patriot_fpga->fpga_reg.t3_bw_1,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						t3_bw_1)));
  setbits8 (&glob_patriot_fpga->fpga_reg.t3_bw_2,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						t3_bw_2)));
  setbits8 (&glob_patriot_fpga->fpga_reg.t3_bw_3,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						t3_bw_3)));
  setbits8 (&glob_patriot_fpga->fpga_reg.e3_mode,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						e3_mode)));
  setbits8 (&glob_patriot_fpga->fpga_reg.e3_bw_1,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						e3_bw_1)));
  setbits8 (&glob_patriot_fpga->fpga_reg.e3_bw_2,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						e3_bw_2)));
  setbits8 (&glob_patriot_fpga->fpga_reg.e3_bw_3,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						e3_bw_3)));
  setbits8 (&glob_patriot_fpga->fpga_reg.fpga_ver,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						fpga_ver)));
  setbits8 (&glob_patriot_fpga->fpga_reg.serial_fifo,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t,
						serial_fifo)));
  setbits8 (&glob_patriot_fpga->fpga_reg.icr,
	    i2c_smbus_read_byte_data (glob_patriot_fpga->client,
				      offsetof (patriot_fpga_reg_t, icr)));
}

int
patriot_display_fpga_reg (unsigned long toUser, u8 hw)
{

  char msg[1024], *p;
  int len = 0;

  if (!glob_patriot_fpga)
    goto out;

  p = &msg[0];
  printk("\nhw is %02x", hw);

  if (hw)
    patriot_read_fpga_settings ();
  len +=
    sprintf (p + len, "LED's Control Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.led));
  len +=
    sprintf (p + len, "Port Type Select Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.port_type));
  len +=
    sprintf (p + len, "Framer GPIO register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.gpio_reg));
  len +=
    sprintf (p + len, "Framer GPIO OE register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.gpio_oe_reg));
  len +=
    sprintf (p + len, "TE Status Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.status));
  len +=
    sprintf (p + len, "TE3 Line Configuration Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.t3e3_liu));
  len +=
    sprintf (p + len, "T3 Subrate Mode Selection Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.t3_mode));
  len +=
    sprintf (p + len,
	     "T3 Subrate Bandwidth Selection Register-1 = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.t3_bw_1));
  len +=
    sprintf (p + len,
	     "T3 Subrate Bandwidth Selection Register-2 = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.t3_bw_2));
  len +=
    sprintf (p + len,
	     "T3 Subrate Bandwidth Selection Register-3 = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.t3_bw_3));
  len +=
    sprintf (p + len, "E3 Subrate Mode Selection Register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.e3_mode));
  len +=
    sprintf (p + len,
	     "E3 Subrate Bandwidth Selection Register-1 = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.e3_bw_1));
  len +=
    sprintf (p + len,
	     "E3 Subrate Bandwidth Selection Register-2 = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.e3_bw_2));
  len +=
    sprintf (p + len,
	     "E3 Subrate Bandwidth Selection Register-3 =  %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.e3_bw_3));
  len +=
    sprintf (p + len, "TDM fpga revision register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.fpga_ver));
  len +=
    sprintf (p + len, "Nibble to Serial FIFO control register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.serial_fifo));
  len +=
    sprintf (p + len, "Interrupt Cause register = %02x\n",
	     in_8 (&glob_patriot_fpga->fpga_reg.icr));
  len +=
    sprintf (p + len, "RxTx fifo full Interrupts received = %02x\n",
	     glob_patriot_fpga->irq_recvd);
  msg[len + 1] = '\0';

  len = strlen (msg);
  if (copy_to_user ((char *) toUser, msg, len))
    {
      printk ("\ncopy to user failed");
      len = -1;
    }
  return len;
out:
  return -1;
}

EXPORT_SYMBOL_GPL (patriot_display_fpga_reg);

static void
patriot_fpga_count (void)
{
  glob_patriot_fpga->intr_dev->fpga_intr_cnt++;
  return;
}


static irqreturn_t
patriot_fpga_irq (int irq, void *dev_id)
{
  struct i2c_client *client = dev_id;
  struct patriot_fpga *patriot_fpga = i2c_get_clientdata (client);

  printk("\nrecvd interrupt");
  disable_irq_nosync (irq);
  patriot_fpga_count();
  schedule_work (&patriot_fpga->work);
  
  return IRQ_HANDLED;
}

static void
patriot_fpga_work (struct work_struct *work)
{
  struct patriot_fpga *fpga = container_of (work, struct patriot_fpga, work);
  int ret, i;
#ifdef DEBUG
  printk("\n%s called", __func__);
#endif  
  ret = patriot_fpga_read_fpga_ver ();
#ifdef DEBUG  
  printk ("\n FPGA Revisions %02x", ret);
  ret = patriot_fpga_read_fpga_ver ();
  printk ("\n FPGA Revisions %02x", ret); 
  ret = patriot_fpga_read_icr();
  printk ("\n icr 1 = 0x%02x", ret);
#endif

  /* Make sure the interrupt bit is cleared before enable interrupt */
  for (i = 0; i < 5; i++) {
      patriot_fpga_write_icr (PATRIOT_TX_FOFL_INTR_OFF);
      mdelay(100);
      ret = patriot_fpga_read_icr();
      if (ret == 0) {
	  break;
      }
  }
#ifdef DEBUG  
  printk ("\n icr 2 = 0x%02x", ret);
#endif  
  fpga->irq_recvd++;  // not used

  if (ret == 0) {
#ifdef DEBUG      
      printk("\nenabling interrupt\n");
#endif      
      enable_irq(fpga->client->irq);
  }

  return;
}

static int __devinit
patriot_fpga_probe (struct i2c_client *client, const struct i2c_device_id *id)
{
    struct i2c_adapter *adapter = client->adapter;
    int ret;
    
    if (!i2c_check_functionality (adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
	printk ("adm1021: detect failed, " "smbus byte data not supported!\n");
	return -ENODEV;
    }
    
    printk ("\n Patriot fpga device found on i2c bus");
    glob_patriot_fpga = kzalloc (sizeof (struct patriot_fpga), GFP_KERNEL);
    if (!glob_patriot_fpga)
	return -ENOMEM;
    
    memset (glob_patriot_fpga, 0, sizeof (struct patriot_fpga));
    INIT_WORK (&glob_patriot_fpga->work, patriot_fpga_work);
    mutex_init (&glob_patriot_fpga->lock);
    glob_patriot_fpga->irq_recvd = 0;
    glob_patriot_fpga->client = client;
    i2c_set_clientdata (client, glob_patriot_fpga);
    
    glob_patriot_fpga->intr_dev = glob_intr_dev;
    
    glob_patriot_fpga->intr_dev->fpga_intr_cnt = 0;
    
    ret = request_irq (client->irq, patriot_fpga_irq, IRQF_SHARED,
		       "patriot_fpga", client);
    if (ret)
	{
	    dev_err (&client->dev, "unable to request IRQ\n");
	    goto out_free;
	}
    printk("\nIRQ is mapped successfully %d", client->irq);
    
    return 0;
    
out_free:
    i2c_set_clientdata (client, NULL);
    kfree (glob_patriot_fpga);
    kfree (glob_intr_dev);
    
    return ret;
}

static int __devexit
patriot_fpga_remove (struct i2c_client *client)
{
  struct patriot_fpga *fpga = i2c_get_clientdata (client);
  i2c_set_clientdata (client, NULL);
  kfree (fpga);
  return 0;
}

static const struct i2c_device_id patriot_fpga_ids[] = {
  {"patriot-fpga",},
  {},
};

MODULE_DEVICE_TABLE (i2c, patriot_fpga_ids);

static struct i2c_driver patriot_fpga_driver = {
  .driver = {
	     .name = "patriot-fpga",
	     .owner = THIS_MODULE,
	     },
  .probe = patriot_fpga_probe,
  .remove = __devexit_p (patriot_fpga_remove),
  .id_table = patriot_fpga_ids,
};

static int __init
patriot_fpga_init (void)
{
      
    printk("\nPATRIOT FPGA INIT\n");
    return i2c_add_driver (&patriot_fpga_driver);
}

module_init (patriot_fpga_init);

static void __exit
patriot_fpga_exit (void)
{
  i2c_del_driver (&patriot_fpga_driver);
}

module_exit (patriot_fpga_exit);

MODULE_DESCRIPTION ("Patriot fpga i2c driver");
MODULE_AUTHOR ("<Damodharam Ammepalli");
MODULE_LICENSE ("GPL");


/******** History ********/ 
/*------------------------------------------------------------------------------
 * $Log: patriot_fpga.c,v $
 * Revision 1.1  2014/03/25 02:12:43  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.2  2012/05/08 23:52:56  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.2.6  2012/04/12 18:37:03  huanngo
 * Clean up and cosmetic changes
 *
 * Revision 1.1.2.5  2012/03/27 07:45:06  steja
 * Fix Warning compilation
 *
 * Revision 1.1.2.4  2012/03/12 23:02:49  huanngo
 * Fixing a bug in FPGA interrupt test
 *
 * Revision 1.1.2.3  2012/02/06 22:30:16  huanngo
 * Update to not use bitbake to compile, use make with local kernel
 *
 * Revision 1.1.2.2  2012/01/09 23:06:19  huanngo
 * Support on xformers mips and informers and clean up
 *
 * Revision 1.1.2.1  2011/12/21 23:49:22  huanngo
 * Support for FPGA kernel module
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */
