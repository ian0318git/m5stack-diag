# $Id: Makefile,v 1.7 2021/06/02 02:56:20 alpeng Exp $ 
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/betelgeuse/Makefile,v $
#------------------------------------------------------------------
#
# Makefile
#
# Copyright (c) 2019 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------

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
# ifeq ($(CROSS_COMPILE),)
# include .config
# export PATH :=$(CROSS_COMPILE_TOOL_PATH):$(PATH)
# endif

# Using toolchain on Cisco server
CROSS_COMPILE_TOOL_PATH = /auto/srg-diag-xcompiler/APM/ESPN/Toolchain/LE/apm-aarch64/6.0.4/bin/
CROSS_COMPILE		= $(CROSS_COMPILE_TOOL_PATH)aarch64-apm-linux-gnu-

CC				= $(CROSS_COMPILE)gcc
AS				= $(CROSS_COMPILE)as
AR				= $(CROSS_COMPILE)ar
LD				= $(CROSS_COMPILE)ld
NM				= $(CROSS_COMPILE)nm
OBJCOPY			= $(CROSS_COMPILE)objcopy
OBJDUMP			= $(CROSS_COMPILE)objdump
SIZE			= $(CROSS_COMPILE)size
STRIP			= $(CROSS_COMPILE)strip
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

