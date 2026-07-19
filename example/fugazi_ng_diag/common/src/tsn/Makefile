# $Id: Makefile,v 1.19 2021/06/02 02:56:25 alpeng Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tsn/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
# Feb 2016, Sofian Teja adapted from PPC Xformers.
#
# Copyright (c) 2017 - 2019 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#
###################################################################
#
# NOTE: When new files are added please sort it properly.
#
###################################################################
#
# NOTE: Need to build on a Linux box.
#

default: all

#
# Tool Chain, Please modify in .config
#

# Using the local config
# ifeq ($(TSN_DIAG_CROSS_COMPILE),)
# include .config
# export PATH :=$(TSN_DIAG_CROSS_COMPILE_PATH):$(PATH)
# endif

# Using toolchain on Cisco server
#TSN_DIAG_CROSS_COMPILE_PATH	= /auto/srg-diag-xcompiler/Marvell/TSN/Tool/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20160301/bin/
TSN_DIAG_CROSS_COMPILE_PATH    = /auto/srg-diag-xcompiler/APM/ESPN/Toolchain/LE/apm-aarch64/6.0.4/bin/
#TSN_DIAG_CROSS_COMPILE		= $(TSN_DIAG_CROSS_COMPILE_PATH)aarch64-marvell-linux-gnu-
TSN_DIAG_CROSS_COMPILE		= $(TSN_DIAG_CROSS_COMPILE_PATH)aarch64-apm-linux-gnu-

CC				= $(TSN_DIAG_CROSS_COMPILE)gcc
AS				= $(TSN_DIAG_CROSS_COMPILE)as
AR				= $(TSN_DIAG_CROSS_COMPILE)ar
LD				= $(TSN_DIAG_CROSS_COMPILE)ld
NM				= $(TSN_DIAG_CROSS_COMPILE)nm
OBJCOPY			= $(TSN_DIAG_CROSS_COMPILE)objcopy
OBJDUMP			= $(TSN_DIAG_CROSS_COMPILE)objdump
SIZE			= $(TSN_DIAG_CROSS_COMPILE)size
STRIP			= $(TSN_DIAG_CROSS_COMPILE)strip
MAKE			= make 

LDFLAGS =  -lpthread 
.PHONY: all clean depend

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

TSNDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		    = $(IROOT)/common/chips
COMSRCDIR	    = $(IROOT)/common/src
CHIPINC		    = $(DEV_DRV)/include
EEPROM_SRC	    = $(DEV_DRV)/dev_at24c0n_atmel
RTC_SRC		    = $(DEV_DRV)/dev_1337_maxim
GE_PHY_SRC	    = $(DEV_DRV)/dev_88e111x_marvell
PCA9555_SRC     = $(DEV_DRV)/dev_pca9555
PCA9557_SRC     = $(DEV_DRV)/dev_pca9557
TMPX75_SRC      = $(DEV_DRV)/dev_tmpx75
NXP_LM75B_SRC   = $(DEV_DRV)/dev_nxp_lm75b
LTE_SWI_SRC = $(DEV_DRV)/dev_lte_swi
TELIT_LTE_SRC   = $(DEV_DRV)/dev_lte_telit
NR_5G_TELIT_SRC = $(DEV_DRV)/dev_NR_5G_telit
PLUG_COMMON_SRC = ../plug_common
PLUG_TC_SRC     = ../plug_testcard
PLUG_LTE_SRC    = ../plug_lte
PLUG_NR_5G_SRC  = ../plug_NR_5G
PLUG_LTE_TELIT_SRC = $(PLUG_LTE_SRC)/plug_lte_telit
PLUG_NR_5G_TELIT_SRC = $(PLUG_NR_5G_SRC)/plug_NR_5G_telit
PLUG_SERIAL_SRC = ../plug_serial
USB_DONG_DIR         = ../usb_dongle
USB_DONG_COMMON_SRC  = $(USB_DONG_DIR)/usb_dongle_common
USB_DONG_LTE_SWI_SRC = $(USB_DONG_DIR)/usb_dongle_lte_swi
LTC_SRC		    = $(DEV_DRV)/dev_ltc4215
RBCPDIR		    = $(COMSRCDIR)/rbcp_lib

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(TSNDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(EEPROM_SRC) \
	-I$(RTC_SRC) \
	-I$(LIB_TAM_ACT2_SRC) \
	-I$(COMSRCDIR) \
	-I$(GE_PHY_SRC) \
	-I$(PCA9555_SRC) \
	-I$(PCA9557_SRC) \
	-I$(TMPX75_SRC) \
	-I$(NXP_LM75B_SRC) \
	-I$(LTE_SWI_SRC) \
	-I$(PLUG_COMMON_SRC) \
	-I$(PLUG_TC_SRC) \
	-I$(PLUG_LTE_SRC) \
	-I$(PLUG_LTE_TELIT_SRC) \
	-I$(PLUG_SERIAL_SRC)\
	-I$(USB_DONG_COMMON_SRC)\
	-I$(USB_DONG_LTE_SWI_SRC)\
	-I$(LTC_SRC) \
	-I$(RBCPDIR) \
	-I$(TELIT_LTE_SRC) \
	-I$(PLUG_NR_5G_TELIT_SRC) \
	-I$(NR_5G_TELIT_SRC) \

#
# defines for compiler
#

DEFS = \
	-DTSN \
	-DLINUX_APP \
    -DTFTP_CLIENT  \
	-DAIKIDO_ACT2 \
    -DSREC2BIN \
	-DTSN_CSCvg25685_AIKIDO_TAM_WDC_ISSUE_WORKAROUND \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.
#-DNOT_GFAST 
    #-DAIKIDO_RESET 
    # -DOLD_SWI_FW
#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS}
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into TSN project
#
#
LIB_TAM_ACT2_SRC = /auto/sp-engops/diags/pld/act2lite/ARM/TSN/SUDI_2099_Aikido
LIB_TAM_ACT2 = $(LIB_TAM_ACT2_SRC)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing_diags.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= tsnlnx

#
# Common device driver objects
#
DEV_OBJS = \
	dev_object.o \
	dev_1337.o \
	dev_mrvl_ge.o \
	dev_pca9555.o \
	dev_pca9557.o \
	dev_tmpx75.o \
	dev_nxp_lm75b.o \
	dev_lte_telit.o \
	dev_lte_telit_at.o \
	dev_lte_swi.o \
	dev_lte_swi_at.o \
	dev_NR_5G_telit.o \
	dev_NR_5G_telit_at.o \


DEV_BASE = \
	$(DEV_DRV)/dev_object.c \
	$(RTC_SRC)/dev_1337.c \
	$(GE_PHY_SRC)/dev_mrvl_ge.c \
	$(PCA9555_SRC)/dev_pca9555.c \
	$(PCA9557_SRC)/dev_pca9557.c \
	$(TMPX75_SRC)/dev_tmpx75.c \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
	$(TELIT_LTE_SRC)/dev_lte_telit.c \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.c \
	$(LTE_SWI_SRC)/dev_lte_swi.c \
	$(LTE_SWI_SRC)/dev_lte_swi_at.c \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit.c \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.c \



DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(RTC_SRC)/dev_1337.h \
	$(GE_PHY_SRC)/dev_mrvl_ge.h \
	$(GE_PHY_SRC)/dev_1112.h \
	$(PCA9555_SRC)/dev_pca9555.h \
	$(PCA9557_SRC)/dev_pca9557.h \
	$(TMPX75_SRC)/dev_tmpx75.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(TELIT_LTE_SRC)/dev_lte_telit.h \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.h \
	$(LTE_SWI_SRC)/dev_lte_swi.h \
	$(LTE_SWI_SRC)/dev_lte_swi_at.h \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit.h \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.h \


#
# Pluggable objects
#

#
# Pluggable Common Files
#
PLUG_COMMON_OBJS = \
    plug_slot.o \
    plug_host_slot_modules.o \
    plug_gpio_exp_test.o \
    plug_gpio_exp_lib.o \
    plug_temp_sensor_test.o \
    plug_temp_sensor_lib.o \
    plug_common_host.o \
    plug_common_host_impl.o \
    plug_common_lib.o \

PLUG_COMMON_BASE = \
    $(PLUG_COMMON_SRC)/plug_slot.c \
    $(TSNDIR)/plug_host_slot_modules.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.c \
    $(PLUG_COMMON_SRC)/plug_common_host.c \
    $(PLUG_COMMON_SRC)/plug_common_lib.c \
    $(TSNDIR)/plug_common_host_impl.c \

PLUG_COMMON_HFILES = \
    $(PLUG_COMMON_SRC)/plug_slot.h \
    $(TSNDIR)/plug_host_slot_modules.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.h \
    $(PLUG_COMMON_SRC)/plug_common_host.h \
    $(PLUG_COMMON_SRC)/plug_common_lib.h \
    $(TSNDIR)/plug_common_host_impl.h \


#
# Pluggable Testcard
#
PLUG_TC_OBJS = \
    plug_testcard_gpio_exp_lib.o \
    plug_testcard_test.o \
    plug_testcard_util.o \
    plug_testcard_phy.o \
    plug_testcard_host.o \
    plug_testcard_usb_lib.o \
    plug_testcard_host_impl.o \

PLUG_TC_BASE = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.c \
    $(PLUG_TC_SRC)/plug_testcard_test.c \
    $(PLUG_TC_SRC)/plug_testcard_util.c \
    $(PLUG_TC_SRC)/plug_testcard_phy.c \
    $(PLUG_TC_SRC)/plug_testcard_host.c \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.c \
    $(TSNDIR)/plug_testcard_host_impl.c \

PLUG_TC_HFILES = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_test.h \
    $(PLUG_TC_SRC)/plug_testcard_util.h \
    $(PLUG_TC_SRC)/plug_testcard_phy.h \
    $(PLUG_TC_SRC)/plug_testcard_host.h \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.h \
    $(TSNDIR)/plug_testcard_host_impl.h \

#
# Pluggable LTE
# 
PLUG_LTE_OBJS = \
    plug_lte_test.o \
    plug_lte_util.o \
    plug_lte_lib.o \
    plug_lte_at.o \
    plug_lte_host.o \
    plug_lte_host_impl.o \

PLUG_LTE_BASE = \
    $(PLUG_LTE_SRC)/plug_lte_test.c \
    $(PLUG_LTE_SRC)/plug_lte_util.c \
    $(PLUG_LTE_SRC)/plug_lte_lib.c \
    $(PLUG_LTE_SRC)/plug_lte_at.c \
    $(PLUG_LTE_SRC)/plug_lte_host.c \
    $(TSNDIR)/plug_lte_host_impl.c \

PLUG_LTE_HFILES = \
    $(PLUG_LTE_SRC)/plug_lte_test.h \
    $(PLUG_LTE_SRC)/plug_lte_util.h \
    $(PLUG_LTE_SRC)/plug_lte_lib.h \
    $(PLUG_LTE_SRC)/plug_lte_at.h \
    $(PLUG_LTE_SRC)/plug_lte_host.h \
    $(TSNDIR)/plug_lte_host_impl.h \

#
# Pluggable LTE Telit
#
PLUG_LTE_TELIT_OBJS = \
    plug_lte_telit_test.o \
	plug_lte_telit_lib.o \
	plug_lte_telit_util.o \
	plug_lte_telit_host.o \

PLUG_LTE_TELIT_BASE = \
    $(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.c \

PLUG_LTE_TELIT_HFILES = \
    $(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.h \

PLUG_NR_5G_TELIT_OBJS = \
	plug_NR_5G_telit_test.o \
	plug_NR_5G_telit_lib.o \
	plug_NR_5G_telit_util.o \
	plug_NR_5G_telit_host.o \

PLUG_NR_5G_TELIT_BASE = \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_test.c \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_lib.c \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_util.c \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_host.c \

PLUG_NR_5G_TELIT_HFILES = \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_test.h \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_lib.h \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_util.h \
	$(PLUG_NR_5G_TELIT_SRC)/plug_NR_5G_telit_host.h \

#
# Pluggable Serial(host)
# 
PLUG_SERIAL_OBJS = \
    platform_plug_serial_test.o \
	platform_plug_serial_util.o \
	platform_plug_serial_host.o \
	plug_serial_host_impl.o

PLUG_SERIAL_BASE = \
    $(PLUG_SERIAL_SRC)/platform_plug_serial_test.c \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_util.c \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_host.c \
	$(TSNDIR)/plug_serial_host_impl.c

PLUG_SERIAL_HFILES = \
    $(PLUG_SERIAL_SRC)/platform_plug_serial_test.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_util.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_host.h \
	$(TSNDIR)/plug_serial_host_impl.h

PLUG_OBJS = \
    plug_host_fpga_lib.o \
	plug_host_fpga_prog.o \
	$(PLUG_COMMON_OBJS) \
	$(PLUG_TC_OBJS) \
	$(PLUG_LTE_OBJS) \
	$(PLUG_LTE_TELIT_OBJS) \
	$(PLUG_SERIAL_OBJS) \
	$(PLUG_NR_5G_TELIT_OBJS) \

PLUG_BASE = \
    $(TSNDIR)/plug_host_fpga_lib.c \
	$(TSNDIR)/plug_host_fpga_prog.c \
	$(PLUG_COMMON_BASE) \
	$(PLUG_TC_BASE) \
	$(PLUG_LTE_BASE) \
	$(PLUG_LTE_TELIT_BASE) \
	$(PLUG_SERIAL_BASE) \
	$(PLUG_NR_5G_TELIT_BASE) \

PLUG_HFILES = \
    $(TSNDIR)/plug_host_fpga_lib.h \
	$(TSNDIR)/plug_host_fpga_prog.h \
	$(PLUG_COMMON_HFILES) \
	$(PLUG_TC_HFILES) \
	$(PLUG_LTE_HFILES) \
	$(PLUG_LTE_TELIT_HFILES) \
	$(PLUG_SERIAL_HFILES) \
	$(PLUG_NR_5G_TELIT_HFILES) \

#
# Common USB Dongle
# 
USB_DONG_COMMON_OBJS = \
	usb_dongle_host_slot_modules.o \
    usb_dongle_common_test.o \
	usb_dongle_common_lib.o \
	usb_dongle_common_host.o \
	usb_dongle_common_host_impl.o \

USB_DONG_COMMON_BASE = \
    $(USB_DONG_COMMON_SRC)/usb_dongle_common_test.c \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_lib.c \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_host.c \
	$(TSNDIR)/usb_dongle_common_host_impl.c \
	$(TSNDIR)/usb_dongle_host_slot_modules.c \

USB_DONG_COMMON_HFILES = \
    $(USB_DONG_COMMON_SRC)/usb_dongle_common_test.h \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_lib.h \
	$(USB_DONG_COMMON_SRC)/usb_dongle_common_host.h \
	$(TSNDIR)/usb_dongle_common_host_impl.h \
	$(TSNDIR)/usb_dongle_host_slot_modules.h \
##
## LTE(SWI) USB Dongle
## 
USB_DONG_LTE_SWI_OBJS = \
	usb_dongle_lte_swi_test.o \
	usb_dongle_lte_swi_lib.o \
	usb_dongle_lte_swi_util.o \

USB_DONG_LTE_SWI_BASE = \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_test.c \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_lib.c \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_util.c \

USB_DONG_LTE_SWI_HFILES = \
	$(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_test.h \
    $(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_lib.h \
    $(USB_DONG_LTE_SWI_SRC)/usb_dongle_lte_swi_util.h \

USB_DONG_OBJS = \
	$(USB_DONG_COMMON_OBJS) \
	$(USB_DONG_LTE_SWI_OBJS) \

USB_DONG_BASE = \
	$(USB_DONG_COMMON_BASE) \
	$(USB_DONG_LTE_SWI_BASE) \

USB_DONG_HFILES = \
	$(USB_DONG_COMMON_HFILES) \
	$(USB_DONG_LTE_SWI_HFILES) \

#
# Common object files
#


COM_OBJS = \
	linux_main.o \
	linux_i2c.o \
    linux_error.o \
    linux_stub.o \
    linux_mem.o \
    linux_api.o \
    linux_coretest.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	pca.o \
	srec2bin.o
	
COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/menu.c \
    $(COMSRCDIR)/common_utils.c \
    $(COMSRCDIR)/print_hex.c \
    $(COMSRCDIR)/query.c \
    $(COMSRCDIR)/linux_pcie.c \
    $(COMSRCDIR)/memops.c \
    $(COMSRCDIR)/testmem.c \
    $(COMSRCDIR)/bit_swaps.c \
    $(COMSRCDIR)/tftpclient.c \
    $(COMSRCDIR)/cookie_4_core.c \
	$(COMSRCDIR)/tam_act2_utils.c \
	$(COMSRCDIR)/tam_aikido_upgrade.c \
	$(COMSRCDIR)/pca.c \
	$(COMSRCDIR)/srec2bin.c \
	$(RBCPDIR)/rbcp_lib.c 

COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_coretest.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
    $(INCDIR)/common_utils.h \
    $(INCDIR)/linux_usb_test.h \
    $(INCDIR)/i2c_api.h \
    $(INCDIR)/i2c_dev.h \
    $(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing.h \
	$(LIB_TAM_ACT2_SRC)/tam_library.h \
    $(LIB_TAM_ACT2_SRC)/tam_lib_object_manager.h \
    $(LTC_SRC)/dev_ltc4215.h

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files :
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	i2c_api.o \
	platform_i2c.o \
	platform_storage.o \
	platform_sensor.o \
	platform_eeprom.o \
	platform_rtc.o \
	linux_popen.o \
    mb_tests.o \
    uart_api.o \
    mgmt_port.o \
    platform_stub.o \
    dsl_tests.o \
    dsl_libs.o \
    tsn_comm.o \
    linux_ntwk.o \
    platform_poe_psu.o \
    diag_poe_psu.o \
    platform_fpga.o \
    diag_usb_lib.o \
    diag_fpga.o \
    diag_plug_fpga.o \
    lte.o \
    platform_cookie.o \
    platform_cpu.o \
    tam_act2_api_drv_support.o \
    platform_smi.o \
    platform_ext_lpbk.o \
    platform_sfp_ext_lpbk.o \
    platform_ge_phy.o \
    diag_ge_phy.o \
    diag_esw.o \
    platform_dimm.o \
    platform_mcu.o \
    diag_ext.o \
    platform_wifi.o \
    wifi_temp_sensor_lib.o \
    wifi_temp_sensor_test.o \
    wifi_tests.o \
    platform_led.o \
    tam_aikido_upgrade.o \
    platform_sfp_cookie.o \
    tam_lib_object_manager.o \
    platform_mcu_upgrade.o \
	full_load.o \
	$(PLUG_OBJS) \
	$(USB_DONG_OBJS) \
    platform_emmc.o \
    cmd_rom_ugd.o \
    diag_reset_button.o \
    gshdsl_tests.o \
    tsn_rbcp_main.o \
    rbcp_platform.o \
    rbcp_lib.o \
    rbcp_util.o \

PLAT_BASE = \
	$(TSNDIR)/linux_main.c \
	$(TSNDIR)/diag.c \
	$(TSNDIR)/i2c_api.c \
    $(TSNDIR)/platform_i2c.c \
    $(TSNDIR)/platform_storage.c \
    $(TSNDIR)/platform_sensor.c \
    $(TSNDIR)/platform_eeprom.c \
    $(TSNDIR)/platform_rtc.c \
    $(TSNDIR)/linux_popen.c \
	$(TSNDIR)/mb_tests.c \
	$(TSNDIR)/uart_api.c \
	$(TSNDIR)/mgmt_port.c \
    $(TSNDIR)/platform_stub.c \
    $(TSNDIR)/dsl_tests.c \
    $(TSNDIR)/dsl_libs.c \
    $(TSNDIR)/tsn_comm.c \
    $(TSNDIR)/linux_ntwk.c \
    $(TSNDIR)/platform_poe_psu.c \
    $(TSNDIR)/diag_poe_psu.c \
    $(TSNDIR)/platform_fpga.c \
    $(TSNDIR)/diag_usb_lib.c \
    $(TSNDIR)/diag_fpga.c \
    $(TSNDIR)/diag_plug_fpga.c \
    $(TSNDIR)/lte.c \
    $(TSNDIR)/tam_act2_api_drv_support.c \
	$(TSNDIR)/platform_cookie.c \
	$(TSNDIR)/platform_cpu.c \
    $(TSNDIR)/platform_smi.c \
    $(TSNDIR)/platform_ext_lpbk.c \
    $(TSNDIR)/platform_sfp_ext_lpbk.c \
    $(TSNDIR)/platform_ge_phy.c \
    $(TSNDIR)/diag_ge_phy.c \
    $(TSNDIR)/diag_esw.c \
    $(TSNDIR)/platform_dimm.c \
    $(TSNDIR)/platform_mcu.c \
    $(TSNDIR)/diag_ext.c \
    $(TSNDIR)/platform_wifi.c \
    $(TSNDIR)/wifi_temp_sensor_lib.c \
    $(TSNDIR)/wifi_temp_sensor_test.c \
    $(TSNDIR)/wifi_tests.c \
    $(TSNDIR)/platform_led.c \
    $(TSNDIR)/platform_sfp_cookie.c \
    $(TSNDIR)/platform_mcu_upgrade.c \
	$(PLUG_BASE) \
	$(USB_DONG_BASE) \
    $(LIB_TAM_ACT2_SRC)/tam_lib_object_manager.c \
    $(TSNDIR)/platform_emmc.c \
    $(TSNDIR)/cmd_rom_ugd.c \
    $(TSNDIR)/diag_reset_button.c \
    $(TSNDIR)/gshdsl_tests.c \
    $(TSNDIR)/tsn_rbcp_main.c \
    $(TSNDIR)/rbcp_util.c \
    $(TSNDIR)/rbcp_platform.c 

PLAT_HFILES = \
    $(TSNDIR)/mb_tests.h \
    $(TSNDIR)/ngio.h \
    $(TSNDIR)/platform_sensor.h \
    $(TSNDIR)/platform_i2c.h \
	$(TSNDIR)/mon_plat_defs.h \
	$(TSNDIR)/pcmap.h \
	$(TSNDIR)/nvmonvars.h \
	$(TSNDIR)/object.h \
	$(TSNDIR)/platform_i2c.h \
	$(TSNDIR)/platform_rtc.h \
	$(TSNDIR)/uart_api.h \
	$(TSNDIR)/mgmt_port.h \
	$(TSNDIR)/platform_stub.h \
    $(TSNDIR)/dsl_tests.h \
    $(TSNDIR)/dsl_libs.h \
    $(TSNDIR)/tsn_comm.h \
    $(TSNDIR)/linux_ntwk.h \
    $(TSNDIR)/platform_poe_psu.h \
    $(TSNDIR)/diag_poe_psu.h \
    $(TSNDIR)/platform_fpga.h \
    $(TSNDIR)/diag_usb_lib.h \
    $(TSNDIR)/lte.h \
    $(TSNDIR)/act2_utils.h \
	$(TSNDIR)/act2l_typedef.h \
	$(TSNDIR)/platform_cookie.h \
	$(TSNDIR)/tam_act2_api_drv_support.h \
	$(TSNDIR)/mfg_api.h \
    $(TSNDIR)/platform_smi.h \
    $(TSNDIR)/platform_ge_phy.h \
    $(TSNDIR)/platform_ext_lpbk.h \
    $(TSNDIR)/diag_ge_phy.h \
    $(TSNDIR)/platform_dimm.h \
    $(TSNDIR)/platform_mcu.h \
    $(TSNDIR)/platform_wifi.h \
    $(TSNDIR)/wifi_temp_sensor_lib.h \
    $(TSNDIR)/wifi_temp_sensor_test.h \
    $(TSNDIR)/wifi_tests.h \
    $(TSNDIR)/platform_cpu.h \
    $(TSNDIR)/platform_fru.h \
    $(TSNDIR)/platform_mcu_upgrade.h \
	$(PLUG_HFILES) \
	$(USB_DONG_HFILES) \
    $(TSNDIR)/platform_emmc.h \
    $(TSNDIR)/full_load.h \
    $(TSNDIR)/cmd_rom_ugd.h \
    $(TSNDIR)/diag_reset_button.h \
    $(TSNDIR)/module_gshdsl.h \
    $(TSNDIR)/tsn_rbcp_main.h \
    $(RBCPDIR)/rbcp_lib.h \
    $(TSNDIR)/rbcp_util.h \
    $(TSNDIR)/rbcp_platform.h \
    $(TSNDIR)/platform_sfp_cookie.h 

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2)

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE)
#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(LTE_SWI_SRC): \
	$(TSNDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(LIB_TAM_ACT2_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(PLUG_SERIAL_SRC): \
	$(USB_DONG_COMMON_SRC): \
	$(USB_DONG_LTE_SWI_SRC): \
	$(RBCPDIR): \
	$(LIB_TAM_ACT2_SRC): \
	$(TELIT_LTE_SRC): \
	$(PLUG_NR_5G_TELIT_SRC): \
	$(NR_5G_TELIT_SRC): \

vpath %.h \
	$(GE_PHY_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(LTE_SWI_SRC): \
	$(TSNDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
    $(RTC_SRC): \
	$(LIB_TAM_ACT2_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(USB_DONG_COMMON_SRC): \
	$(USB_DONG_LTE_SWI_SRC): \
	$(RBCPDIR): \
	$(PLUG_SERIAL_SRC): \
	$(TELIT_LTE_SRC): \
	$(PLUG_NR_5G_TELIT_SRC): \
	$(NR_5G_TELIT_SRC): \

.SUFFIXES:
.SUFFIXES: .o .lis .S .c .s .dis

%.o : %.S
	$(CC) -c $(ASFLAGS) $<
%.o : %.c
	$(CC) -c $(CFLAGS) $<
%.lis : %.c
	lister3 CC=$(CC) AS=$(AS) $< $(CFLAGS) | grep -v '.stab' >$*.lis

%.dis : %.o
	$(OBJDUMP) -l --disassemble --source $< > $*.dis

%.i : %.c
	$(CC) -E $(CFLAGS) $< > $*.i

%.s : %.c
	$(CC) -S -fsource-asm $(CFLAGS) $< 

#
# No ROM version for informers
#

all: $(NETTARGET)

clean:
	rm -rf *.o banner.c \
	.compile ID.temp


TARGET_DEPEND = \
	Makefile \
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)
	
ENV_PRT :
	@echo "##########################################################"
	@echo "#################### Env Variables #######################"
	@echo "##########################################################"
	@echo "PATH: " $(PATH)
	@echo "TSN_DIAG_CROSS_COMPILE: " $(TSN_DIAG_CROSS_COMPILE)
	@echo "IROOT: " $(IROOT)
	@echo "TSNDIR: " $(TSNDIR)
	@echo "KLMDIR: " $(KLMDIR)
	@echo "DEV_DRV: " $(DEV_DRV)
	@echo "INCDIR: " $(INCDIR)
	@echo "COMSRCDIR: " $(COMSRCDIR)
	@echo "CFLAGS: " $(CFLAGS)
	@echo "##########################################################"

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : ENV_PRT $(TARGET_DEPEND) $(HFILES)
	sh $(IROOT)/utils/banner.sh TSN
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS}

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols
	
#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) > dependancies
	@echo "Computing TAGS file for common/src"
	$(shell which ctags) $(DEPEND_LST)
	@echo "Computing ID file for common/src"
	@mkid32 -s -m/sw/packages/id-utils/3.2d-p4/share/id-lang.map \
	 -lasm:-c\| -lasm:-u -oID.temp $(DEPEND_LST)
	
scope:
	cscope -b $(DEPEND_LST)

dependancies:
	rm -f dependancies
	touch dependancies

sinclude dependancies




# DO NOT DELETE THIS LINE -- make depend depends on it.

# *-------------------------------------------------
# $Log: Makefile,v $
# Revision 1.19  2021/06/02 02:56:25  alpeng
# merge sears into trunk
#
# Revision 1.18.34.1  2021/04/26 23:02:10  tshanmug
# Sear PIM support for TSN
#
# Revision 1.18  2019/10/16 02:27:15  sherliu2
# Fix CSCvq98193, disable auto-suspend feature for Star C1109-4P
#
# Revision 1.17  2019/06/14 09:59:28  steja
# Supported Cooper usb dongle LTE
#
# Revision 1.16  2019/05/14 09:18:32  sherliu2
# Support hyperloop
#
# Revision 1.15.4.2  2018/12/15 00:39:50  shjung
# Changed image name to tsnlnx_hp for Hyperloop project
#
# Revision 1.15.4.1  2018/12/13 19:19:27  shjung
# Supported Hyperloop PIM on Star platform
#
# Revision 1.15  2018/11/23 08:46:12  hondwang
# Re-instruct pluggable common code with CDETs CSCvn17216
#
# Revision 1.14  2018/11/12 06:36:39  iachang
# CSCvm79684:Fixed Aikido WDC USER ID delete issue with TAM lib. newer than V2.10.2.
#
# Revision 1.13  2018/10/18 06:36:59  iachang
# CSCvm79684:Rollback TAM lib. to V2.10.2, the TAM lib V3.3.27. will case the Aikido WDC program issue
#
# Revision 1.12  2018/09/21 02:51:01  iachang
# CSCvm54395: Upgraded TAM lib. to V3.3.27 and supported Act2 SUDI 2099
#
# Revision 1.11.10.2  2018/11/22 11:30:58  iachang
# Sync up with main trunk.
#
# Revision 1.11.10.1  2018/10/15 06:53:07  hondwang
# pluggable common code re-instruct modify code
#
# Revision 1.11  2018/08/06 02:37:59  harrchan
# Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)
#
# Revision 1.10  2018/07/10 00:28:08  lucywang
# Enhanced CPU core test to  make sure each CPU core is activated
#
# Revision 1.9  2018/05/24 09:47:10  steja
# CSCvj57981-Enhance SFP GLC-GE-100FX Support
#
# Revision 1.8  2018/05/09 06:53:12  letsai
# Add TSN GSHDSL portion
#
# Revision 1.7  2018/02/09 09:56:53  hondwang
# Merge Star branch star-branch-c9xx to main trunk
#
# Revision 1.6.6.1  2018/01/20 06:27:23  hondwang
# prepare merge star-branch-c9xx to main trunk
#
# Revision 1.6  2017/12/01 13:45:20  palin2
# Added support RESET button test (CSCvg96921).
#
# Revision 1.5  2017/10/19 14:04:28  palin2
# Added support to upgrade ROMMON to TSN Oct-2017 Pilot version, 16.6(1r).
#
# Revision 1.4  2017/10/19 13:53:05  palin2
# Fixed CSCvg25685, TSN: TAM FW wiping out the WDC object during
# power-cycle-with-software-running testing.
#
# Revision 1.3  2017/09/04 16:09:41  palin2
# Added utilities to enable fully eMMC pSLC mode and show eMMC info.(CSCvf82437)
#
# Revision 1.2.4.8  2017/12/13 09:31:58  hondwang
# Check WDC dummmy workaround function for HW reuqest
#
# Revision 1.2.4.7  2017/12/13 08:33:25  shjung
# Added diagnostic test mode for pluggable LTE-WP76xx GPS pin test
#
# Revision 1.2.4.6  2017/12/09 04:14:18  steja
# Support CISCO MCU upgrade
#
# Revision 1.2.4.5  2017/12/05 02:47:08  lucywang
# Sync from TSN trunk : Added support RESET button test (CSCvg96921).
#
# Revision 1.2.4.4  2017/09/20 08:18:09  lucywang
# added full load utility
#
# Revision 1.2.4.3  2017/09/04 15:13:15  shjung
# Add utility to enable eMMC pSLC
#
# Revision 1.2.4.2  2017/08/17 12:19:44  hondwang
# Set AIKIDO ACT2 by default
#
# Revision 1.2.4.1  2017/08/15 14:18:38  hondwang
# star branch c9xx initial check in
#
# Revision 1.2  2017/08/02 14:21:44  steja
# Support TSN-H/M platform code
#
# Revision 1.1.8.2  2017/07/29 03:40:58  steja
# tsn-branch5 synced with Maintrunk repositories
#
# Revision 1.1.6.5  2017/07/24 14:14:10  palin2
# 1. To improve code readability.
# 2. All changes are verified before check-in.
#
# Revision 1.1.6.4  2017/07/21 09:17:41  iachang
# clean up code
#
# Revision 1.1.6.3  2017/07/20 14:36:33  steja
# remove unused files
#
# Revision 1.1.6.2  2017/07/20 13:38:03  steja
# tsn-branch4 merge with maintrunk
#
# Revision 1.1.4.10.2.6  2017/07/17 14:41:00  steja
# code cleanup
#
# Revision 1.1.4.10.2.5  2017/07/11 13:46:07  steja
# 1. Add Check Motherboard Aikido cookie ID
# 2. Add Check Aikido I2C Scan
# 2. Add Check SFP present I2C Scan
#
# Revision 1.1.4.10.2.4  2017/07/11 10:13:08  steja
# 1. Remove Debugcard test
# 2. Add LTE micro usb utility to basic utilities
# 3. Code clean up
#
# Revision 1.1.4.10.2.3  2017/07/06 07:01:07  iachang
# Upgrade TAM lib. to V2.10.2
# Changed tam_lib_platform_spi_r/w() function name to tam_lib_platform_mbx_r/w()
#
# Revision 1.1.4.10.2.2  2017/04/14 00:52:47  steja
# Add define for GFAST
#
# Revision 1.1.4.10.2.1  2017/03/08 14:51:18  steja
# Update Makefile for ACT2 Programming library
#
# Revision 1.1.4.10  2016/11/25 08:11:42  steja
# Fix CSCvc13983: TSN-M VDSL bootup fail randomly during EEDVT
#
# Revision 1.1.4.9  2016/10/04 06:39:07  petteng
# Add enhanced error message
#
# Revision 1.1.4.8  2016/09/13 14:35:47  steja
# Commit Aikido / TAM Mailbox code
#
# Revision 1.1.4.7  2016/09/13 08:14:23  palin2
# Added CPU to GE PHY MAC loopback test.
#
# Revision 1.1.4.6  2016/09/08 13:45:19  steja
# Add Utilitiy to read SFP eeprom
#
# Revision 1.1.4.5  2016/08/09 09:47:54  iachang
# Supported FPGA/Aikido firmware upgrade.
#
# Revision 1.1.4.4  2016/08/01 02:32:22  steja
# Fix dependencies file size 0
#
# Revision 1.1.4.3  2016/07/10 10:29:31  steja
# Add LED test
#
# Revision 1.1.4.2  2016/06/30 06:22:47  steja
# tsn-branch2 sync with main trunk
#
# Revision 1.1.2.24  2016/06/28 02:08:55  steja
# Workaround make depend
#
# Revision 1.1.2.23  2016/06/21 04:36:33  palin2
# Added voltage margin utility and MCU register R/W utilities.
#
# Revision 1.1.2.22  2016/06/01 10:37:15  steja
# Change tsn_lnx to tsnlnx
#
# Revision 1.1.2.21  2016/05/20 03:37:38  leschen
# Support wifi
#
# Revision 1.1.2.20  2016/05/16 09:13:39  steja
# Add Extened feature for Bridge software
#
# Revision 1.1.2.19  2016/05/06 16:10:18  steja
# Bring up I2C-2 for RTC
#
# Revision 1.1.2.18  2016/04/29 10:14:56  palin2
# Updated code and added support ext. loopback test after bring up Switch.
#
# Revision 1.1.2.17  2016/04/26 20:48:49  palin2
# Updated code after bring up SFP external loopback test.
#
# Revision 1.1.2.16  2016/04/23 15:00:29  steja
# Check in for fix SPD Read RAW
#
# Revision 1.1.2.15  2016/04/22 12:28:36  palin2
# Updated code after bring up GE PHY external loopback test.
#
# Revision 1.1.2.14  2016/04/14 06:09:49  palin2
# 1. Removed cpld.c and cpld.h because TSN don't have CPLD.
# 2. Linked related function to correct FPGA one.
#
# Revision 1.1.2.13  2016/03/29 02:50:03  palin2
# Added GE PHY Diag.
#
# Revision 1.1.2.12  2016/03/25 08:05:40  steja
# Add CPU Stress tools
#
# Revision 1.1.2.11  2016/03/24 10:35:03  steja
# Add Cookie and Act2 programming
#
# Revision 1.1.2.10  2016/03/24 03:58:26  steja
# Add LTE test
#
# Revision 1.1.2.9  2016/03/23 03:31:11  palin2
# Added FPGA Diag.
#
# Revision 1.1.2.8  2016/03/22 22:19:12  palin2
# Added PoE PSU Diag.
#
# Revision 1.1.2.7  2016/03/22 09:40:49  leschen
# Add BCM63168 test items.
#
# Revision 1.1.2.6  2016/03/21 02:56:06  steja
# Add debug card test items
#
# Revision 1.1.2.5  2016/03/20 14:06:28  steja
# Add RTC test and utilities
#
# Revision 1.1.2.4  2016/03/20 05:32:40  steja
# 1. Add i2c scan and i2c read write utility
# 2. Add sensor temperature and eeprom utility
#
# Revision 1.1.2.3  2016/03/16 08:57:54  steja
# add usb test
#
# Revision 1.1.2.2  2016/03/14 14:32:03  steja
# Add memory test
#
# Revision 1.1.2.1  2016/03/08 09:55:10  steja
# Initial Check-in
#
#
#
