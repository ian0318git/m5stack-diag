# $Id: Makefile,v 1.8 2020/05/22 02:28:50 qingcwan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build CSX-Tachi BMC Diagmon
#
# June 2015, Times Huang
#
# Copyright (c) 2015-2018 by Cisco Systems, Inc.
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

#
# Crash us if /bin/make is used:
#
SERVER:=$(HOSTNAME)

ifeq ($(origin USER),undefined)
USER=Bozo
endif

default: make_all

TOOLS_VER	= c4.7.0-p1
TOOLS_ARCH	= x86_64

CROSS_COMPILE = arm-unknown-linux-gnu-

export CROSS_COMPILE

#
# Version 
#
BMC_VERSION_FILE    = version
BMC_MAJOR_NUM = $$(grep MAJOR_VERSION $(BMC_VERSION_FILE) | cut -d= -f2)
BMC_MINOR_NUM = $$(grep MINOR_VERSION $(BMC_VERSION_FILE) | cut -d= -f2)
BMC_RELEASE_NUM = $$(grep RELEASE $(BMC_VERSION_FILE) | cut -d= -f2)
NETTARGET_VER = $(NETTARGET).V$(BMC_MAJOR_NUM).$(BMC_MINOR_NUM).$(BMC_RELEASE_NUM)
RELEASE_NAME  = $(NETTARGET).tar.gz.V$(BMC_MAJOR_NUM).$(BMC_MINOR_NUM).$(BMC_RELEASE_NUM)

NIM_INTEL_IMG       = tachi_nimtest
NIM_INTEL_IMG_VER = $(NIM_INTEL_IMG).V$(BMC_MAJOR_NUM).$(BMC_MINOR_NUM).$(BMC_RELEASE_NUM)

#
# Tool Chain
#

#CC		= gcc.$(TOOLS_VER).$(TOOLS_ARCH)
CC		= $(CROSS_COMPILE)gcc
AS		= as
AR		= ar.$(TOOLS_VER).$(TOOLS_ARCH)
LD		= ld
MAKE		= make 
NM		= nm.$(TOOLS_VER).$(TOOLS_ARCH)
OBJCOPY		= objcopy.$(TOOLS_VER).$(TOOLS_ARCH)
OBJDUMP		= objdump.$(TOOLS_VER).$(TOOLS_ARCH)
SIZE		= size.$(TOOLS_VER).$(TOOLS_ARCH)
#STRIP		= strip.$(TOOLS_VER).$(TOOLS_ARCH)
STRIP		= $(CROSS_COMPILE)strip

.PHONY: all clean clobber depend force-tools install

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include

#
# The platform specific source directory
#

SRCDIR		= .
TACHIDIR     = ..
KLMDIR		= $(IROOT)/klm
#
# The processor specific source directory
#

PROCSRCDIR	= $(IROOT)/x86/src
PROCINCDIR	= $(IROOT)/x86/include

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim

KERNDIR		= $(COMSRCDIR)/kernel
UTIL_DIR	= $(IROOT)/utils
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client


#
# The NIM directories
#
TESTCARDDIR     = .
INTEL_PCIE_DIR     = ./intel

#
# Path to fpga images
#

# ACT2 Library
ACT2_BASE_DIR = /auto/sp-engops/diags/pld/act2lite/ARM/Tachi
ACT2_LIB_OBJS = $(ACT2_BASE_DIR)/tam_lib_manufacturing_Tachi.a \
			$(ACT2_BASE_DIR)/tam_library_w_standalone_Tachi.a

ACT2_PLAT_OBJS = $(SRCDIR)/diag_tam_api_drv_support.o \
				 $(SRCDIR)/diag_plat_cookie.o\
				 $(SRCDIR)/legacy_smart_cookie.o

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(TACHIDIR) \
	-I$(PROCINCDIR) \
	-I$(KERNDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(RTC_SRC) \
	-I$(ACT2_BASE_DIR) \
	-I$(TESTCARDDIR) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(FRU_DIR) \

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
#

DEFS	= \
	-DTACHI \
	-DLINUX_APP \
	-DDIAGMON \
	-DDRV_EXTENDED_REG_TEST \
	-DVIR_IS_PHY \
	-DL_ENDIAN \
	-DINFORMERS_LNX_APP \
	-DSREC2BIN \
	-DTFTP_CLIENT 

# If BANNER is on, add DISPLAY_ENGINEERING_BANNER to the DEFS.
# `make clean`
# `make BANNER=on` to display the engineering banner
# `make` to ignore the banner
ifeq ($(BANNER),on)
DEFS                += -DDISPLAY_ENGINEERING_BANNER
endif

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror $(INC) ${DEFS}
LDFLAGS     = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS} 
ASFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