PROJ_DIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
DEV_MAX31730	= $(DEV_DRV)/dev_maxim_max31730
GE_PHY_SRC	= $(DEV_DRV)/dev_n88e111x_marvell
POE_TPS2386X_SRC	= $(DEV_DRV)/dev_tps2386b
PCA9555_SRC = $(DEV_DRV)/dev_pca9555
PCA9557_SRC = $(DEV_DRV)/dev_pca9557
TMPX75_SRC  = $(DEV_DRV)/dev_tmpx75
NXP_LM75B_SRC  = $(DEV_DRV)/dev_nxp_lm75b
LTC_SRC		= $(DEV_DRV)/dev_ltc4215
RBCPDIR		= $(COMSRCDIR)/rbcp_lib
MRV_88E6176_SRC = $(DEV_DRV)/dev_88e6176_marvell
MRV_88E6390_SRC = $(DEV_DRV)/dev_88e6390_marvell
TELIT_LTE_SRC   = $(DEV_DRV)/dev_lte_telit
NR_5G_TELIT_SRC = $(DEV_DRV)/dev_NR_5G_telit
PLUG_COMMON_SRC = ../plug_common
PLUG_TC_SRC     = ../plug_testcard
PLUG_LTE_SRC    = ../plug_lte
PLUG_NR_5G_SRC  = ../plug_NR_5G
PLUG_LTE_TELIT_SRC = $(PLUG_LTE_SRC)/plug_lte_telit
PLUG_NR_5G_TELIT_SRC = $(PLUG_NR_5G_SRC)/plug_NR_5G_telit
PLUG_SERIAL_SRC = ../plug_serial

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(PROJ_DIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(EEPROM_SRC) \
	-I$(RTC_SRC) \
	-I$(DEV_MAX31730) \
	-I$(LIB_TAM_ACT2_SRC) \
	-I$(COMSRCDIR) \
	-I$(GE_PHY_SRC) \
	-I$(POE_TPS2386X_SRC) \
	-I$(PCA9555_SRC) \
	-I$(PCA9557_SRC) \
	-I$(TMPX75_SRC) \
	-I$(NXP_LM75B_SRC) \
	-I$(LTC_SRC) \
	-I$(RBCPDIR) \
	-I$(MRV_88E6176_SRC) \
	-I$(MRV_88E6390_SRC) \
	-I$(PLUG_COMMON_SRC) \
	-I$(PLUG_TC_SRC) \
	-I$(PLUG_LTE_SRC) \
	-I$(PLUG_LTE_TELIT_SRC) \
	-I$(PLUG_SERIAL_SRC) \
	-I$(TELIT_LTE_SRC) \
	-I$(PLUG_NR_5G_TELIT_SRC) \
	-I$(NR_5G_TELIT_SRC) \

#
# defines for compiler
#

DEFS = \
	-DTSN \
	-DENHANCE_ERROR_MSG_RDY \
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
# Build ACT2 lib into project
#
#
LIB_TAM_ACT2_SRC = /auto/sp-engops/diags/pld/act2lite/ARM/TSN/SUDI_2099_Aikido
LIB_TAM_ACT2 = $(LIB_TAM_ACT2_SRC)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(LIB_TAM_ACT2_SRC)/tam_lib_manufacturing_diags.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= bglnx

#
# Common device driver objects
#
DEV_OBJS = \
	dev_object.o \
	dev_1337.o \
	dev_maxim_max31730.o \
	dev_mrvl_ge.o \
	dev_tps2386b.o \
	dev_pca9555.o \
	dev_pca9557.o \
	dev_tmpx75.o \
	dev_nxp_lm75b.o \
	dev_88e6176.o \
	dev_88e6390.o \
	dev_lte_telit.o \
	dev_lte_telit_at.o \
	dev_NR_5G_telit.o \
	dev_NR_5G_telit_at.o \

DEV_BASE = \
	$(DEV_DRV)/dev_object.c \
	$(RTC_SRC)/dev_1337.c \
	$(DEV_MAX31730)/dev_maxim_max31730.c \
	$(GE_PHY_SRC)/dev_mrvl_ge.c \
	$(POE_TPS2386X_SRC)/dev_tps2386b.c \
	$(PCA9555_SRC)/dev_pca9555.c \
	$(PCA9557_SRC)/dev_pca9557.c \
	$(TMPX75_SRC)/dev_tmpx75.c \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
	$(MRV_88E6176_SRC)/dev_88e6176.c \
	$(MRV_88E6390_SRC)/dev_88e6390.c \
	$(TELIT_LTE_SRC)/dev_lte_telit.c \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.c\
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit.c \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.c \


DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(RTC_SRC)/dev_1337.h \
	$(DEV_MAX31730)/dev_maxim_max31730.h \
	$(GE_PHY_SRC)/dev_mrvl_ge.h \
	$(POE_TPS2386X_SRC)/dev_tps2386b.h \
	$(PCA9555_SRC)/dev_pca9555.h \
	$(PCA9557_SRC)/dev_pca9557.h \
	$(TMPX75_SRC)/dev_tmpx75.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(MRV_88E6176_SRC)/dev_88e6176.h \
	$(MRV_88E6390_SRC)/dev_88e6390.h \
	$(TELIT_LTE_SRC)/dev_lte_telit.h \
	$(TELIT_LTE_SRC)/dev_lte_telit_at.h\
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit.h \
	$(NR_5G_TELIT_SRC)/dev_NR_5G_telit_at.h \


#
# Pluggable Common Files
#
PLUG_COMMON_OBJS = \
    plug_common_host.o \
    plug_common_lib.o \
    plug_gpio_exp_test.o \
    plug_gpio_exp_lib.o \
    plug_slot.o \
    plug_temp_sensor_test.o \
    plug_temp_sensor_lib.o \
    plug_host_slot_modules.o \
    plug_common_host_impl.o \

PLUG_COMMON_BASE = \
    $(PLUG_COMMON_SRC)/plug_common_host.c \
    $(PLUG_COMMON_SRC)/plug_common_lib.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.c \
    $(PLUG_COMMON_SRC)/plug_slot.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.c \
    $(PROJ_DIR)/plug_host_slot_modules.c \
    $(PROJ_DIR)/plug_common_host_impl.c \

PLUG_COMMON_HFILES = \
    $(PLUG_COMMON_SRC)/plug_common_host.h \
    $(PLUG_COMMON_SRC)/plug_common_lib.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.h \
    $(PLUG_COMMON_SRC)/plug_slot.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.h \
    $(PROJ_DIR)/plug_host_slot_modules.h \
    $(PROJ_DIR)/plug_common_host_impl.h \

#
# Pluggable Testcard
#
PLUG_TC_OBJS = \
    plug_testcard_gpio_exp_lib.o \
    plug_testcard_host.o \
    plug_testcard_phy.o \
    plug_testcard_test.o \
    plug_testcard_usb_lib.o \
    plug_testcard_util.o \
    plug_testcard_host_impl.o \

PLUG_TC_BASE = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.c \
    $(PLUG_TC_SRC)/plug_testcard_host.c \
    $(PLUG_TC_SRC)/plug_testcard_phy.c \
    $(PLUG_TC_SRC)/plug_testcard_test.c \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.c \
    $(PLUG_TC_SRC)/plug_testcard_util.c \
    $(PROJ_DIR)/plug_testcard_host_impl.c \

PLUG_TC_HFILES = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_host.h \
    $(PLUG_TC_SRC)/plug_testcard_phy.h \
    $(PLUG_TC_SRC)/plug_testcard_test.h \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_util.h \
    $(PROJ_DIR)/plug_testcard_host_impl.h \

#
# Pluggable LTE
# 
PLUG_LTE_OBJS = \
    plug_lte_at.o \
    plug_lte_host.o \
    plug_lte_lib.o \
    plug_lte_test.o \
    plug_lte_util.o \
    plug_lte_host_impl.o \

PLUG_LTE_BASE = \
    $(PLUG_LTE_SRC)/plug_lte_at.c \
    $(PLUG_LTE_SRC)/plug_lte_host.c \
    $(PLUG_LTE_SRC)/plug_lte_lib.c \
    $(PLUG_LTE_SRC)/plug_lte_test.c \
    $(PLUG_LTE_SRC)/plug_lte_util.c \
    $(PROJ_DIR)/plug_lte_host_impl.c \

PLUG_LTE_HFILES = \
    $(PLUG_LTE_SRC)/plug_lte_at.h \
    $(PLUG_LTE_SRC)/plug_lte_host.h \
    $(PLUG_LTE_SRC)/plug_lte_lib.h \
    $(PLUG_LTE_SRC)/plug_lte_test.h \
    $(PLUG_LTE_SRC)/plug_lte_util.h \
    $(PROJ_DIR)/plug_lte_host_impl.h \

#
# Pluggable LTE Telit 
#
PLUG_LTE_TELIT_OBJS = \
	plug_lte_telit_host.o \
	plug_lte_telit_lib.o \
	plug_lte_telit_test.o \
	plug_lte_telit_util.o \

PLUG_LTE_TELIT_BASE = \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.c \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.c \

PLUG_LTE_TELIT_HFILES = \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_host.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_lib.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_test.h \
	$(PLUG_LTE_TELIT_SRC)/plug_lte_telit_util.h \

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
	$(PROJ_DIR)/plug_serial_host_impl.c

PLUG_SERIAL_HFILES = \
    $(PLUG_SERIAL_SRC)/platform_plug_serial_test.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_util.h \
	$(PLUG_SERIAL_SRC)/platform_plug_serial_host.h \
	$(PROJ_DIR)/plug_serial_host_impl.h

PLUG_OBJS = \
    diag_sirius_fpga_lib.o \
	diag_sirius_fpga_util.o \
	plug_host_fpga_lib.o \
	$(PLUG_COMMON_OBJS) \
	$(PLUG_TC_OBJS) \
	$(PLUG_LTE_OBJS) \
	$(PLUG_LTE_TELIT_OBJS) \
	$(PLUG_SERIAL_OBJS) \
	$(PLUG_NR_5G_TELIT_OBJS) \

PLUG_BASE = \
    $(PROJ_DIR)/diag_sirius_fpga_lib.c \
	$(PROJ_DIR)/diag_sirius_fpga_util.c \
	$(PROJ_DIR)/plug_host_fpga_lib.c \
	$(PLUG_COMMON_BASE) \
	$(PLUG_TC_BASE) \
	$(PLUG_LTE_BASE) \
	$(PLUG_LTE_TELIT_BASE) \
	$(PLUG_SERIAL_BASE) \
	$(PLUG_NR_5G_TELIT_BASE) \

PLUG_HFILES = \
    $(PROJ_DIR)/diag_sirius_fpga_lib.h \
	$(PROJ_DIR)/diag_sirius_fpga_util.h \
	$(PROJ_DIR)/plug_host_fpga_lib.h \
	$(PLUG_COMMON_HFILES) \
	$(PLUG_TC_HFILES) \
	$(PLUG_LTE_HFILES) \
	$(PLUG_LTE_TELIT_HFILES) \
	$(PLUG_NR_5G_TELIT_HFILES) \
	$(PLUG_SERIAL_HFILES) \

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
	diag_i2c_lib.o \
	diag_i2c_test.o \
	diag_i2c_util.o \
	platform_i2c.o \
	diag_usb_lib.o \
	diag_usb_test.o \
	diag_spi_test.o \
	diag_rtc_lib.o \
	diag_rtc_util.o \
	diag_rtc_test.o \
	diag_temp_sensor_lib.o \
	diag_temp_sensor_util.o \
	diag_temp_sensor_test.o \
	linux_popen.o \
    diag_mother_board_test.o \
    diag_uart_lib.o \
    platform_stub.o \
    diag_dsl_lib.o \
    diag_dsl_test.o \
    diag_dsl_util.o \
    linux_ntwk.o \
    diag_poe_psu_lib.o \
    diag_poe_psu_util.o \
    diag_poe_psu_test.o \
    diag_moka_fpga_lib.o \
    diag_moka_fpga_test.o \
    diag_moka_fpga_util.o \
    diag_aikido_fpga_lib.o \
    diag_aikido_fpga_test.o \
    diag_aikido_fpga_util.o \
    diag_sirius_fpga_test.o \
    platform_cookie.o \
	diag_cpu_lib.o \
	diag_cpu_util.o \
	diag_cpu_test.o \
    tam_act2_api_drv_support.o \
    diag_smi_lib.o \
    diag_ge_phy_test.o \
    diag_ge_phy_lib.o \
    diag_ge_phy_util.o \
    diag_esw_test.o \
    diag_esw_lib.o \
    diag_esw_util.o \
    diag_pkt_txrx_lib.o \
	diag_dimm_lib.o \
	diag_dimm_util.o \
    diag_mcu_util.o \
    diag_mcu_lib.o \
	diag_wifi_lib.o \
	diag_wifi_util.o \
	diag_wifi_test.o \
	diag_led_util.o \
	diag_led_test.o \
    tam_aikido_upgrade.o \
    tam_lib_object_manager.o \
	$(PLUG_OBJS) \
	diag_emmc_lib.o \
	diag_emmc_util.o \
	diag_emmc_test.o \
    diag_reset_button_test.o \
    diag_reset_button_lib.o \
    diag_gshdsl_lib.o \
    diag_gshdsl_test.o \
    diag_gshdsl_util.o \
    diag_rbcp_lib.o \
    diag_rbcp_test.o \
    rbcp_lib.o \
    diag_rbcp_util.o \

PLAT_BASE = \
	$(PROJ_DIR)/linux_main.c \
	$(PROJ_DIR)/diag.c \
	$(PROJ_DIR)/diag_i2c_lib.c \
    $(PROJ_DIR)/platform_i2c.c \
    $(PROJ_DIR)/diag_usb_lib.c \
    $(PROJ_DIR)/diag_usb_test.c \
    $(PROJ_DIR)/diag_spi_test.c \
    $(PROJ_DIR)/diag_rtc_lib.c \
    $(PROJ_DIR)/diag_rtc_util.c \
    $(PROJ_DIR)/diag_rtc_test.c \
    $(PROJ_DIR)/diag_temp_sensor_lib.c \
    $(PROJ_DIR)/diag_temp_sensor_util.c \
    $(PROJ_DIR)/diag_temp_sensor_test.c \
    $(COMSRCDIR)/linux_popen.c \
	$(PROJ_DIR)/diag_mother_board_test.c \
	$(PROJ_DIR)/diag_uart_lib.c \
    $(PROJ_DIR)/platform_stub.c \
    $(PROJ_DIR)/diag_dsl_lib.c \
    $(PROJ_DIR)/diag_dsl_test.c \
    $(PROJ_DIR)/diag_dsl_util.c \
    $(COMSRCDIR)/linux_ntwk.c \
    $(PROJ_DIR)/diag_poe_psu_lib.c \
    $(PROJ_DIR)/diag_poe_psu_util.c \
    $(PROJ_DIR)/diag_poe_psu_test.c \
    $(PROJ_DIR)/diag_moka_fpga_lib.c \
    $(PROJ_DIR)/diag_moka_fpga_test.c \
    $(PROJ_DIR)/diag_moka_fpga_util.c \
    $(PROJ_DIR)/diag_aikido_fpga_lib.c \
    $(PROJ_DIR)/diag_aikido_fpga_test.c \
    $(PROJ_DIR)/diag_aikido_fpga_util.c \
    $(PROJ_DIR)/diag_sirius_fpga_test.c \
    $(PROJ_DIR)/tam_act2_api_drv_support.c \
	$(PROJ_DIR)/platform_cookie.c \
	$(PROJ_DIR)/diag_cpu_lib.c \
	$(PROJ_DIR)/diag_cpu_util.c \
	$(PROJ_DIR)/diag_cpu_test.c \
    $(PROJ_DIR)/diag_smi_lib.c \
    $(PROJ_DIR)/diag_ge_phy_test.c \
    $(PROJ_DIR)/diag_ge_phy_lib.c \
    $(PROJ_DIR)/diag_ge_phy_util.c \
    $(PROJ_DIR)/diag_esw_test.c \
    $(PROJ_DIR)/diag_esw_lib.c \
    $(PROJ_DIR)/diag_esw_util.c \
    $(PROJ_DIR)/diag_pkt_txrx_lib.c \
	$(PROJ_DIR)/diag_dimm_lib.c \
	$(PROJ_DIR)/diag_dimm_util.c \
    $(PROJ_DIR)/diag_mcu_util.c \
    $(PROJ_DIR)/diag_mcu_lib.c \
	$(PROJ_DIR)/diag_wifi_lib.c \
	$(PROJ_DIR)/diag_wifi_util.c \
	$(PROJ_DIR)/diag_wifi_test.c \
	$(PROJ_DIR)/diag_led_util.c \
	$(PROJ_DIR)/diag_led_test.c \
	$(PLUG_BASE) \
    $(LIB_TAM_ACT2_SRC)/tam_lib_object_manager.c \
	$(PROJ_DIR)/diag_emmc_lib.c \
	$(PROJ_DIR)/diag_emmc_util.c \
	$(PROJ_DIR)/diag_emmc_test.c \
    $(PROJ_DIR)/diag_reset_button_test.c \
    $(PROJ_DIR)/diag_reset_button_lib.c \
    $(PROJ_DIR)/diag_gshdsl_lib.c \
    $(PROJ_DIR)/diag_gshdsl_test.c \
    $(PROJ_DIR)/diag_gshdsl_util.c \
    $(PROJ_DIR)/diag_rbcp_util.c \
    $(PROJ_DIR)/diag_rbcp_lib.c \
    $(PROJ_DIR)/diag_rbcp_test.c \

PLAT_HFILES = \
    $(PROJ_DIR)/mon_plat_defs.h \
    $(PROJ_DIR)/cookie_plat.h \
    $(PROJ_DIR)/linux_main.h \
    $(PROJ_DIR)/diag_mother_board_test.h \
    $(PROJ_DIR)/ngio.h \
    $(PROJ_DIR)/platform_i2c.h \
    $(PROJ_DIR)/diag_usb_lib.h \
    $(PROJ_DIR)/diag_usb_test.h \
    $(PROJ_DIR)/diag_spi_test.h \
    $(PROJ_DIR)/diag_i2c_lib.h \
    $(PROJ_DIR)/diag_i2c_test.h \
    $(PROJ_DIR)/diag_i2c_util.h \
	$(PROJ_DIR)/pcmap.h \
	$(PROJ_DIR)/nvmonvars.h \
	$(PROJ_DIR)/object.h \
	$(PROJ_DIR)/diag_rtc_lib.h \
	$(PROJ_DIR)/diag_rtc_util.h \
	$(PROJ_DIR)/diag_rtc_test.h \
	$(PROJ_DIR)/diag_temp_sensor_lib.h \
	$(PROJ_DIR)/diag_temp_sensor_util.h \
	$(PROJ_DIR)/diag_temp_sensor_test.h \
	$(PROJ_DIR)/diag_uart_lib.h \
	$(PROJ_DIR)/platform_stub.h \
    $(PROJ_DIR)/diag_dsl_lib.h \
    $(PROJ_DIR)/diag_dsl_test.h \
    $(PROJ_DIR)/diag_dsl_util.h \
    $(INCDIR)/linux_ntwk.h \
    $(PROJ_DIR)/diag_poe_psu_lib.h \
    $(PROJ_DIR)/diag_poe_psu_util.h \
    $(PROJ_DIR)/diag_poe_psu_test.h \
    $(PROJ_DIR)/act2_utils.h \
	$(PROJ_DIR)/act2l_typedef.h \
	$(PROJ_DIR)/platform_cookie.h \
	$(PROJ_DIR)/tam_act2_api_drv_support.h \
    $(PROJ_DIR)/diag_smi_lib.h \
    $(PROJ_DIR)/diag_esw_test.h \
    $(PROJ_DIR)/diag_esw_lib.h \
    $(PROJ_DIR)/diag_esw_util.h \
    $(PROJ_DIR)/diag_ge_phy_test.h \
    $(PROJ_DIR)/diag_ge_phy_lib.h \
    $(PROJ_DIR)/diag_ge_phy_util.h \
    $(PROJ_DIR)/diag_moka_fpga_lib.h \
    $(PROJ_DIR)/diag_moka_fpga_test.h \
    $(PROJ_DIR)/diag_moka_fpga_util.h \
    $(PROJ_DIR)/diag_aikido_fpga_lib.h \
    $(PROJ_DIR)/diag_aikido_fpga_test.h \
    $(PROJ_DIR)/diag_aikido_fpga_util.h \
    $(PROJ_DIR)/diag_sirius_fpga_test.h \
    $(PROJ_DIR)/diag_pkt_txrx_lib.h \
	$(PROJ_DIR)/diag_dimm_lib.h \
	$(PROJ_DIR)/diag_dimm_util.h \
	$(PROJ_DIR)/diag_mcu_util.h \
	$(PROJ_DIR)/diag_mcu_lib.h \
	$(PROJ_DIR)/diag_wifi_lib.h \
	$(PROJ_DIR)/diag_wifi_util.h \
	$(PROJ_DIR)/diag_wifi_test.h \
	$(PROJ_DIR)/diag_cpu_lib.h \
	$(PROJ_DIR)/diag_cpu_util.h \
	$(PROJ_DIR)/diag_cpu_test.h \
	$(PROJ_DIR)/diag_enhance_err_msg_lib.h \
	$(PLUG_HFILES) \
	$(PROJ_DIR)/diag_led_util.h \
	$(PROJ_DIR)/diag_led_test.h \
	$(PROJ_DIR)/diag_emmc_lib.h \
	$(PROJ_DIR)/diag_emmc_util.h \
	$(PROJ_DIR)/diag_emmc_test.h \
    $(PROJ_DIR)/diag_reset_button_test.h \
    $(PROJ_DIR)/diag_reset_button_lib.h \
    $(PROJ_DIR)/diag_gshdsl_lib.h \
    $(PROJ_DIR)/diag_gshdsl_test.h \
    $(PROJ_DIR)/diag_gshdsl_util.h \
    $(RBCPDIR)/rbcp_lib.h \
    $(PROJ_DIR)/rbcp_platform.h \
    $(PROJ_DIR)/diag_rbcp_lib.h \
    $(PROJ_DIR)/diag_rbcp_test.h \
    $(PROJ_DIR)/diag_rbcp_util.h \

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
	$(POE_TPS2386X_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(PROJ_DIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(DEV_MAX31730): \
	$(LIB_TAM_ACT2_SRC): \
	$(RBCPDIR): \
	$(LIB_TAM_ACT2_SRC): \
	$(MRV_88E6176_SRC): \
	$(MRV_88E6390_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(PLUG_SERIAL_SRC): \
	$(TELIT_LTE_SRC): \
	$(PLUG_NR_5G_TELIT_SRC): \
	$(NR_5G_TELIT_SRC): \

vpath %.h \
	$(GE_PHY_SRC): \
	$(POE_TPS2386X_SRC): \
	$(PCA9555_SRC): \
	$(PCA9557_SRC): \
	$(TMPX75_SRC): \
	$(NXP_LM75B_SRC): \
	$(PROJ_DIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(RTC_SRC): \
	$(DEV_MAX31730): \
	$(LIB_TAM_ACT2_SRC): \
	$(RBCPDIR): \
	$(MRV_88E6176_SRC): \
	$(MRV_88E6390_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(PLUG_LTE_TELIT_SRC): \
	$(PLUG_SERIAL_SRC):\
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
	@echo "CROSS_COMPILE: " $(CROSS_COMPILE)
	@echo "IROOT: " $(IROOT)
	@echo "PROJ_DIR: " $(PROJ_DIR)
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
	sh $(IROOT)/utils/banner.sh BETELGEUSE
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
#$Log: Makefile,v $
#Revision 1.7  2021/06/02 02:56:20  alpeng
#merge sears into trunk
#
#Revision 1.6.44.1  2020/09/05 00:12:07  tshanmug
#Adding Sear PIM support
#
#Revision 1.6  2019/06/24 07:21:37  wilbhuan
#Supported Pluggable Serial Module.
#
#Revision 1.5  2019/06/12 01:08:00  wilbhuan
#Re-opened the CDETS:CSCvq01828. Solved the daily build issue.
#
#Revision 1.4  2019/06/10 01:23:58  wilbhuan
#1. Solved issue with CDETS: CSCvq01828.
#2. Summary of CSCvq01828: For daily build purpose, there's an error will occur if execute the command "make depend". In order to fix this issue, we should delete some of nonexistent files in the Makefile.
#
#Revision 1.3  2019/05/14 09:44:09  sherliu2
#Support Hyerloop
#
#Revision 1.2.6.4  2019/03/11 13:10:46  shjung
#Modified image name for Hyperloop
#
#Revision 1.2.6.3  2019/02/25 06:19:25  sherliu2
#Support Hyperloop on Betelgeuse
#
#Revision 1.2.6.2  2019/02/23 03:45:43  sherliu2
#Sync up with main trunk
#
#Revision 1.2  2019/01/10 06:36:21  wilbhuan
#The beginning of Betelgeuse application code.
#
# *-------------------------------------------------
