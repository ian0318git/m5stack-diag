/* $Id: platform_idprom_utils.h,v 1.2 2021/06/02 08:22:35 iachang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/platform_idprom_utils.h,v $
 *------------------------------------------------------------------
 *
 * platform_idprom_utils.h: Proto types for platform_idprom_utils.c
 *
 * May 2013 - porting the code from Nightster
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Original Author: Jay McCloy
 */

#ifndef _IDPROM_UTILS_H_
#define _IDPROM_UTILS_H_
/*
 * From: idprom_utils.c
 */
void idprom_update_field_length(uchar *data, int length, int table_length, 
                                 int field_type, int field_length);
int idprom_get_field_data(uchar *data, int data_length, int searched_type,
                          void *return_buffer, int maxlength, boolean exact);
int idprom_find_field_data(uchar *data, int data_length, int searched_type,
                          void *return_buffer, int maxlength, boolean exact);
uint32 init_idprom_struc(uchar *dflt_data, uchar* real_data, int data_length);
boolean idprom_update_field_data(uchar *data, int data_length, 
				                 boolean permit_new, int field_type, 
				                 int field_length, void *field_data, 
				                 ushort *f_start, ushort *f_length);
void print_hex(uchar *ep, int len);
void print_ascii(uchar *ep, int len);
void getMacString(uchar *ep, uchar *mac);
void *pem_idprom_get_entry(uchar *data, uchar *end, int *type, int *size,
		                   int *disp_type);
void idprom_print_field(uchar *ep, int type, int disp_type, int size);
void idprom_print_all_fields(uchar *data, int length);
void idprom_display(uchar *buf, int size);
int mcp_print_buffer_data_with_ascii( uchar *buffer, uint32 size, int char_per_line, int extra_space );
#ifndef     MCP_1RUL
void idprom_display_env_voltages(idprom_env_voltage_t *env_voltages);
int idprom_find_env_voltage_data(uchar *data, int data_length, idprom_env_voltage_t *return_buffer);
#endif   /* MCP_1RUL */

#define PCAMAP_SENSOR_NAME_LEN    4

#endif  /* _IDPROM_UTILS_H_ */


/*-------------------------------------------------
 * $Log: platform_idprom_utils.h,v $
 * Revision 1.2  2021/06/02 08:22:35  iachang
 * CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
 *
 * Revision 1.1.8.2  2020/08/26 02:37:51  iachang
 * Merge Fugazi code into main trunk
 *
 * Revision 1.1.6.3  2020/08/04 08:37:06  iachang
 * Update Copyright to 2020
 *
 * Revision 1.1.6.2  2019/03/14 03:48:27  letsai
 * Initial check in.
 *
 *
 *
 *
 * $Endlog$
 */
