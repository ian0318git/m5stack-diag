/* $Id: ds3170.c,v 1.1 2014/03/25 02:12:32 huanngo Exp $
 * $Source: 
 *******************************************************************************
 * File Name: ds3170.c
 *
 * Description: drivers for framers DS3170 T3/E3
 *
 *
 * Author: Sofian Teja
 * Copyright (c)2011 - 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *******************************************************************************
 */

/* Includes. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "patriot_main.h"
#include "common_utils.h"
#include "ds3170.h"     /* structure definition for the chip */

extern uchar err_msg[];

/* Global Register offset 0x0000 ~ 0x001d */
static reg_info_t ds3170_gl_regs[] = {
    {"Global ID RegL                ", 0x0000, READ_ONLY,  {1}, 0xFF, 0x00},
    {"Global ID RegH                ", 0x0001, READ_ONLY,  {1}, 0xFF, 0x00},
    {"Global Ctrl RegL 1            ", 0x0002, READ_WRITE, {1}, 0xFC, 0x02},
    {"Global Ctrl RegH 1            ", 0x0003, READ_WRITE, {1}, 0x40, 0x00},
    {"Global Ctrl RegL 2            ", 0x0004, READ_WRITE, {1}, 0x0E, 0x00},
    {"Global Ctrl RegH 2            ", 0x0005, READ_WRITE, {1}, 0x0F, 0x00},
    {"Global GPIO Ctrl RegL         ", 0x000a, READ_WRITE, {1}, 0xFF, 0x00},
    {"Global GPIO Ctrl RegH         ", 0x000b, READ_WRITE, {1}, 0xFF, 0x00},
    {"Global Int Status RegL        ", 0x0010, READ_ONLY,  {1}, 0x11, 0x00},
    {"Global Int Status RegH        ", 0x0011, READ_ONLY,  {1}, 0x00, 0x00},
    {"Global Int Enable RegL        ", 0x0012, READ_WRITE, {1}, 0x11, 0x00},
    {"Global Int Enable RegH        ", 0x0013, READ_WRITE, {1}, 0x00, 0x00},
    {"Global Status RegL            ", 0x0014, READ_ONLY,  {1}, 0x03, 0x00},
    {"Global Status RegH            ", 0x0015, READ_ONLY,  {1}, 0x00, 0x00},
    {"Global Status RegL Latched    ", 0x0016, READ_ONLY,  {1}, 0x1F, 0x00},
    {"Global Status RegH Latched    ", 0x0017, READ_ONLY,  {1}, 0x00, 0x00},
    {"Global Status RegL Int Enbled ", 0x0018, READ_ONLY,  {1}, 0x07, 0x00},
    {"Global Status RegH Int Enbled ", 0x0019, READ_ONLY,  {1}, 0x00, 0x00},
    {"Global GPIO read regL         ", 0x001c, READ_ONLY,  {1}, 0xFF, 0x00},
    {"Global GPIO read regH         ", 0x001d, READ_ONLY,  {1}, 0x00, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* Port Register offset 0x0040 ~ 0x0057 */
static reg_info_t ds3170_port_regs[] = {
    {"Port Ctrl RegL 1              ", 0x0040, READ_WRITE,  {1}, 0xD8, 0x06},
    {"Port Ctrl RegH 1              ", 0x0041, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Ctrl RegL 2              ", 0x0042, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Ctrl RegH 2              ", 0x0043, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Ctrl RegL 3              ", 0x0044, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Ctrl RegH 3              ", 0x0045, READ_WRITE,  {1}, 0x3F, 0x00},
    {"Port Ctrl RegL 4              ", 0x0046, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Ctrl RegH 4              ", 0x0047, READ_WRITE,  {1}, 0x0F, 0x00},
    {"Port Invert Ctrl RegL 1       ", 0x004a, READ_WRITE,  {1}, 0xFF, 0x00},
    {"Port Invert Ctrl RegH 1       ", 0x004b, READ_WRITE,  {1}, 0xDF, 0x00},
    {"Port Invert Ctrl RegL 2       ", 0x004c, READ_WRITE,  {1}, 0xDE, 0x00},
    {"Port Invert Ctrl RegH 2       ", 0x004d, READ_WRITE,  {1}, 0x76, 0x00},
    {"Port Interrupt Status RegL    ", 0x0050, READ_ONLY,   {1}, 0xFF, 0x00},
    {"Port Interrupt Status RegH    ", 0x0051, READ_ONLY,   {1}, 0x03, 0x00},
    {"Port Status RegL              ", 0x0052, READ_ONLY,   {1}, 0x07, 0x00},
    {"Port Status RegH              ", 0x0053, READ_ONLY,   {1}, 0x00, 0x00},
    {"Port Status RegL Latched      ", 0x0054, READ_ONLY,   {1}, 0xCF, 0x00},
    {"Port Status RegH Latched      ", 0x0055, READ_ONLY,   {1}, 0x00, 0x00},
    {"Port Status RegL Int Enable   ", 0x0056, READ_WRITE,  {1}, 0x07, 0x00},
    {"Port Status RegH Int Enable   ", 0x0057, READ_WRITE,  {1}, 0x00, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* BERT Register offset 0x0060 ~ 0x007b */
static reg_info_t ds3170_bert_regs[] = {
    {"BERT Ctrl RegL                 ",0x0060, READ_WRITE,  {1}, 0xFF, 0x00},
    {"BERT Ctrl RegH                 ",0x0061, READ_WRITE,  {1}, 0x00, 0x00},
    {"BERT Pattern Config RegL       ",0x0062, READ_WRITE,  {1}, 0x7F, 0x00},
    {"BERT Pattern Config RegH       ",0x0063, READ_WRITE,  {1}, 0x1F, 0x00},
    {"BERT Seed/Pattern RegL 1       ",0x0064, READ_WRITE,  {1}, 0xFF, 0x00},
    {"BERT Seed/Pattern RegH 1       ",0x0065, READ_WRITE,  {1}, 0xFF, 0x00},
    {"BERT Seed/Pattern RegL 2       ",0x0066, READ_WRITE,  {1}, 0xFF, 0x00},
    {"BERT Seed/Pattern RegH 2       ",0x0067, READ_WRITE,  {1}, 0xFF, 0x00},
    {"BERT Tx Err Insertion Ctrl RegL",0x0068, READ_WRITE,  {1}, 0x3F, 0x00},
    {"BERT Tx Err Insertion Ctrl RegH",0x0069, READ_WRITE,  {1}, 0x00, 0x00},
    {"BERT Status RegL               ",0x006c, READ_ONLY,   {1}, 0x0B, 0x00},
    {"BERT Status RegH               ",0x006d, READ_ONLY,   {1}, 0x00, 0x00},
    {"BERT Status RegL Latched       ",0x006e, READ_ONLY,   {1}, 0x0F, 0x00},
    {"BERT Status RegH Latched       ",0x006f, READ_ONLY,   {1}, 0x00, 0x00},
    {"BERT Status RegL Int Enable    ",0x0070, READ_WRITE,  {1}, 0x0F, 0x00},
    {"BERT Status RegH Int Enable    ",0x0071, READ_WRITE,  {1}, 0x00, 0x00},
    {"BERT Rx Bit Err Count RegL 1   ",0x0074, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Err Count RegH 1   ",0x0075, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Err Count RegL 2   ",0x0076, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Err Count RegH 2   ",0x0077, READ_ONLY,   {1}, 0x00, 0x00},
    {"BERT Rx Bit Count RegL 1       ",0x0078, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Count RegH 1       ",0x0079, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Count RegL 2       ",0x007a, READ_ONLY,   {1}, 0xFF, 0x00},
    {"BERT Rx Bit Count RegH 2       ",0x007b, READ_ONLY,   {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* Line Register offset 0x008c ~ 0x009f */
static reg_info_t ds3170_line_regs[] = {
    {"LINE Tx Ctrl RegL                   ",0x008c, READ_WRITE,{1}, 0x1F, 0x00},
    {"LINE Tx Ctrl RegH                   ",0x008d, READ_WRITE,{1}, 0x00, 0x00},
    {"LINE Rx Ctrl RegL                   ",0x0090, READ_WRITE,{1}, 0x0F, 0x00},
    {"LINE Rx Ctrl RegH                   ",0x0091, READ_WRITE,{1}, 0x00, 0x00},
    {"LINE Rx Status RegL                 ",0x0094, READ_ONLY, {1}, 0x0B, 0x00},
    {"LINE Rx Status RegH                 ",0x0095, READ_ONLY, {1}, 0x00, 0x00},
    {"LINE Rx Status RegL Latched         ",0x0096, READ_ONLY, {1}, 0x3F, 0x00},
    {"LINE Rx Status RegH Latched         ",0x0097, READ_ONLY, {1}, 0x00, 0x00},
    {"LINE Rx Status RegL Int Enable      ",0x0098, READ_WRITE,{1}, 0x3F, 0x00},
    {"LINE Rx Status RegH Int Enable      ",0x0099, READ_WRITE,{1}, 0x00, 0x00},
    {"LINE Rx Bipolar Violation Count RegL",0x009c, READ_ONLY, {1}, 0xFF, 0x00},
    {"LINE Rx Bipolar Violation Count RegH",0x009d, READ_ONLY, {1}, 0xFF, 0x00},
    {"LINE Rx Excessive 0 Count RegL      ",0x009e, READ_ONLY, {1}, 0xFF, 0x00},
    {"LINE Rx Excessive 0 Count RegH      ",0x009f, READ_ONLY, {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* HDLC Register offset 0x00a0 ~ 0x00bd */
static reg_info_t ds3170_hdlc_regs[] = {
    {"HDLC Tx Ctrl RegL                  ",0x00a0, READ_WRITE,{1}, 0x7F, 0x00},
    {"HDLC Tx Ctrl RegH                  ",0x00a1, READ_WRITE,{1}, 0x1F, 0x08},
    {"HDLC Tx FIFO Data RegL             ",0x00a2, READ_WRITE,{1}, 0x01, 0x00},
    {"HDLC Tx FIFO Data RegH             ",0x00a3, READ_WRITE,{1}, 0x00, 0x00},
    {"HDLC Tx Status RegL                ",0x00a4, READ_ONLY, {1}, 0x07, 0x00},
    {"HDLC Tx Status RegH                ",0x00a5, READ_ONLY, {1}, 0x3F, 0x00},
    {"HDLC Tx Status RegL Latched        ",0x00a6, READ_ONLY, {1}, 0x3B, 0x00},
    {"HDLC Tx Status RegH Latched        ",0x00a7, READ_ONLY, {1}, 0x00, 0x00},
    {"HDLC Tx Status RegL Int Enable     ",0x00a8, READ_WRITE,{1}, 0x3B, 0x00},
    {"HDLC Tx Status RegH Int Enable     ",0x00a9, READ_WRITE,{1}, 0x00, 0x00},
    {"HDLC Rx Ctrl RegL                  ",0x00b0, READ_WRITE,{1}, 0x0F, 0x00},
    {"HDLC Rx Ctrl RegH                  ",0x00b1, READ_WRITE,{1}, 0x1F, 0x08},
    {"HDLC Rx Status RegL                ",0x00b4, READ_ONLY, {1}, 0x07, 0x00},
    {"HDLC Rx Status RegH                ",0x00b5, READ_ONLY, {1}, 0x00, 0x00},
    {"HDLC Rx Status RegL Latched        ",0x00b6, READ_ONLY, {1}, 0x9D, 0x00},
    {"HDLC Rx Status RegH Latched        ",0x00b7, READ_ONLY, {1}, 0x00, 0x00},
    {"HDLC Rx Status RegL Int Enable     ",0x00b8, READ_WRITE,{1}, 0x9D, 0x00},
    {"HDLC Rx Status RegH Int Enable     ",0x00b9, READ_WRITE,{1}, 0x00, 0x00},
    {"HDLC Rx FIFO Data RegL             ",0x00bc, READ_ONLY, {1}, 0x0F, 0x00},
    {"HDLC Rx FIFO Data RegH             ",0x00bd, READ_ONLY, {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* FEAC Register offset 0x00c0 ~ 0x00dd */
static reg_info_t ds3170_feac_regs[] = {
    {"FEAC Tx Ctrl RegL                  ",0x00c0, READ_WRITE,{1}, 0x07, 0x00},
    {"FEAC Tx Ctrl RegH                  ",0x00c1, READ_WRITE,{1}, 0x00, 0x00},
    {"FEAC Tx FEAC Data RegL             ",0x00c2, READ_WRITE,{1}, 0x3F, 0x00},
    {"FEAC Tx FEAC Data RegH             ",0x00c3, READ_WRITE,{1}, 0x3F, 0x00},
    {"FEAC Tx Status RegL                ",0x00c4, READ_ONLY, {1}, 0x01, 0x00},
    {"FEAC Tx Status RegH                ",0x00c5, READ_ONLY, {1}, 0x00, 0x00},
    {"FEAC Tx Status RegL Latched        ",0x00c6, READ_ONLY, {1}, 0x01, 0x00},
    {"FEAC Tx Status RegH Latched        ",0x00c7, READ_ONLY, {1}, 0x00, 0x00},
    {"FEAC Tx Status RegL Int Enable     ",0x00c8, READ_WRITE,{1}, 0x01, 0x00},
    {"FEAC Tx Status RegH Int Enable     ",0x00c9, READ_WRITE,{1}, 0x00, 0x00},
    {"FEAC Rx Ctrl RegL                  ",0x00d0, READ_WRITE,{1}, 0x01, 0x00},
    {"FEAC Rx Ctrl RegH                  ",0x00d1, READ_WRITE,{1}, 0x00, 0x00},
    {"FEAC Rx Status RegL                ",0x00d4, READ_ONLY, {1}, 0x0B, 0x00},
    {"FEAC Rx Status RegH                ",0x00d5, READ_ONLY, {1}, 0x00, 0x00},
    {"FEAC Rx Status RegL Latched        ",0x00d6, READ_ONLY, {1}, 0x07, 0x00},
    {"FEAC Rx Status RegH Latched        ",0x00d7, READ_ONLY, {1}, 0x00, 0x00},
    {"FEAC Rx Status RegL Int Enable     ",0x00d8, READ_WRITE,{1}, 0x07, 0x00},
    {"FEAC Rx Status RegH Int Enable     ",0x00d9, READ_WRITE,{1}, 0x00, 0x00},
    {"FEAC Rx FIFO Data RegL             ",0x00dc, READ_ONLY, {1}, 0xBF, 0x00},
    {"FEAC Rx FIFO Data RegH             ",0x00dd, READ_ONLY, {1}, 0x00, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* Trail Trace Register offset 0x00e8 ~ 0x00ff */
static reg_info_t ds3170_tt_regs[] = {
    {"TT Tx Ctrl RegL                  ",0x00e8, READ_WRITE,{1}, 0x0F, 0x00},
    {"TT Tx Ctrl RegH                  ",0x00e9, READ_WRITE,{1}, 0x00, 0x00},
    {"TT Tx Id Addr RegL               ",0x00ea, READ_ONLY, {1}, 0x0F, 0x00},
    {"TT Tx Id Addr RegH               ",0x00eb, READ_ONLY, {1}, 0x00, 0x00},
    {"TT Tx Id RegL                    ",0x00ec, READ_ONLY, {1}, 0xFF, 0x00},
    {"TT Tx Id RegH                    ",0x00ed, READ_ONLY, {1}, 0x00, 0x00},
    {"TT Rx Ctrl RegL                  ",0x00f0, READ_WRITE,{1}, 0x0F, 0x00},
    {"TT Rx Ctrl RegH                  ",0x00f1, READ_WRITE,{1}, 0x00, 0x00},
    {"TT Rx Id Addr RegL               ",0x00f2, READ_ONLY, {1}, 0x0F, 0x00},
    {"TT Rx Id Addr RegH               ",0x00f3, READ_ONLY, {1}, 0x0F, 0x00},
    {"TT Rx Status RegL                ",0x00f4, READ_ONLY, {1}, 0x07, 0x00},
    {"TT Rx Status RegH                ",0x00f5, READ_ONLY, {1}, 0x00, 0x00},
    {"TT Rx Status RegL Latched        ",0x00f6, READ_ONLY, {1}, 0x0F, 0x00},
    {"TT Rx Status RegH Latched        ",0x00f7, READ_ONLY, {1}, 0x00, 0x00},
    {"TT Rx Status RegL Int Enable     ",0x00f8, READ_WRITE,{1}, 0x0F, 0x00},
    {"TT Rx Status RegH Int Enable     ",0x00f9, READ_WRITE,{1}, 0x00, 0x00},
    {"TT Rx Id RegL                    ",0x00fc, READ_ONLY, {1}, 0xFF, 0x00},
    {"TT Rx Id RegH                    ",0x00fd, READ_ONLY, {1}, 0x00, 0x00},
    {"TT Expected Id  RegL             ",0x00fe, READ_ONLY, {1}, 0xFF, 0x00},
    {"TT Expected Id  RegH             ",0x00ff, READ_ONLY, {1}, 0x00, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* T3 Register offset 0x0118 ~ 0x013b */
static reg_info_t ds3170_t3_regs[] = {
    {"T3 Tx Ctrl RegL                    ",0x0118, READ_WRITE, {1}, 0x3F, 0x00},
    {"T3 Tx Ctrl RegH                    ",0x0119, READ_WRITE, {1}, 0x9F, 0x00},
    {"T3 Tx Err Insertion RegL           ",0x011a, READ_WRITE, {1}, 0xFF, 0x00},
    {"T3 Tx Err Insertion RegH           ",0x011b, READ_WRITE, {1}, 0x0F, 0x00},
    {"T3 Rx Ctrl RegL                    ",0x0120, READ_WRITE, {1}, 0xFF, 0x00},
    {"T3 Rx Ctrl RegH                    ",0x0121, READ_WRITE, {1}, 0xFF, 0x00},
    {"T3 Rx Status RegL 1                ",0x0124, READ_ONLY,  {1}, 0xDF, 0x00},
    {"T3 Rx Status RegH 1                ",0x0125, READ_ONLY,  {1}, 0x0F, 0x00},
    {"T3 Rx Status RegL 2                ",0x0126, READ_ONLY,  {1}, 0x0F, 0x00},
    {"T3 Rx Status RegH 2                ",0x0127, READ_ONLY,  {1}, 0x00, 0x00},
    {"T3 Rx Status RegL Latched 1        ",0x0128, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx Status RegH Latched 1        ",0x0129, READ_ONLY,  {1}, 0x0F, 0x00},
    {"T3 Rx Status RegL Latched 2        ",0x012a, READ_ONLY,  {1}, 0x0F, 0x00},
    {"T3 Rx Status RegH Latched 2        ",0x012b, READ_ONLY,  {1}, 0x0F, 0x00},
    {"T3 Rx Status RegL Int Enable 1     ",0x012c, READ_WRITE, {1}, 0xFF, 0x00},
    {"T3 Rx Status RegH Int Enable 1     ",0x012d, READ_WRITE, {1}, 0xBF, 0x00},
    {"T3 Rx Status RegL Int Enable 2     ",0x012e, READ_WRITE, {1}, 0x0F, 0x00},
    {"T3 Rx Status RegH Int Enable 2     ",0x012f, READ_WRITE, {1}, 0x0F, 0x00},
    {"T3 Rx Framing Err Count RegL       ",0x0134, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx Framing Err Count RegH       ",0x0135, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx P-Bit Parity Err Count RegL  ",0x0136, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx P-Bit Parity Err Count RegH  ",0x0137, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx Far-End Block Err Count RegL ",0x0138, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx Far-End Block Err Count RegH ",0x0139, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx C-Bit Parity Err Count RegL  ",0x013a, READ_ONLY,  {1}, 0xFF, 0x00},
    {"T3 Rx C-Bit Parity Err Count RegH  ",0x013b, READ_ONLY,  {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* E3G751 Register offset 0x0118 ~ 0x0135 */
static reg_info_t ds3170_e3g751_regs[] = {
    {"E3G751 Tx Ctrl RegL                ",0x0118, READ_WRITE, {1}, 0x3F, 0x00},
    {"E3G751 Tx Ctrl RegH                ",0x0119, READ_WRITE, {1}, 0x9F, 0x00},
    {"E3G751 Tx Err Insertion RegL       ",0x011a, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G751 Tx Err Insertion RegH       ",0x011b, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G751 Rx Ctrl RegL                ",0x0120, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G751 Rx Ctrl RegH                ",0x0121, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G751 Rx Status RegL 1            ",0x0124, READ_ONLY,  {1}, 0xDF, 0x00},
    {"E3G751 Rx Status RegH 1            ",0x0125, READ_ONLY,  {1}, 0x01, 0x00},
    {"E3G751 Rx Status RegL 2            ",0x0126, READ_ONLY,  {1}, 0x01, 0x00},
    {"E3G751 Rx Status RegH 2            ",0x0127, READ_ONLY,  {1}, 0x00, 0x00},
    {"E3G751 Rx Status RegL Latched 1    ",0x0128, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G751 Rx Status RegH Latched 1    ",0x0129, READ_ONLY,  {1}, 0x01, 0x00},
    {"E3G751 Rx Status RegL Latched 2    ",0x012a, READ_ONLY,  {1}, 0x01, 0x00},
    {"E3G751 Rx Status RegH Latched 2    ",0x012b, READ_ONLY,  {1}, 0x01, 0x00},
    {"E3G751 Rx Status RegL Int Enable 1 ",0x012c, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G751 Rx Status RegH Int Enable 1 ",0x012d, READ_WRITE, {1}, 0xBF, 0x00},
    {"E3G751 Rx Status RegL Int Enable 2 ",0x012e, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G751 Rx Status RegH Int Enable 2 ",0x012f, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G751 Rx Framing Err Count RegL   ",0x0134, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G751 Rx Framing Err Count RegH   ",0x0135, READ_ONLY,  {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/* E3G832 Register offset 0x0118 ~ 0x0139 */
static reg_info_t ds3170_e3g832_regs[] = {
    {"E3G832 Tx Ctrl RegL                ",0x0118, READ_WRITE, {1}, 0x3F, 0x00},
    {"E3G832 Tx Ctrl RegH                ",0x0119, READ_WRITE, {1}, 0x9F, 0x00},
    {"E3G832 Tx Err Insertion RegL       ",0x011a, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Tx Err Insertion RegH       ",0x011b, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G832 Tx MA Byte RegL             ",0x011c, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Tx MA Byte RegH             ",0x011d, READ_WRITE, {1}, 0x00, 0x00},
    {"E3G832 Tx NR & GC Byte RegL        ",0x011e, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Tx NR & GC Byte RegH        ",0x011f, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Rx Ctrl RegL                ",0x0120, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Rx Ctrl RegH                ",0x0121, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Rx MA Byte Ctrl RegL        ",0x0122, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G832 Rx MA Byte Ctrl RegH        ",0x0123, READ_WRITE, {1}, 0x00, 0x00},
    {"E3G832 Rx Status RegL 1            ",0x0124, READ_ONLY,  {1}, 0x1F, 0x00},
    {"E3G832 Rx Status RegH 1            ",0x0125, READ_ONLY,  {1}, 0x19, 0x00},
    {"E3G832 Rx Status RegL 2            ",0x0126, READ_ONLY,  {1}, 0x07, 0x00},
    {"E3G832 Rx Status RegH 2            ",0x0127, READ_ONLY,  {1}, 0x00, 0x00},
    {"E3G832 Rx Status RegL Latched 1    ",0x0128, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Status RegH Latched 1    ",0x0129, READ_ONLY,  {1}, 0x3D, 0x00},
    {"E3G832 Rx Status RegL Latched 2    ",0x012a, READ_ONLY,  {1}, 0x07, 0x00},
    {"E3G832 Rx Status RegH Latched 2    ",0x012b, READ_ONLY,  {1}, 0x07, 0x00},
    {"E3G832 Rx Status RegL Int Enable 1 ",0x012c, READ_WRITE, {1}, 0xFF, 0x00},
    {"E3G832 Rx Status RegH Int Enable 1 ",0x012d, READ_WRITE, {1}, 0xBF, 0x00},
    {"E3G832 Rx Status RegL Int Enable 2 ",0x012e, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G832 Rx Status RegH Int Enable 2 ",0x012f, READ_WRITE, {1}, 0x0F, 0x00},
    {"E3G832 Rx MA Byte RegL             ",0x0130, READ_ONLY,  {1}, 0x7F, 0x00},
    {"E3G832 Rx MA Byte RegH             ",0x0131, READ_ONLY,  {1}, 0x00, 0x00},
    {"E3G832 Rx NR and GC Byte RegL      ",0x0132, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx NR and GC Byte RegH      ",0x0133, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Framing Err Count RegL   ",0x0134, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Framing Err Count RegH   ",0x0135, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Parity Err Count RegL    ",0x0136, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Parity Err Count RegH    ",0x0137, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Remote Err IndicationRegL",0x0138, READ_ONLY,  {1}, 0xFF, 0x00},
    {"E3G832 Rx Remote Err IndicationRegH",0x0139, READ_ONLY,  {1}, 0xFF, 0x00},
    {"end",     0, 0, {0}, 0, 0x00},
};


/**********************************************************************
 *
 * Function: patriot_ds3170_reg_read
 *
 * This function is framer utilities for read register
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
void patriot_ds3170_reg_read(void)
{
    ushort offset = 0;
    uchar temp = 0;

    printf("\npatriot_ds3170_register_read\n");
    offset = gethex_answer("Register Offset: ", 0, 0, 0xffff);

    ds3170_read (&temp, (uint)offset);
    printf("\nDS3170 Register @%#x = %#x\n", offset, temp);
}


/**********************************************************************
 *
 * Function: patriot_ds3170_reg_write
 *
 * This function is framer utilities for write register
 *
 * Input : NONE
 *
 * Output: NONE
 *
 **********************************************************************
 */
void patriot_ds3170_reg_write(void)
{
    ushort offset = 0;
    uchar temp = 0;

    printf("\npatriot_ds3170_register_write\n");
    offset = gethex_answer("Register Offset: ", 0, 0, 0xffff);
    temp = gethex_answer("Register Value: ", temp, 0, 0xff);
    ds3170_write(temp, (uint)offset);

    //#ifdef DEBUG
    ds3170_read (&temp, (uint)offset);
    printf("\n Read Back DS3170 Register @%#x = %#x\n", offset, temp);
    //#endif

}


/**********************************************************************
 *
 * Function: patriot_ds3170_dump_reg
 *
 * This function displays the framer register
 *
 * Input : reg_type - register type
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_ds3170_dump_reg(uchar reg_type)
{
    reg_info_t *reg_ptr;

    printf("\npatriot_ds3170_dump_reg\n");

    switch (reg_type) {
    case DS3170_GLOBAL:
        printf("\nDump DS3170 Global Register\n");
        reg_ptr = (reg_info_t *)ds3170_gl_regs;
        break;
    case DS3170_PORT:
        printf("\nDump DS3170 Port Register\n");
        reg_ptr = (reg_info_t *)ds3170_port_regs;
        break;
    case DS3170_BERT:
        printf("\nDump DS3170 BERT Register\n");
        reg_ptr = (reg_info_t *)ds3170_bert_regs;
        break;
    case DS3170_LINE:
        printf("\nDump DS3170 Line Register\n");
        reg_ptr = (reg_info_t *)ds3170_line_regs;
        break;
    case DS3170_HDLC:
        printf("\nDump DS3170 HDLC Register\n");
        reg_ptr = (reg_info_t *)ds3170_hdlc_regs;
        break;
    case DS3170_FEAC:
        printf("\nDump DS3170 FEAC Register\n");
        reg_ptr = (reg_info_t *)ds3170_feac_regs;
        break;
    case DS3170_TT:
        printf("\nDump DS3170 Trail Trace Register \n");
        reg_ptr = (reg_info_t *)ds3170_tt_regs;
        break;
    case DS3170_T3:
        printf("\nDump DS3170 DS3 Register\n");
        if (ds3170_select_frame_mode(MODE_T3) == FAILED){
            printf("\n ds3170_select_frame_mode(), failed\n");
            return (FAILED);
        }
        reg_ptr = (reg_info_t *)ds3170_t3_regs;
        break;
    case DS3170_E3G751:
        printf("\nDump DS3170 E3 G751 Register\n");
        if (ds3170_select_frame_mode(MODE_E3) == FAILED){
            printf("\n ds3170_select_frame_mode(), failed\n");
            return (FAILED);
        }
        reg_ptr = (reg_info_t *)ds3170_e3g751_regs;
        break;
    case DS3170_E3G832:
        printf("\nDump DS3170 E3 G832 Register\n");
        reg_ptr = (reg_info_t *)ds3170_e3g832_regs;
        break;
    default:
         printf("\npatriot_ds3170_dump_reg, not correct reg_type\n");
         return (FAILED);
    }

    return (register_display(reg_ptr, SPI_BUS));

}


/**********************************************************************
 *
 * Function: patriot_ds3170_alter_reg
 *
 * This function provide for ds3170 utilites to alter register
 *
 * Input : reg_type - register type
 *
 * Output: NONE
 *
 **********************************************************************
 */
int patriot_ds3170_alter_reg(uchar reg_type)
{
    reg_info_t *reg_ptr;

    printf("\npatriot_ds3170_alter_reg\n");

    switch (reg_type) {
    case DS3170_GLOBAL:
        printf("\nAlter DS3170 Global Register\n");
        reg_ptr = (reg_info_t *)ds3170_gl_regs;
        break;
    case DS3170_PORT:
        printf("\nAlter DS3170 Port Register\n");
        reg_ptr = (reg_info_t *)ds3170_port_regs;
        break;
    case DS3170_BERT:
        printf("\nAlter DS3170 BERT Register\n");
        reg_ptr = (reg_info_t *)ds3170_bert_regs;
        break;
    case DS3170_LINE:
        printf("\nAlter DS3170 Line Register\n");
        reg_ptr = (reg_info_t *)ds3170_line_regs;
        break;
    case DS3170_HDLC:
        printf("\nAlter DS3170 HDLC Register\n");
        reg_ptr = (reg_info_t *)ds3170_hdlc_regs;
        break;
    case DS3170_FEAC:
        printf("\nAlter DS3170 FEAC Register\n");
        reg_ptr = (reg_info_t *)ds3170_feac_regs;
        break;
    case DS3170_TT:
        printf("\nAlter DS3170 Trail Trace Register \n");
        reg_ptr = (reg_info_t *)ds3170_tt_regs;
        break;
    case DS3170_T3:
        printf("\nAlter DS3170 DS3 Register\n");
        if (ds3170_select_frame_mode(MODE_T3) == FAILED){
            printf("\n ds3170_select_frame_mode(), failed\n");
            return (FAILED);
        }
        reg_ptr = (reg_info_t *)ds3170_t3_regs;
        break;
    case DS3170_E3G751:
        printf("\nAlter DS3170 E3 G751 Register\n");
        if (ds3170_select_frame_mode(MODE_E3) == FAILED){
            printf("\n ds3170_select_frame_mode(), failed\n");
            return (FAILED);
        }
        reg_ptr = (reg_info_t *)ds3170_e3g751_regs;
        break;
    case DS3170_E3G832:
        printf("\nAlter DS3170 E3 G832 Register\n");
        reg_ptr = (reg_info_t *)ds3170_e3g832_regs;
        break;
    default:
         printf("\npatriot_ds3170_alter_reg, not correct reg_type\n");
         return (FAILED);
    }

    return (register_alter(reg_ptr, SPI_BUS));

}


/**********************************************************************
 *
 * Function: tx_ais
 *
 * This function will set or clear the AIS Tx Alarm
 *
 * Input : mode : T3 / E3
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int
tx_ais (uint onoff, uint mode)
{
    uchar temp = 0;

    printf("\ntx_ais\n");

    if(onoff) {
        printf("\n Set tx_ais\n");
        if (mode == MODE_T3) {
            if (ds3170_read(&temp, T3_TCR_ADDR_L)){
                printf("\n T3_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
            temp = temp | T3_TCR_TAIS;
            if (ds3170_write(temp, T3_TCR_ADDR_L)){
                printf("\n T3_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
        } else { /* E3 */
             /* G751 */
             if (ds3170_read(&temp, G751_TCR_ADDR_L)) {
                 printf("\n G751_TCR_ADDR_L Reg read fail\n");
                 return (FAILED);
             }
             temp = temp | G751_TCR_TAIS;
             if (ds3170_write(temp, G751_TCR_ADDR_L)){
                 printf("\n G751_TCR_ADDR_L Reg read fail\n");
                 return (FAILED);
             }
        }
    } else {
        printf("\n Clear tx_ais\n");
        if (mode == MODE_T3) {
            if (ds3170_read(&temp, T3_TCR_ADDR_L)){
                printf("\n T3_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
            temp = temp & (~T3_TCR_TAIS);
            if (ds3170_write(temp, T3_TCR_ADDR_L)){
                printf("\n T3_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
        } else { /* E3 */
            /* G751 */
            if (ds3170_read(&temp, G751_TCR_ADDR_L)) {
                printf("\n G751_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
            temp = temp & (~G751_TCR_TAIS);
            if (ds3170_write(temp, G751_TCR_ADDR_L)){
                printf("\n G751_TCR_ADDR_L Reg read fail\n");
                return (FAILED);
            }
        }
    }
}


/**********************************************************************
 *
 * Function: ds3170_init_clear_te3
 *
 * This function initializes the framer for clear operation.
 *
 * Input : mode - T3 / E3
 *
 * Output: PASSED / FAILED
 *
 **********************************************************************
 */
int ds3170_init_clear_te3(uint mode)
{
    uchar temp, i = 0;

    printf("\nds3170_init_clear_te3\n");

    /* Step 1: Check Device ID code , current id 0x004Fh */
    if (ds3170_read(&temp, IDR_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:IDR_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
    if (temp != 0x4F) { /* Device id 1 Codes 0x4Fh */
        sprintf(err_msg, "\n%s, [#%d]:Device id 1 is not correct, "
        		"expected 0x4F, read 0x%02x\n", __FUNCTION__, __LINE__, temp);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
    if (ds3170_read (&temp, IDR_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:IDR_ADDR_H Reg read fail\n",
        		__FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
    if ((temp != 0x00) && (temp != 0x10)) { /* Device id 2 Codes 0x00h or 0x10h */
        sprintf(err_msg, "\n%s, [#%d]:Device id 2 is not correct, "
        		"expected 0x00 or 0x10 , read 0x%02x\n", __FUNCTION__, __LINE__,temp);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    /* Step 2: Initialize the device
     * Global reset, Force all internal register to their default values.
     *
     * Notes: A Port reset is not necessary since the global reset includes a
     * reset of the port to its default values.
     */
    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp |= GCR1_RST;

    if (ds3170_write(temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg write fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]: GCR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= GCR1_RST;
     if(!temp) {
    	 sprintf(err_msg, "\n%s, [#%d]:Global Reset fail\n"
    			 , __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }



    /* Step 3: Clear the reset , default mode : C-Bit T3
     * LIU Disable */
    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp &= ~GCR1_RST;

    if (ds3170_write(temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg write fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]: GCR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= GCR1_RST;
     if(temp) {
    	 sprintf(err_msg, "\n%s, [#%d]: force all internal reg fail\n"
    			 , __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }

    
    
    /* Step 4: Clear the data path resets and the port power down bit
     * GL.CR1.RSTDP = 0;
     */
    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp &= ~GCR1_RSTDP;

    if (ds3170_write(temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg write fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
    /* PORT.CR1.RSTDP = 0,
     * PORT.CR1.PD = 0
     */
    if (ds3170_read(&temp, CR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp &= ~(CR1_RSTDP | CR1_PD);

    if (ds3170_write(temp, CR1_ADDR_L)) {
       sprintf(err_msg, "\n%s, [#%d]:Pwr up and unreset data path, "
    		   "CR1_ADDR_L Reg write fail\n", __FUNCTION__, __LINE__);
       print_err(FALSE, err_msg, LVL_2);
       return (FAILED);
    }

    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]: GCR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= GCR1_RSTDP;
     if(temp) {
    	 sprintf(err_msg, "\n%s, [#%d]: clear the  data path resets reg fail\n"
    			 , __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }

     if (ds3170_read(&temp, CR1_ADDR_L)) {
         sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		 , __FUNCTION__, __LINE__);
         print_err(FALSE, err_msg, LVL_2);
         return (FAILED);
     }
#ifdef DEBUG
     printf("\n%s, [#%d]: CR1_ADDR_L contents = 0x%02x", __FUNCTION__,
    	   __LINE__, temp);
#endif
     temp &= (CR1_RSTDP | CR1_PD);
     if(temp) {
    	 sprintf(err_msg, "\n%s, [#%d]: port power down reg fail\n",
    			 __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }

    
    /* Step 5: Configure the CLAD
     * set the CLAD to REFCLK (INPUT) 44.736 MHz */
    if (ds3170_read(&temp, GCR2_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR2_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp |= GCR2_CLAD(0);

    if (ds3170_write(temp, GCR2_ADDR_L)) {
       sprintf(err_msg, "\n%s, [#%d]:Config CLAD, GCR2_ADDR_L Reg write fail\n"
    		   , __FUNCTION__, __LINE__);
       print_err(FALSE, err_msg, LVL_2);
       return (FAILED);
    }

    if (ds3170_read(&temp, GCR2_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR2_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]: GCR2_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= GCR2_CLAD(0);
     if(temp) {
    	 sprintf(err_msg, "\n%s, [#%d]:Config CLAD reg fail\n"
    			 , __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }
    
    /* Step 6: Select the clock source for the transmitter
     * CLAD source : Set PORT.CR3.CLADC = 0
     * CLAD Transmit Clock Source Control
     * Use CLAD clocks for the transmit clock as appropriate
     */
    if (ds3170_read(&temp, CR3_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:CR3_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp &= ~CR3_CLADC;

    if (ds3170_write(temp, CR3_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:Using CLAD for the tx clock, "
        		"CR3_ADDR_L Reg write fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    if (ds3170_read(&temp,  CR3_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:CR3_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
    printf("\n%s, [#%d]:  CR3_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= CR3_CLADC;
     if(temp) {
    	 sprintf(err_msg, "\n%s, [#%d]:select CLAD source reg fail\n"
    			 , __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }

    /* Step 7: Configure the framing mode and the line mode
     * PORT.CR2.LM[2:0] = 010 (LIU on, JA in tx side) */
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_H Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
    temp |= CR2_LM(0x2);

    if (ds3170_write(temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:Config LIU on, JA rx, "
        		"CR2_ADDR_H Reg write fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]: CR2_ADDR_H Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]:  CR2_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= CR2_LM(0x2);
     if(!temp) {
    	 sprintf(err_msg, "\n%s, [#%d]: Config frame mode "
    			 "(LIU on, JA in tx side) reg fail\n", __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }
     
     if (ds3170_select_frame_mode(mode) == FAILED){
	 sprintf(err_msg, "\n%s, [#%d]:ds3170_select_frame_mode(), failed\n"
			 , __FUNCTION__, __LINE__);
	 print_err(FALSE, err_msg, LVL_2);
	 return (FAILED);
     }
     
    /* Step 8: Disable PAIS and LINE AIS*/
    if (ds3170_read(&temp,  CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR1_ADDR_H Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp |= (CR1_PAIS(0x7) | CR1_LAIS(0x3));

    if (ds3170_write(temp, CR1_ADDR_H)) {
       sprintf(err_msg, "\n%s, [#%d]:PAIS + LINE AIS, CR1_ADDR_H Reg"
    		   " write fail\n", __FUNCTION__, __LINE__);
       print_err(FALSE, err_msg, LVL_2);
       return (FAILED);
    }

    if (ds3170_read(&temp,  CR1_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]: CR1_ADDR_H Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
     printf("\n%s, [#%d]:  CR1_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
     temp &= (CR1_PAIS(0x7) | CR1_LAIS(0x3));
     if(!temp) {
    	 sprintf(err_msg, "\n%s, [#%d]:Disable PAIS and LINE AIS reg "
    			 "fail\n", __FUNCTION__, __LINE__);
    	 print_err(FALSE, err_msg, LVL_2);
    	 return (FAILED);
     }
    
    /* Step 9: Enable port normal operation
     */
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:Disable the tx line interface, "
        		"CR2_ADDR_H Reg read fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp |= CR2_TLEN;

    if (ds3170_write(temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:Disable the tx line interface, "
        		"CR2_ADDR_H Reg write fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
     }

    if (ds3170_read(&temp,  CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_H Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
    printf("\n%s, [#%d]:  CR2_ADDR_H contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
    temp &= CR2_TLEN;
    if(!temp) {
   	    sprintf(err_msg, "\n%s, [#%d]: Enable port normal operate reg fail\n"
   	    		, __FUNCTION__, __LINE__);
   	    print_err(FALSE, err_msg, LVL_2);
   	    return (FAILED);
    }
    
    if (ds3170_read(&temp, GCR1_ADDR_L)) {
	sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
			, __FUNCTION__, __LINE__);
	print_err(FALSE, err_msg, LVL_2);
	return (FAILED);
    }
    /* clear on read */
    temp |=  GCR1_LSBCRE;

    /*  Latched Status Bit Clear on Read Enable */
    if(ds3170_write(temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_LSBCRE Reg write fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    if (ds3170_read(&temp,  GCR1_ADDR_L )) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }
#ifdef DEBUG
    printf("\n%s, [#%d]:  GCR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
    temp &= GCR1_LSBCRE;

    if(!temp) {
   	 sprintf(err_msg, "\n%s, [#%d]:Latched status bit set clear on "
   			 "read reg fail\n", __FUNCTION__, __LINE__);
   	 print_err(FALSE, err_msg, LVL_2);
   	 return (FAILED);
    }

    /* The loopback cable is less than 225 feet long so set bit PORT.CR2.TLBO */
    if (ds3170_read(&temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:Read bit TLBO, "
        		"CR2_ADDR_H Reg read fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
    }

    temp |= CR2_TLBO;

    if (ds3170_write(temp, CR2_ADDR_H)) {
        sprintf(err_msg, "\n%s, [#%d]:Set bit TLBO, "
        		"CR2_ADDR_H Reg write fail\n", __FUNCTION__, __LINE__);
        print_err(FALSE, err_msg, LVL_2);
        return (FAILED);
     }
    
    return (PASSED);
}


/**********************************************************************
 *
 * Function: ds3170_register_test
 *
 * This function wrapper for register test on DS3170
 *
 * Input : base_addr - framer chip base address
 *
 * Output: NONE
 *
 **********************************************************************
 */
int ds3170_register_test(void)
{
    int ret = TO_HOST_DS3170_REG_TEST_OK;

    if (register_tests(ds3170_gl_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fail...\n",
    			__FUNCTION__, __LINE__, "ds3170_gl_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s pass...\n", "ds3170_gl_regs");
    /* Port reg */
    if (register_tests(ds3170_port_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n",
    			__FUNCTION__, __LINE__, "ds3170_port_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("%s, [#%d]:Reg test %s passes...\n", __FUNCTION__, __LINE__,
    		"ds3170_port_regs");
    /* BERT reg */
    /* Need to Init DS3170 by unreset before run reg BERT test */

    /* Unreset DS3170 */
    if (ds3170_unreset() == FAILED) {
        printf("\n%s, [#%d]:ds3170_unreset(), failed\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }

    if (register_tests(ds3170_bert_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n",
    			__FUNCTION__, __LINE__, "ds3170_bert_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s passes...\n", "ds3170_bert_regs");
    /* Line reg */
    if (register_tests(ds3170_line_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n",
    			__FUNCTION__, __LINE__, "ds3170_line_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s passes...\n", "ds3170_line_regs");
    /* HDLC reg */
    if (register_tests(ds3170_hdlc_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n",
    			__FUNCTION__, __LINE__, "ds3170_hdlc_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s passes...\n", "ds3170_hdlc_regs");

    /* T3 reg (DS3 CBIT) */
    if (ds3170_select_frame_mode(MODE_T3) == FAILED){
        sprintf(err_msg, "\n%s, [#%d]:ds3170_select_frame_mode(), failed\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }

    if (register_tests(ds3170_t3_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n", __FUNCTION__,
    			__LINE__, "ds3170_t3_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s passes...\n", "ds3170_t3_regs");

    /* E3 G751 reg */
    if (ds3170_select_frame_mode(MODE_E3) == FAILED){
        sprintf(err_msg, "\n%s, [#%d]:ds3170_select_frame_mode(), failed\n",
        		__FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }

    if (register_tests(ds3170_e3g751_regs, SPI_BUS)) {
    	sprintf(err_msg, "%s, [#%d]:Reg test %s fails...\n",
    			__FUNCTION__, __LINE__, "ds3170_e3g751_regs");
    	print_err(TRUE, err_msg, LVL_0);
        ret = TO_HOST_DS3170_REG_TEST_FAIL;
    }
    printf("Reg test %s passes...\n", "ds3170_e3g751_regs");

    return (ret);
}


/**********************************************************************
 *
 * Function: ds3170_unreset
 *
 * This function unreset on DS3170 chips for global and port reset bit
 *
 * Input : NONE
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
int ds3170_unreset (void)
{
	uchar temp = 0;

    if (ds3170_read(&temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

    temp &= ~(GCR1_RST | GCR1_RSTDP);

    if (ds3170_write(temp, GCR1_ADDR_L)) {
        sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg write fail\n"
        		, __FUNCTION__, __LINE__);
        print_err(TRUE, err_msg, LVL_0);
        return (FAILED);
    }

	/* PORT.CR1.RSTDP = 0,
	* PORT.CR1.PD = 0
	*/
	if (ds3170_read(&temp, CR1_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:GCR1_ADDR_L Reg read fail\n"
				, __FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
		return (FAILED);
	}

	temp &= ~(CR1_RSTDP | CR1_PD);

	if (ds3170_write(temp, CR1_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:Pwr up and unreset data path, CR1_ADDR_L "
				"Reg write fail\n", __FUNCTION__, __LINE__);
		print_err(TRUE, err_msg, LVL_0);
		return (FAILED);
	}

#ifdef DEBUG
	if (ds3170_read(&temp, GCR1_ADDR_L)) {
		printf("\nGCR1_ADDR_L Reg read fail\n");
		return (FAILED);
	}
	printf("\n%s, [#%d]: GCR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	__LINE__, temp);

	if (ds3170_read(&temp, CR1_ADDR_L)) {
		printf("\nGCR1_ADDR_L Reg read fail\n");
		return (FAILED);
	}
	printf("\n%s, [#%d]: CR1_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif

	return (PASSED);
}


/**********************************************************************
 *
 * Function: ds3170_select_frame_mode
 *
 * This function select the frame mode Default is DS3 C-Bit
 *
 * Input : NONE
 *
 * Output: PASSED or FAILED
 *
 **********************************************************************
 */
int ds3170_select_frame_mode (uint mode)
{
	uchar temp = 0;

	/* PORT.CR2.FM[2:0]
	 */
	if (mode == MODE_T3) {
		/* Set Framer mode for T3 CBIT*/
		if (ds3170_read(&temp, CR2_ADDR_L)) {
			sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n",
					__FUNCTION__, __LINE__);
			print_err(FALSE, err_msg, LVL_3);
			return (FAILED);
		}
		temp |= CR2_FM(0);

		if(ds3170_write(temp, CR2_ADDR_L)) {
			sprintf(err_msg, "\n%s, [#%d]:Config Framer mode T3 CBIT, "
					"CR2_ADDR_H Reg write fail\n", __FUNCTION__, __LINE__);
			print_err(FALSE, err_msg, LVL_3);
			return (FAILED);
		}

	if (ds3170_read(&temp,  CR2_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n",
				__FUNCTION__, __LINE__);
		print_err(FALSE, err_msg, LVL_3);
		return (FAILED);
	}
#ifdef DEBUG
	 printf("\n%s, [#%d]:  CR2_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
	 temp &= CR2_FM(0);
	 if(temp) {
		 sprintf(err_msg, "\n%s, [#%d]:Config T3 mode reg fail\n",
				 __FUNCTION__, __LINE__);
		 print_err(FALSE, err_msg, LVL_3);
		 return (FAILED);
	 }

	}else {
		/* Set Framer mode for E3 G751 */
		if (ds3170_read(&temp, CR2_ADDR_L)) {
			sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n",
					__FUNCTION__, __LINE__);
			print_err(FALSE, err_msg, LVL_3);
			return (FAILED);
		}
		temp |= CR2_FM(0x2);

		if(ds3170_write(temp, CR2_ADDR_L)) {
			sprintf(err_msg, "\n%s, [#%d]:Config Framer mode E3 G751, "
					"CR2_ADDR_H Reg write fail\n", __FUNCTION__, __LINE__);
			print_err(FALSE, err_msg, LVL_3);
			return (FAILED);
		}

	if (ds3170_read(&temp,  CR2_ADDR_L)) {
		sprintf(err_msg, "\n%s, [#%d]:CR2_ADDR_L Reg read fail\n",
				__FUNCTION__, __LINE__);
		print_err(FALSE, err_msg, LVL_3);
		return (FAILED);
	}
#ifdef DEBUG
	 printf("\n%s, [#%d]:  CR2_ADDR_L contents = 0x%02x", __FUNCTION__,
	   __LINE__, temp);
#endif
	 temp &= CR2_FM(0x2);
	 if(!temp) {
		 sprintf(err_msg, "\n%s, [#%d]:Config E3 mode reg fail\n",
				 __FUNCTION__, __LINE__);
		 print_err(FALSE, err_msg, LVL_3);
		 return (FAILED);
	 }

	}



	return (PASSED);

}

/*------------------------------------------------------------------------------
 * $Log: ds3170.c,v $
 * Revision 1.1  2014/03/25 02:12:32  huanngo
 * Adding patriot_linux directory to ng_diag code tree
 *
 * Revision 1.6  2013/07/03 02:12:34  steja
 * Add support new maxim chips DS3177 ID (0x10)
 *
 * Revision 1.5  2012/12/13 00:48:24  huanngo
 * Setting PORT_CR2 TLBO bit for loopback cable < 225 ft
 *
 * Revision 1.4  2012/12/03 12:35:16  steja
 * 1. Add Error message utility
 * 2. Fix Framer interrupt Diagnostic loopback
 *
 * Revision 1.3  2012/10/16 07:43:42  steja
 * Improve the DS3170 init to verify the value after set.
 *
 * Revision 1.2  2012/05/08 23:52:54  huanngo
 * Support SM Patriot on ngd main code tree
 *
 * Revision 1.1.4.10  2012/02/06 22:29:04  huanngo
 * Update to not compile code using bitbake, use make with local kernel
 *
 * Revision 1.1.4.9  2011/12/08 15:07:11  steja
 * Update IO Test function
 *
 * Revision 1.1.4.8  2011/11/24 00:40:01  huanngo
 * Update code for Patriot to fix bugs and support new tests
 *
 * Revision 1.1.4.7  2011/11/15 14:05:53  steja
 * Update DS3170 code
 * 1. Fix the AIS test
 * 2. Register BERT test
 *
 * Revision 1.1.4.6  2011/11/11 16:07:43  steja
 * Update the DS3170 code to read back the register after write.
 *
 * Revision 1.1.4.5  2011/10/27 09:35:08  steja
 * Update DS3170 BERT test
 *
 * Revision 1.1.4.4  2011/10/11 01:51:29  steja
 * Update DS3170 Register test code
 *
 * Revision 1.1.4.3  2011/09/20 10:10:56  steja
 * Update DS3170 code for AIS and BERT register
 *
 * Revision 1.1.4.2  2011/08/18 19:43:22  huanngo
 * Update code to patriot2-branch
 *
 * Revision 1.1.2.18  2011/08/06 00:17:39  huanngo
 * Update code for Patriot
 *
 * Revision 1.1.2.17  2011/08/03 01:51:35  steja
 * Update DS3170 code :
 * 1. Init DS3170
 * 2. Loopback test
 *
 * Revision 1.1.2.16  2011/07/26 14:35:51  steja
 * Update DS3170 code
 *
 * Revision 1.1.2.15  2011/07/21 12:14:15  steja
 * Update DS3170 functionality
 *
 * Revision 1.1.2.14  2011/07/08 10:38:18  steja
 * Clean up code
 *
 * Revision 1.1.2.13  2011/07/07 16:21:54  steja
 * 1. Clean up code
 * 2. Add check statur register after loopback test for DS3170.
 *
 * Revision 1.1.2.12  2011/07/01 15:39:07  steja
 * 1. Update DS3170 utility test code
 * 2. Update Internal and External loopback test for DS3170
 *
 * Revision 1.1.2.11  2011/06/30 16:31:42  steja
 * 1. Update DS3170 Register table
 * 2. Update DS3170 patriot_clear_t3_intr_test
 *
 * Revision 1.1.2.10  2011/06/29 16:24:55  steja
 * Update DS3170 code.
 *
 * Revision 1.1.2.9  2011/06/28 16:59:50  steja
 * 1. Update FPGA register read and write function
 * 2. Update DS3170 register test function
 * 3. Update Common register test, reg alter, reg display
 *
 * Revision 1.1.2.8  2011/06/28 06:27:55  huanngo
 * Update code to support Patriot SM
 *
 * Revision 1.1.2.7  2011/06/27 14:14:07  steja
 * 1. Update FPGA register test function
 * 2. Add FPGA dump register function
 * 3. Add FPGA register read / write utility function
 * 4. Add FPGA initialization function
 *
 * Revision 1.1.2.6  2011/06/13 06:48:56  steja
 * Update code for DS3170 and FPGA
 *
 * Revision 1.1.2.5  2011/06/09 07:03:37  steja
 * Update the code for DS3170 and FPGA's Patriot
 *
 * Revision 1.1.2.4  2011/05/26 03:29:16  steja
 * Fix the checkin log
 *
 *------------------------------------------------------------------------------
 * $Endlog$
 *
 *------------------------------------------------------------------------------
 */