FRU_DIR  = $(COMSRCDIR)/tachi_bmc/fru
GOLDSCHLAGER_DIR = $(COMSRCDIR)/goldschlager
FORTITUDE_DIR = $(COMSRCDIR)/fortitude
WALLANDER_DIR = $(COMSRCDIR)/wallander
REVA_DIR      = $(COMSRCDIR)/reva

# test, try to compile library in big-endian by ICC

# CSX-Tachi is 64 bit
LIB_AUTH_DIR = /auto/sp-engops/diags/pld/lib/intel/Linux

# achanta is 32 bit lib, overlord is 64 bit
PLDLIB_INTEL_LINUX_ACHANTA = /auto/sp-engops/diags/pld/lib/intel/Linux

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= tachi_lnx
NETELF		= $(NETTARGET).elf

#
# Use -Qoption,bepostld,-verbose to turn on postld verbose 
# to see whether postld do the necessary big-endian global pointers swap
# POSTLDVERBOSE	= -Qoption,bepostld,-verbose
#


#
# Lib sources
#


CRYPTO_OBJS = \
	$(LIB_AUTH_DIR)/crypto_ecdsa.a \
	$(LIB_AUTH_DIR)/crypto_utilities.a 

  
#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

DEV_OBJS = \
	dev_object.o \
	dev_1337.o \
	dev_ltc4215.o \

DEV_BASE = \
	$(DEV_DRV)/dev_object.c \
	$(RTC_SRC)/dev_1337.c \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \

DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(RTC_SRC)/dev_1337.h \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \

#
# Common object files
#

COM_OBJS = \
	linux_main.o \
	linux_pcie.o \
	linux_error.o \
	linux_pstat.o \
	linux_stub.o \
	linux_mem.o \
	linux_api.o \
	linux_memory_tester.o \
	linux_memory_tester_utils.o \
	common_utils.o \
	menu.o \
	print_hex.o \
	alias.o \
	builtin.o \
	query.o \
	memops.o \
	testmem.o \
	parsetoken.o \
	bit_swaps.o \
	pca.o \
	uio_utils.o \
	tam_act2_utils.o \
	linux_uart.o \
	oir_ltc4215_api.o \
	tftpclient.o \
	srec2bin.o \
	adapter_fpga.o

COM_BASE = \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/linux_memory_tester.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/print_hex.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/linux_pstat.c \
	$(COMSRCDIR)/uio_utils.c \
	$(COMSRCDIR)/pca.c \
	$(COMSRCDIR)/linux_uart.c \
	$(COMSRCDIR)/oir_ltc4215_api.c \
	$(COMSRCDIR)/tftpclient.c \
	$(COMSRCDIR)/adapter_fpga.c

COM_HFILES = \
	$(INCDIR)/aglobal.h \
	$(INCDIR)/atypes.h \
	$(INCDIR)/bexterr.h \
	$(INCDIR)/bitdef.h \
	$(INCDIR)/bsafe.h \
	$(INCDIR)/bsfmacro.h \
	$(INCDIR)/bsfplatf.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/common.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/console.h \
	$(INCDIR)/cookie_4.h \
	$(INCDIR)/cpu.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/crypto_credential.h \
	$(INCDIR)/crypto_ecdsa.h \
	$(INCDIR)/defs.h \
	$(INCDIR)/dev_print.h \
	$(INCDIR)/dnld.h \
	$(INCDIR)/duart16552.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/error.h \
	$(INCDIR)/ethernet.h \
	$(INCDIR)/extern.h \
	$(INCDIR)/flash.h \
	$(INCDIR)/free.h \
	$(INCDIR)/hwic_slot.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/ich9_i2c_regs.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_eth.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/menu.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/new_proto.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/oir_ltc4215_api.h \
	$(INCDIR)/pca9545a.h \
	$(INCDIR)/pca.h \
	$(INCDIR)/pci.h \
	$(INCDIR)/pm_utils.h \
	$(INCDIR)/proto.h \
	$(INCDIR)/queryflags.h \
	$(INCDIR)/reason.h \
	$(INCDIR)/resizeob.h \
	$(INCDIR)/router_if.h \
	$(INCDIR)/s2681.h \
	$(INCDIR)/sgmii_defs.h \
	$(INCDIR)/signals.h \
	$(INCDIR)/smart_cookie_auth.h \
	$(INCDIR)/sm_patriot.h \
	$(INCDIR)/spi_cookie.h \
	$(INCDIR)/strings.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/sh.h \
	$(INCDIR)/slot.h \
	$(INCDIR)/smart_cookie.h \
	$(INCDIR)/sm_slot.h \
	$(INCDIR)/stdlibrf.h \
	$(INCDIR)/t1e1_common.h \
	$(INCDIR)/term.h \
	$(INCDIR)/testmem.h \
	$(INCDIR)/types.h \
	$(INCDIR)/uio_utils.h \
	$(INCDIR)/oir_ltc4215_api.h \
	$(INCDIR)/adapter_fpga.h

#
# FRU object files
#
FRU_OBJS = \
		bmc_main.o	\
		diag_arch.o	\
		diag_dev.o	\
		diag_lib.o	\
		diag_glob.o	\
		diag_reg.o	\
		diag_cmd.o	\
		diag_err.o	\
		platform.o	\
		platform_dev.o	\
		platform_fru_i2c.o	\
		platform_gpio.o \
		platform_sprom_acc.o \
		diag_sock.o 	\
		diag_serv.o 	\
                ipmi_sprom_tachi.o	\
		ipmi_sprom_set.o	\
		ipmi_sprom_verify.o	\
		ipmi_sprom_cmds.o	\
		ipmi_sprom_pfm_api.o	\
		ipmi_sprom_show.o	\
		ipmi_sprom_api.o	\
		ipmi_sprom_dflt.o	\
		ipmi_sprom_ops.o	\

FRU_BASE = \
          $(FRU_DIR)/bmc_main.c \
          $(FRU_DIR)/diag_arch.c \
          $(FRU_DIR)/diag_dev.c \
          $(FRU_DIR)/diag_lib.c \
          $(FRU_DIR)/diag_glob.c \
          $(FRU_DIR)/diag_reg.c \
          $(FRU_DIR)/diag_cmd.c \
          $(FRU_DIR)/diag_err.c \
          $(FRU_DIR)/platform.c \
          $(FRU_DIR)/platform_dev.c \
          $(FRU_DIR)/platform_fru_i2c.c \
          $(FRU_DIR)/platform_gpio.c \
          $(FRU_DIR)/platform_sprom_acc.c \
          $(FRU_DIR)/diag_sock.c \
          $(FRU_DIR)/diag_serv.c \
          $(FRU_DIR)/ipmi_sprom_tachi.c \
          $(FRU_DIR)/ipmi_sprom_set.c \
          $(FRU_DIR)/ipmi_sprom_verify.c\
          $(FRU_DIR)/ipmi_sprom_pfm_api.c \
          $(FRU_DIR)/ipmi_sprom_show.c \
          $(FRU_DIR)/ipmi_sprom_api.c \
          $(FRU_DIR)/ipmi_sprom_dflt.c \
          $(FRU_DIR)/ipmi_sprom_ops.c \

 FRU_HFILES = \
             $(FRU_DIR)/diag_cmn.h \
             $(FRU_DIR)/diag_dev.h \
             $(FRU_DIR)/diag_err.h \
             $(FRU_DIR)/diag_glob.h \
             $(FRU_DIR)/diag_main.h \
             $(FRU_DIR)/diag_ops.h \
             $(FRU_DIR)/diag_reg.h \
             $(FRU_DIR)/diag_sock.h \
             $(FRU_DIR)/diag_sys.h \
             $(FRU_DIR)/ipmi_sprom.h \
             $(FRU_DIR)/ipmi_sprom_ops.h \
             $(FRU_DIR)/gpio_ioctl.h \
             $(FRU_DIR)/i2c.h \

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#	platform_i2c_usb.o \
	platform_i2c.o \
	platform_30w_poe.o \
	dash_fpga.o \
	dash_reg_test.o \
	dash_get_addr.o \
	dash_mem.o \
	platform_pll.o \
	platform_pcie_sw.o \
	platform_plx_pcie_sw.o \
	platform_pem_utils.o \
	platform_pem_monitor_read.o \
	platform_pem_eeprom_modify.o \
	platform_pem_fan.o \
	platform_idprom_utils.o \
	act2_api_drv_support.o \
	act2_utils.o \
	platform_30w_poe.o \
	eth_pkt_utils.o 
#

#
# Test Card related files
#
#
#### for tachi entry p1a-1 : 
#### Skip testcard_pcie, testcard_plx_pcie_sw, testcard_tlk_10232 ####
#### pcie related test will use nc command on intel
#### eth port is not ready on cetus, cetus will be available on p1a-2
TESTCARD_OBJS = \
	$(TESTCARDDIR)/ngio_testcard.o \
	$(TESTCARDDIR)/testcard_fpga.o \
	$(TESTCARDDIR)/testcard_eth.o \
	$(TESTCARDDIR)/testcard_plx_pcie_sw.o \
	$(TESTCARDDIR)/testcard_tlk_10232.o \
	$(TESTCARDDIR)/testcard_xaui.o \
	$(TESTCARDDIR)/testcard_uart.o

TESTCARD_BASE = \
	$(TESTCARDDIR)/ngio_testcard.c \
	$(TESTCARDDIR)/testcard_fpga.c \
	$(TESTCARDDIR)/testcard_eth.c \
	$(TESTCARDDIR)/testcard_plx_pcie_sw.c \
	$(TESTCARDDIR)/testcard_tlk_10232.c \
	$(TESTCARDDIR)/testcard_xaui.o \
	$(TESTCARDDIR)/testcard_uart.c

TESTCARD_HFILES = \
	$(TESTCARDDIR)/ngio_testcard.h \
	$(TESTCARDDIR)/testcard_fpga.h \
	$(TESTCARDDIR)/testcard_eth.h \
	$(TESTCARDDIR)/testcard_plx_pcie_sw.h \
	$(TESTCARDDIR)/testcard_xaui.h \
	$(TESTCARDDIR)/testcard_tlk_10232.h 

F35_OBJS = nim_f35.o

F35_BASE = $(COMSRCDIR)/nim_f35.c

F2W_OBJS = nim_f2w.o
F2W_BASE = $(COMSRCDIR)/nim_f2w.c

ARKENSTONE_OBJS = nim_arkenstone.o
ARKENSTONE_BASE = $(COMSRCDIR)/nim_arkenstone.c

DYNAMO_OBJS = nim_dynamo.o
DYNAMO_BASE = $(COMSRCDIR)/nim_dynamo.c

GOLDSCHLAGER_OBJS = \
	ngwic_goldschlager.o \
	ngwic_goldschlager_comm.o \
	bcm63268_lib.o \

GOLDSCHLAGER_BASE = \
	$(GOLDSCHLAGER_DIR)/ngwic_goldschlager.c \
	$(GOLDSCHLAGER_DIR)/ngwic_goldschlager_comm.c \
	$(GOLDSCHLAGER_DIR)/bcm63268_lib.c \

FORTITUDE_OBJS = ngwic_fortitude.o
FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c

WALLANDER_OBJS = nim_wallander.o
WALLANDER_BASE = $(COMSRCDIR)/nim_wallander.c

REVA_OBJS = ngwic_reva.o
REVA_BASE = $(COMSRCDIR)/ngwic_reva.c

PLAT_OBJS = \
	mb_tests.o \
	intel_tests.o \
	daughtercard_test.o \
	diag.o \
	env_mcu_firmware.o \
	diag_blk_test.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	diag_peci_lib.o \
	diag_peci_test.o \
	diag_power_lib.o \
	diag_fpga_lib.o \
	diag_fpga_uart.o \
	diag_fpga_i2c.o \
	diag_fpga_test.o \
	diag_lewis_gesw_test.o \
	diag_lewis_gesw_util.o \
	diag_lewis_gesw_phy_util.o \
	diag_lewis_gesw_poe_util.o \
	diag_fpga_util.o \
	diag_fpga_prog.o \
	diag_barometer_util.o \
	diag_barometer_lib.o \
	diag_raid_test.o \
	diag_testcard_test.o \
	diag_raid_util.o \
	diag_raid_lib.o \
	diag_raid_fw.o \
	diag_smi_lib.o \
	diag_console_util.o \
	diag_mcu_test.o \
	diag_mcu_util.o \
	diag_mcu_lib.o \
	diag_temp_sensor_lib.o \
	diag_gephy_test.o \
	diag_gephy_util.o \
	diag_geswitch_test.o \
	diag_geswitch_util.o \
	diag_temp_sensor_test.o \
	diag_temp_sensor_util.o \
	diag_fan_test.o \
	diag_rtc_test.o \
	diag_aux_test.o \
	diag_i2c_util.o \
	diag_fan_util.o \
	diag_margin_util.o \
	diag_led_util.o \
	diag_idt_fw.o \
	diag_eth_pkt_txrx.o \
	diag_eth_pkt_txrx_api.o \
	diag_eth_pkt_txrx_utils.o \
	diag_nc_client.o \
	diag_nc_client_lib.o \
	diag_nc_common.o \
	diag_nc_server.o \
	diag_nc_server_lib.o \
	diag_pem_util.o \
	diag_pem_monitor_rw.o \
	diag_pem_fan.o \
	diag_pem_lib.o \
	diag_idprom_util.o \
	ngio.o \
	slot.o \
	diag_i2c_api.o \
	platform_i2c.o \
	platform_stub.o \
	platform_slot.o \
	platform_slot_test.o \
	cookie_4_core.o \
	platform_cookie.o \
	linux_ntwk.o \
	cetus_gesw_api.o \
	cetus_gesw_tests.o \
	diag_fru_util.o \

PLAT_BASE = \
	linux_main.c \
	diag.c \
	mb_tests.c \
	intel_tests.c \
	daughtercard_test.c \
	platform_stub.c \
	platform_slot.c \
	platform_slot_test.c \
	platform_cookie.c \
	linux_ntwk.c \
	cetus_gesw_api.c \
	cetus_gesw_tests.c \

PLAT_HFILES = \
	mb_tests.h \
	intel_tests.h \
	daughtercard_test.h \
	mon_plat_defs.h \
	nvmonvars.h \
	pcmap.h \
	dash_fpga.h \
	platform_stub.h \
	platform_slot.h \
	platform_slot_test.h \
	plat_defs.h \
	diag_blk_test.h \
	diag_i2c_test.h \
	diag_i2c_lib.h \
	diag_peci_test.h \
	diag_peci_lib.h \
	diag_power_lib.h \
	diag_fpga_lib.h \
	diag_fpga_uart.h \
	diag_i2c_lib.h \
	diag_fpga_test.h \
	diag_lewis_gesw_test.h \
	diag_fpga_util.h \
	diag_fpga_prog.h \
	diag_smi_lib.h \
	diag_console_util.h \
	diag_mcu_test.h \
	diag_mcu_util.h \
	diag_mcu_lib.h \
	diag_temp_sensor_lib.o \
	diag_barometer_util.h \
	diag_barometer_lib.h \
	diag_raid_test.h \
	diag_testcard_test.h \
	diag_raid_util.h \
	diag_raid_lib.h \
	diag_gephy_test.h \
	diag_gephy_util.h \
	diag_geswitch_test.h \
	diag_geswitch_util.h \
	diag_temp_sensor_test.h \
	diag_temp_sensor_util.h \
	diag_i2c_util.h \
	diag_fan_util.h \
	diag_margin_util.h \
	diag_led_util.h \
	diag_fan_test.h \
	diag_rtc_test.h \
	diag_aux_test.h \
	diag_i2c_api.h \
	diag_pem_lib.h \
	diag_pem_fan.h \
	diag_idprom.h \
	diag_idprom_util.h \
	platform_i2c.h \
	cookie_plat.h \
	ngio.h \
	act2_utils.h \
	diag_tam_api_drv_support.h \
	diag_plat_cookie.h \
	legacy_smart_cookie.h \
	diag_eth_info.h \
	diag_eth_pkt_txrx.h \
	diag_eth_pkt_txrx_api.h \
	diag_eth_pkt_txrx_utils.h \
	diag_nc_common.h \
	platform_cookie.h \
	linux_ntwk.h \
	cetus_gesw_defs.h \
	diag_fru_util.h \
	platform_fru.h \

NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(ACT2_PLAT_OBJS) \
	$(ACT2_LIB_OBJS) \
	$(TESTCARD_OBJS) \
	$(F35_OBJS) \
	$(F2W_OBJS) \
	$(FRU_OBJS) \
	$(ARKENSTONE_OBJS) \
	$(GOLDSCHLAGER_OBJS) \
	$(DYNAMO_OBJS) \
	$(FORTITUDE_OBJS) \
	$(WALLANDER_OBJS) \
	$(REVA_OBJS) \

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE) \
	$(TESTCARD_BASE) \
	$(F35_BASE) \
	$(F2W_BASE) \
	$(FRU_BASE) \
	$(ARKENSTONE_BASE) \
	$(GOLDSCHLAGER_BASE) \
	$(DYNAMO_BASE) \
	$(FORTITUDE_BASE) \
	$(WALLANDER_BASE) \
	$(REVA_BASE) \

	

#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
	$(TESTCARD_HFILES) \
	$(FRU_HFILES) \

#
#########   UCODE (MicroCodes Files #########
#

UCODES	= \
	hwic_dsu_v3_fmw

#
#########   ASM Files #########
#

ASM	= \
	$(PLAT_AFILES)

vpath %.c \
	$(SRCDIR): \
	$(TACHIDIR): \
	$(COMSRCDIR): \
	$(PROCSRCDIR): \
	$(DEV_DRV): \
	$(ACT2_BASE_DIR): \
	$(RTC_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(FRU_DIR): \
	$(GOLDSCHLAGER_DIR): 

vpath %.h \
	$(SRCDIR): \
	$(TACHIDIR): \
	$(PROCINCDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(CHIPINC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(FRU_DIR): \
	$(GOLDSCHLAGER_DIR): 

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
# No ROM version for CSX-Tachi
#
# Build BMC then intel pcie app. 

all: $(NETTARGET)
	@$(MAKE) intel_pcie_app

make_all: $(NETTARGET).tar.gz

$(NETTARGET).tar.gz: release
	@if [ -a $(NETTARGET).tar.gz ]; then rm $(NETTARGET).tar.gz; fi;
	@if [ -a $(NETTARGET).tar.gz.symbols ]; then rm $(NETTARGET).tar.gz.symbols; fi;
	@ln -s $(RELEASE_NAME) $(NETTARGET).tar.gz
	@ln -s $(NETTARGET).symbols $(NETTARGET).tar.gz.symbols

release: $(NETTARGET)
	@$(MAKE) intel_pcie_app
	@mv $(NETTARGET) $(NETTARGET_VER)
	@mv $(NIM_INTEL_IMG) $(NIM_INTEL_IMG_VER)
	@tar -czf $(RELEASE_NAME) $(NETTARGET_VER) $(NIM_INTEL_IMG_VER)
	@echo "Generating $(RELEASE_NAME)" 
	@chmod +x $(RELEASE_NAME)
	

#
# do not delete the last line for object dump just in case
# we need to do this when debugging a problem. temp comment out
#
#install: $(SRECFILE) $(NETTARGET) install.sh
#
# temp comment out ROM version due to compiler update problem
#

#install: $(NETTARGET) install.sh
#	sh install.sh
#	touch install.sh
#	$(OBJDUMP) -l --disassemble --source $(NETELF) > $(NETTARGET).dis


clean:
	rm -f *.o  banner.c \
	tools temp.* *.dat .compile ID.temp \
	$(NETTARGET) $(NETTARGET_VER) \
	$(NIM_INTEL_IMG) $(NIM_INTEL_IMG_VER)
	$(MAKE) -C $(INTEL_PCIE_DIR) clean

#clobber: clean
#	rm -f moncode* pfsi syms*

superclean: clobber
	$(MAKE) --no-print-directory -C $(UTIL_DIR) clean
	$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc clean

force-tools:
	rm -f tools

tools:
	@$(MAKE) --no-print-directory -C $(UTIL_DIR) depend $(UTOOLS)
	@$(MAKE) --no-print-directory -C $(IROOT)/sys/obj-4k-c3640 mksum_elf
	@$(MAKE) --no-print-directory -C $(UTIL_DIR)/hxldsrc depend $(HTOOLS)

TARGET_DEPEND = \
	Makefile \
	$(NET_OBJS)

TARGET_OBJS = \
	$(NET_OBJS)

#
# NOTE: $(LIBDIR)/$(LIB) has been pre-built and checked in
#	but we want to rebuild without -fomit-frame-pointer specified.
#	or lib source got modified.
#


$(NETTARGET) : $(TARGET_DEPEND) $(HFILES)
	sh $(IROOT)/utils/banner.sh CSX_TACHI
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) $(LDFLAGS) 
	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols
	touch dependancies
	touch $(NETTARGET).symbols

intel_pcie_app:
	@$(MAKE) -C $(INTEL_PCIE_DIR)

#
# The include file dependencies
#

DEPEND_LST = $(CBASE) $(HFILES) $(ASM)

depend: dependancies $(DEPEND_LST)
	@echo "Computing dependancies for common/src"
	@$(CC) $(CPPOPTS) -M $(CBASE) $(ASM) > dependancies
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

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.8  2020/05/22 02:28:50  qingcwan
# Merge switzer-carrier code into main chunk.
#
# Revision 1.7  2018/08/06 02:37:59  harrchan
# Changing daily build server from sjc-ads-1686 to sjc-ads-9106 (CSCvk60118)
#
# Revision 1.6  2018/06/12 01:41:48  haohsu
# Add REVA NIM for TACHI platform
#
# Revision 1.5  2017/03/30 08:30:53  hondwang
# Tachi-L brach merge
#
# Revision 1.4.10.5  2017/02/24 06:25:43  hondwang
# Fix Sam replace files miss wallander and fortitude
#
# Revision 1.4.10.4  2017/02/20 08:11:12  haohsu
# Add NIM Dynamo to Tachil-l
#
# Revision 1.4.10.3  2017/01/09 12:17:29  hondwang
# Add Wallander support
#
# Revision 1.4.10.2  2016/12/27 10:28:36  hondwang
# Add Fortitude support
#
# Revision 1.4.10.1  2016/12/22 01:03:56  haohsu
# Add NIM to Tachi-l
#
# Revision 1.4  2016/08/09 02:45:49  jimmyya
# Add srec2bin for MCU updating utility
#
# Revision 1.3  2016/06/04 09:22:20  alpeng
# initial check in for f2w
#
# Revision 1.2  2016/04/20 11:37:00  benchen2
# merge tachi to main trunk
#
# Revision 1.1.2.37  2016/03/04 09:16:18  benchen2
# add fru util
#
# Revision 1.1.2.36  2016/03/01 02:40:36  alpeng
# update version for supporting daily-build
#
# Revision 1.1.2.35  2016/02/25 09:35:29  benchen2
# fix make depend issue
#
# Revision 1.1.2.34  2016/02/19 02:17:05  benchen2
# change nameing rule
#
# Revision 1.1.2.33  2016/02/05 01:41:24  benchen2
# raid card support SBR eeprom program
#
# Revision 1.1.2.32  2016/01/29 09:15:07  alpeng
# include nim pcie test while complie
#
# Revision 1.1.2.31  2016/01/11 10:28:15  tirawan
# Add Test card menu to run FPGA i2c register test, btb test from x86 and Lewis
#
# Revision 1.1.2.30  2016/01/07 12:35:52  benchen2
# integrate led to main utility
#
# Revision 1.1.2.29  2015/12/30 05:34:45  tirawan
# Add Lewis poe utility & phy utility. On behalf of Jimmy Yang
#
# Revision 1.1.2.28  2015/12/28 06:12:30  hondwang
# Add and modify files for INTEL NC command support
#
# Revision 1.1.2.27  2015/12/23 11:16:13  alpeng
# support PEM(PSU) utility and its fan utils
#
# Revision 1.1.2.26  2015/12/21 22:34:28  jskow
# Rename files from lewis to lewis_gesw for clarity.  Implement external loopback flag for external loopback menu item.  Implement port internal loopback test.
#
# Revision 1.1.2.25  2015/12/17 16:37:02  jskow
# Create Lewis utility menu/file to run some utilities from BMC.  Update Lewis test file functions to allow running utilities and tests on Switch.
#
# Revision 1.1.2.24  2015/12/08 00:47:59  jskow
# Add Lewis test file and header, modify mb_tests to include Lewis in menu
#
# Revision 1.1.2.23  2015/11/24 12:14:31  alpeng
# add nc infrastructure
#
# Revision 1.1.2.22  2015/11/13 07:57:29  tirawan
# Add Voltage and Frequency Margin
#
# Revision 1.1.2.21  2015/11/13 07:34:31  benchen2
# add raid card test and utility
#
# Revision 1.1.2.20  2015/10/12 08:22:27  benchen2
# add raid card cpld upgrade
#
# Revision 1.1.2.19  2015/09/26 05:22:35  alpeng
# update nim test entry
#
# Revision 1.1.2.18  2015/09/17 13:04:34  tirawan
# Support Cisco FPGA firmware upgrade
#
# Revision 1.1.2.17  2015/08/31 06:42:08  tirawan
# Ported legacy smart cookie to support Quack chip read as TAM library cookie read function doesn't work on Quack chip
#
# Revision 1.1.2.16  2015/08/30 05:57:35  tirawan
# To support NIM ACT2 R/W access using TAM library
#
# Revision 1.1.2.15  2015/08/21 06:46:28  alpeng
# support ge/xaui test for testcard; clean up repo;
#
# Revision 1.1.2.14  2015/08/19 03:14:59  meho
# Replace space into tab in Makefile.
#
# Revision 1.1.2.13  2015/08/11 07:44:28  meho
# Added f35 nim tests.
#
# Revision 1.1.2.12  2015/08/04 03:32:20  meho
# Added RTC tests.
#
# Revision 1.1.2.11  2015/08/04 02:25:43  hondwang
# add lpbk util, lib, api
#
# Revision 1.1.2.10  2015/08/01 01:37:40  tirawan
# Update FPGA SPI Read/Write function
#
# Revision 1.1.2.9  2015/07/31 10:39:59  alpeng
# first check in for testcard
#
# Revision 1.1.2.8  2015/07/31 08:21:42  hondwang
# add smi, barometer, temp sensor
#
# Revision 1.1.2.7  2015/07/28 09:32:38  alpeng
# adding entry for ngio test on platform_slot.c
#
# Revision 1.1.2.6  2015/07/26 06:02:21  tirawan
# Add FPGA I2C read/write function and ACT2 cookie/smartchip programming
#
# Revision 1.1.2.5  2015/07/24 06:59:58  alpeng
# Add ngio.c to support NIM test
#
# Revision 1.1.2.4  2015/07/24 03:39:35  tirawan
# Add FPGA I2C read/write function, ACT2 cookie read/write function
#
# Revision 1.1.2.3  2015/07/12 06:52:45  tirawan
# Add Console Switch Utility, SPI driver and FPGA programming
#
# Revision 1.1.2.2  2015/06/11 02:04:44  tirawan
# Remove unnecessary files
#
# Revision 1.1.2.1  2015/06/11 02:01:06  tirawan
# Add files for Tachi BMC project
#
# 
#
# $Endlog$
