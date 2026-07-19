# $Id: Makefile,v 1.3 2018/09/03 04:23:51 haohsu Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/vg400/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Atreides VG400.
#
# Jan 2016, Sam Hsu port from USD Platform
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

default: all

TOOLS_VER	= c4.7.0-p1
TOOLS_ARCH	= x86_64

#
# Tool Chain
#
CC		= gcc.$(TOOLS_VER).$(TOOLS_ARCH)
AS		= as
AR		= ar.$(TOOLS_VER).$(TOOLS_ARCH)
LD		= ld
MAKE		= make 
NM		= nm.$(TOOLS_VER).$(TOOLS_ARCH)
OBJCOPY		= objcopy.$(TOOLS_VER).$(TOOLS_ARCH)
OBJDUMP		= objdump.$(TOOLS_VER).$(TOOLS_ARCH)
SIZE		= size.$(TOOLS_VER).$(TOOLS_ARCH)
STRIP		= strip.$(TOOLS_VER).$(TOOLS_ARCH)

.PHONY: all clean clobber depend force-tools install

SHELL		= /bin/sh
OPTIONS 	=
IROOT		= ../../..
INCDIR		= $(IROOT)/common/include
FPGA_PATH	= /auto/sp-engops/diags/pld/fpga_image

#
# The platform specific source directory
#

SRCDIR		= .
OVERLORDDIR     = ../overlord/
UTAHDIR         = ../overlord/utah
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

CLK1_SRC	= $(DEV_DRV)/dev_idt_ics844s2416i
DIMM_SRC	= $(DEV_DRV)/dev_dimm
EEPROM_SRC	= $(DEV_DRV)/dev_at24c0n_atmel
ENV_SRC		= $(DEV_DRV)/dev_csco_10698
I2CMX_SRC	= $(DEV_DRV)/dev_pca9545a_ti
KERNDIR		= $(COMSRCDIR)/kernel
M29312DIR	= $(COMSRCDIR)/m29312
RTC_SRC		= $(DEV_DRV)/dev_1337_maxim
SNSR_SRC	= $(DEV_DRV)/dev_max1617a
Socrates 	= $(COMSRCDIR)/Socrates
TDM_PLL_SRC	= $(DEV_DRV)/dev_idt82v32021
USB_SRC		= $(DEV_DRV)/dev_cy7c64215_cypress
UTIL_DIR	= $(IROOT)/utils
CODESIGN_SRC_DIR = $(UTIL_DIR)/code_sign/src
ABRAXAS_CLIENT_DIR = $(UTIL_DIR)/code_sign/abraxas_client
POE30_SRC = $(DEV_DRV)/dev_ilp
PLDOVLD = /nfs/sp-engops/diags/pld/overlord
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/ISR4K_2099_SUDI

#
# Path to fpga images
#
PLDDIR			= /auto/sp-engops/diags/pld
PLDLIB			= $(PLDDIR)/lib

# BCM GE switch related defines
#
BCM_SDK_ROOT = /auto/overlord/ovld-vendor-src/bcmsdk/bcm-gesw-sdk
BCM_SDK_INCDIR = $(BCM_SDK_ROOT)/include
BCM_SYSTEMS_DIR = $(BCM_SDK_ROOT)/systems

#
# The BCM *.a lib files are copied from the build directory of the BCM tree
# to the /auto/sp-engops/diags/pld/overlord/bcm_lib automatically when
# user run the build-for-ovld script in the BCM tree.
# BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
#
BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
BCM_BLD_VERSION_OBJ = $(BCM_LIB_DIR)/version.o
BCM_PLATFORM_DEFINES_OBJ = $(BCM_LIB_DIR)/platform_defines.o

BCM_LDFLAGS =  -lpthread -lm -lrt -Wl,--relax
BCM_DEFS = \
	-DSYS_BE_PIO=0 -DSYS_BE_PACKET=0 -DSYS_BE_OTHER=0 \
	-DBCM_ESW_SUPPORT \
	-DUNIX \
	-DBCM_WARM_BOOT_SUPPORT_SW_DUMP \

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(UTAHDIR) \
	-I$(OVERLORDDIR) \
	-I$(PROCINCDIR) \
	-I$(KERNDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(CLK1_SRC) \
	-I$(CLK2_SRC) \
	-I$(CLKBUF_SRC) \
	-I$(DIMM_SRC) \
	-I$(EEPROM_SRC) \
	-I$(ENV_SRC) \
	-I$(I2CMX_SRC) \
	-I$(SNSR_SRC) \
	-I$(TDM_PLL_SRC) \
	-I$(USB_SRC) \
	-I$(POE30_SRC) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(TAMLIB)

INC	+= -I$(BCM_SDK_INCDIR) -I$(BCM_SYSTEMS_DIR)

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
#

DEFS	= \
	-DOVERLORD \
	-DLINUX_APP \
	-DSREC2BIN \
	-DDIAGMON \
	-DDRV_EXTENDED_REG_TEST \
	-DVIR_IS_PHY \
	-DL_ENDIAN \
	-DINFORMERS_LNX_APP \
	-DTFTP_CLIENT \
	-DGOLDBEACH \
	-DUTAH \
	-DVG400 \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

# If BANNER is on, add DISPLAY_ENGINEERING_BANNER to the DEFS.
# `make clean`
# `make BANNER=on` to display the engineering banner
# `make` to ignore the banner
ifeq ($(BANNER),on)
DEFS                += -DDISPLAY_ENGINEERING_BANNER
endif

DREAMLINER_DEFS = \
	-DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#
CPPOPTS     	= -Wall -Werror $(INC) ${DEFS} ${BCM_DEFS} $(DREAMLINER_DEFS)
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
ASFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

#test, try to compile library in big-endian by ICC
NEW_CLI_OBJS = cli_cmd.o
NEW_CLI_BASE = cli_cmd.c
NEW_CLI_HFILES = cli_cmd.h

# overlord is 64 bit
LIB_AUTH_DIR = /auto/sp-engops/diags/pld/lib/intel/Linux

# achanta is 32 bit lib, overlord is 64 bit
PLDLIB_INTEL_LINUX_ACHANTA = /auto/sp-engops/diags/pld/lib/intel/Linux

LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_ACT2 = $(PLDOVLD)/act2_lib64.a
LIB_PCI_UTILS = /auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

LIB_QUACK = $(PLDLIB_INTEL_LINUX_ACHANTA)/ec_rsa_toolkit.a


# BCM GE Switch LIB
BCM_LIBS = $(wildcard $(BCM_LIB_DIR)/lib*.a)

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= vg400_lnx
NETELF		= $(NETTARGET).elf
NETTARGET_BZ2	= $(NETTARGET).bz2

#
# Use -Qoption,bepostld,-verbose to turn on postld verbose 
# to see whether postld do the necessary big-endian global pointers swap
# POSTLDVERBOSE	= -Qoption,bepostld,-verbose
#
POSTLDVERBOSE	=
RAMLDFLAGS 	=

#
# Lib sources
#

CRYPTO_OBJS = \
	$(LIB_AUTH_DIR)/crypto_ecdsa.a \
	$(LIB_AUTH_DIR)/crypto_utilities.a \
        $(LIB_AUTH_DIR)/smart_cookie_auth.a
  
#
# Common device driver objects
#
DEV_CORE_OBJS = \
	dev_object.o

DEV_OBJS = \
	dev_32021.o \
	dev_idt_ics844s2416i.o \
	dev_ltc4215.o \
	dev_object.o \
	dev_pca9545.o \
	dev_at24c0n.o \
	dev_dimm.o \
	dev_1337.o \
	dev_csco_10698.o \
	dev_ilp.o \
	dev_cy7c64215.o


DEV_CORE_BASE = \
	$(DEV_DRV)/dev_object.c

DEV_BASE = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.c \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
	$(DEV_DRV)/dev_object.c \
	$(TDM_PLL_SRC)/dev_32021.c \
	$(I2CMX_SRC)/dev_pca9545.c \
	$(EEPROM_SRC)/dev_at24c0n.c \
	$(DIMM_SRC)/dev_dimm.c \
	$(ENV_SRC)/dev_csco_10698.c \
	$(POE30_SRC)/dev_ilp.c \
	$(USB_SRC)/dev_cy7c64215.c \


DEV_HFILES = \
	$(CLK1_SRC)/dev_idt_ics844s2416i.h \
	$(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
	$(DEV_DRV)/dev_object.h \
	$(TDM_PLL_SRC)/dev_32021.h \
	$(I2CMX_SRC)/dev_pca9545.h \
	$(EEPROM_SRC)/dev_at24c0n.h \
	$(DIMM_SRC)/dev_dimm.h \
	$(RTC_SRC)/dev_1337.h \
	$(ENV_SRC)/dev_csco_10698.h \
	$(POE30_SRC)/dev_ilp.h \
	$(USB_SRC)/dev_cy7c64215.h

#
# Common object files
#

COM_OBJS = \
	linux_main.o \
	linux_ntwk.o \
	linux_i2c.o \
	linux_pciutils.o \
	linux_error.o \
	linux_stub.o \
	linux_mem.o \
	linux_uart.o \
	linux_pstat.o \
	linux_api.o \
	linux_memory_tester.o \
	linux_memory_tester_utils.o \
	menu.o \
	pca.o \
	common_utils.o \
	cli_cmd.o \
	ngio.o \
	slot.o \
	platform_slot.o \
	print_hex.o \
	shcmd.o \
	alias.o \
	monitor.o \
	parsetoken.o \
	builtin.o \
	rtc.o \
	linux_pcie.o \
	query.o \
	memops.o \
	testmem.o \
	smart_cookie.o \
	crypto_credential.o \
	cookie_4_core.o \
	bit_swaps.o \
	i2c_drv.o \
	platform_intr_test.o \
	uio_utils.o \
	oir_ltc4215_api.o \
	linux_usb_test.o \
	tftpclient.o \
	srec2bin.o \
	tam_act2_utils.o \
	linux_coretest.o \

COM_BASE = \
	$(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_pciutils.c \
	$(COMSRCDIR)/linux_error.c \
	$(COMSRCDIR)/linux_stub.c \
	$(COMSRCDIR)/linux_mem.c \
	$(COMSRCDIR)/linux_uart.c \
	$(COMSRCDIR)/linux_pstat.c \
	$(COMSRCDIR)/linux_api.c \
	$(COMSRCDIR)/linux_memory_tester.c \
	$(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/hwic_stub.c \
	$(COMSRCDIR)/slot.c \
	$(COMSRCDIR)/menu.c \
	$(COMSRCDIR)/common_utils.c \
	$(COMSRCDIR)/cli_cmd.c \
	$(COMSRCDIR)/shcmd.c \
	$(COMSRCDIR)/alias.c \
	$(COMSRCDIR)/monitor.c \
	$(COMSRCDIR)/parsetoken.c \
	$(COMSRCDIR)/print_hex.c \
	$(COMSRCDIR)/builtin.c \
	$(COMSRCDIR)/rtc.c \
	$(COMSRCDIR)/linux_pcie.c \
	$(COMSRCDIR)/query.c \
	$(COMSRCDIR)/memops.c \
	$(COMSRCDIR)/testmem.c \
	$(COMSRCDIR)/smart_cookie.c \
	$(COMSRCDIR)/crypto_credential.c \
	$(COMSRCDIR)/cookie_4_core.c \
	$(COMSRCDIR)/bit_swaps.c \
	$(COMSRCDIR)/oir_ltc4215_api.c \
	$(COMSRCDIR)/uio_utils.c \
	$(COMSRCDIR)/pca.c \
	$(COMSRCDIR)/tftpclient.c \
	$(COMSRCDIR)/tam_act2_utils.c \
	$(COMSRCDIR)/linux_coretest.c \


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
	$(INCDIR)/print_hex.h \
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
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	env_mcu_firmware.o \
	VMON_MCU_01_01.o \
	igb_eeprom.o \
	ich9_i2c.o \
	platform_i2c.o \
	platform_i2c_usb.o \
	i2c_api.o \
	platform_margin_utils.o \
	platform_pwr_seq.o \
	platform_sys_clk.o \
	platform_gh_gesw_clk.o \
	platform_psu.o \
	platform_poe_psu.o \
	platform_pcie_clk.o \
	platform_sensor.o \
	platform_led.o \
	platform_eeprom.o \
	platform_eeprom_access.o \
	platform_prom.o \
	platform_30w_poe.o \
	platform_pll.o \
	platform_pem_utils.o \
	platform_pem_eeprom_modify.o \
	platform_idprom_utils.o \
	tam_act2_api_drv_support.o \
	platform_pcie_sw.o \
	platform_eth.o \
	platform_sfp_ext_lpbk.o \
	platform_ext_lpbk.o \
	platform_macsec.o \
	platform_stub.o \
	platform_fan.o \
	platform_dimm.o \
	mb_tests.o \
	platform_cookie.o \
	platform_barometer.o \
	moncmds.o \
	dash_fpga.o \
	ht.o \
	dash_reg_test.o \
	dash_get_addr.o \
	dash_mem.o \
	eth_pkt_utils.o \
	platform_124mux.o \
	platform_sfp_cookie.o \
	platform_temp_sensor.o \
	netstat.o \
	bootflash_test.o \
	vg400_virtual_nim.o \
	platform_cpu.o \
	phy_reg_test.o \

PLAT_OBJS += \
	bcm_gesw_init.o \
	bcm_gesw_tests.o \
	bcm_gesw_utils.o \
	bcm_gesw.o \
	bcm_gesw_api.o \
	platform_eth_pkt_txrx.o \
	cavecreek_sgmii.o

PLAT_BASE = \
	diag.c \
	platform_cookie.c \
	platform_eth.c \
	phy_reg_test.c \
	platform_slot.c \
	$(UTAHDIR)/platform_cpu.c \
	$(UTAHDIR)/platform_i2c.c \
    $(UTAHDIR)/linux_main.c \
	$(UTAHDIR)/mb_tests.c \
	$(UTAHDIR)/platform_idprom_utils.c \
	$(UTAHDIR)/platform_pem_eeprom_modify.c \
	$(UTAHDIR)/platform_pem_utils.c \
	$(UTAHDIR)/platform_margin_utils.c \
	$(UTAHDIR)/platform_pwr_seq.c \
	$(UTAHDIR)/platform_sys_clk.c \
	$(UTAHDIR)/platform_gh_gesw_clk.c \
	$(UTAHDIR)/platform_stub.c \
	$(UTAHDIR)/platform_fan.c \
	$(UTAHDIR)/platform_led.c \
	$(UTAHDIR)/ht.c \
	$(UTAHDIR)/platform_ext_lpbk.c \
	$(UTAHDIR)/platform_sfp_ext_lpbk.c \
	$(UTAHDIR)/platform_sfp_cookie.c \
	$(UTAHDIR)/platform_macsec.c \
	$(OVERLORDDIR)/env_mcu_firmware.c \
	$(OVERLORDDIR)/linux_ntwk.c \
	$(OVERLORDDIR)/platform_psu.c \
	$(OVERLORDDIR)/platform_poe_psu.c \
	$(OVERLORDDIR)/platform_i2c_usb.c \
	$(OVERLORDDIR)/platform_pcie_clk.c \
	$(OVERLORDDIR)/platform_sensor.c \
	$(OVERLORDDIR)/platform_eeprom.c \
	$(OVERLORDDIR)/platform_eeprom_access.c \
	$(OVERLORDDIR)/platform_prom.c \
	$(OVERLORDDIR)/platform_30w_poe.c \
	$(OVERLORDDIR)/platform_pll.c \
	$(OVERLORDDIR)/platform_pcie_sw.c \
	$(OVERLORDDIR)/platform_intr_test.c \
	$(OVERLORDDIR)/tam_act2_api_drv_support.c \
	$(OVERLORDDIR)/platform_barometer.c \
	$(OVERLORDDIR)/moncmds.c \
	$(OVERLORDDIR)/i2c_drv.c \
	$(OVERLORDDIR)/ngio.c \
	$(OVERLORDDIR)/dash_reg_test.c \
	$(OVERLORDDIR)/dash_get_addr.c \
	$(OVERLORDDIR)/dash_fpga.c \
	$(OVERLORDDIR)/dash_mem.c \
	$(OVERLORDDIR)/platform_dimm.c \
	$(COMSRCDIR)/linux_usb_test.c \
	$(OVERLORDDIR)/eth_pkt_utils.c \
	$(OVERLORDDIR)/bcm_gesw_init.c \
	$(OVERLORDDIR)/bcm_gesw_tests.c \
	$(OVERLORDDIR)/bcm_gesw_utils.c \
	$(OVERLORDDIR)/bcm_gesw.c \
	$(OVERLORDDIR)/bcm_gesw_api.c \
	$(OVERLORDDIR)/platform_eth_pkt_txrx.c \
	$(UTAHDIR)/netstat.c \
	$(UTAHDIR)/bootflash_test.c \

PLAT_BASE += \
	$(OVERLORDDIR)/cavecreek_sgmii.c

PLAT_HFILES = \
	$(UTAHDIR)/i2c_address.h \
	$(UTAHDIR)/platform_i2c.h \
	$(UTAHDIR)/cookie_plat.h \
	$(UTAHDIR)/platform_pem_utils.h \
	$(UTAHDIR)/platform_idprom_utils.h \
	$(UTAHDIR)/platform_idprom.h \
	$(UTAHDIR)/mb_tests.h \
	$(UTAHDIR)/platform_ext_lpbk.h \
	$(UTAHDIR)/platform_sfp_cookie.h \
	$(UTAHDIR)/platform_macsec.h \
	$(UTAHDIR)/platform_fru.h \
	$(UTAHDIR)/i2c_address.h \
	$(UTAHDIR)/ht.h \
	$(UTAHDIR)/platform_eth.h \
	$(UTAHDIR)/platform_temp_sensor.h \
	$(OVERLORDDIR)/act2_api_drv_support.h \
	$(OVERLORDDIR)/act2_utils.h \
	$(OVERLORDDIR)/act2l_typedef.h \
	$(OVERLORDDIR)/ata_dev_dvr.h \
	$(OVERLORDDIR)/bcm_gesw.h \
	$(OVERLORDDIR)/bcm_gesw_defs.h \
	$(OVERLORDDIR)/ich_i2c.h \
	$(OVERLORDDIR)/linux_ntwk.h \
	$(OVERLORDDIR)/mfg_api.h \
	$(OVERLORDDIR)/mon_plat_defs.h \
	$(OVERLORDDIR)/nvmonvars.h \
	$(OVERLORDDIR)/pcmap.h \
	$(OVERLORDDIR)/platform_30w_poe.h \
	$(OVERLORDDIR)/platform_pcie_sw.h \
	$(OVERLORDDIR)/platform_barometer.h \
	$(OVERLORDDIR)/platform_cookie.h \
	$(OVERLORDDIR)/platform_debug.h \
	$(OVERLORDDIR)/platform_eeprom_access.h \
	$(OVERLORDDIR)/platform_eth_pkt_txrx.h \
	$(OVERLORDDIR)/platform_i2c_usb.h \
	$(OVERLORDDIR)/platform_intr_test.h \
	$(UTAHDIR)/platform_led.h \
	$(OVERLORDDIR)/platform_pci.h \
	$(OVERLORDDIR)/platform_pcie_clk.h \
	$(OVERLORDDIR)/platform_poe_psu.h \
	$(OVERLORDDIR)/platform_prom.h \
	$(OVERLORDDIR)/platform_psu.h \
	$(OVERLORDDIR)/platform_slot.h \
	$(UTAHDIR)/platform_pwr_seq.h \
	$(OVERLORDDIR)/platform_sensor.h \
	$(OVERLORDDIR)/platform_smi.h \
	$(UTAHDIR)/platform_sys_clk.h \
	$(UTAHDIR)/platform_gh_gesw_clk.h \
	$(OVERLORDDIR)/platform_usb_fw.h \
	$(UTAHDIR)/platform_margin_utils.h \
	$(OVERLORDDIR)/dash_fpga.h \
	$(OVERLORDDIR)/eth_pkt_utils.h \
	$(OVERLORDDIR)/ngio.h \
	$(OVERLORDDIR)/plat_defs.h \
	$(OVERLORDDIR)/platform_dimm.h \
	$(OVERLORDDIR)/uart_fpga.h \
    vg400_virtual_nim.h \

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#

NET_SRC = \
	$(PLAT_BASE) \
	$(COM_BASE) \
	$(DEV_BASE) \

NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(CRYPTO_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_PCI_UTILS) \
	$(LIB_QUACK) \

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(M29312_BASE) \
	$(PLAT_BASE) \
	$(PSE2_BASE) \
	$(PVDM_BASE) \
	$(PSE_BASE) \

#
#########   Header Files #########
#

HFILES  = \
        $(PLAT_HFILES) \
        $(COM_HFILES) \
        $(DEV_HFILES) \

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
	$(UTAHDIR): \
	$(SRCDIR): \
	$(OVERLORDDIR): \
	$(COMSRCDIR): \
	$(PROCSRCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(CX27470): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DIMM_SRC): \
	$(DOBRO_FW_PATH): \
	$(DSP_5510_FW_PATH): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(ENZO_FPGA_PATH): \
	$(ENZO_PLUS_FPGA_PATH) \
	$(ENZO_UCODE): \
	$(ENZOPLUS_UCODE) \
	$(FPGA_IMAGE_PATH): \
	$(GE_QUAD_PHY_SRC): \
	$(GUIDO_FPGA_PATH): \
	$(I2CMX_SRC): \
	$(INFORMERS_FPGA_PATH): \
	$(JEBBIN_FPGA_PATH): \
	$(M29312DIR): \
	$(NEZHA_DIR): \
	$(PADME_FPGA_PATH): \
	$(PATH_OF_BORG_FW): \
	$(PATH_OF_DSL_FW): \
	$(PATH_OF_BORG_FPGA): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(PVDM_BASE): \
	$(RTC_SRC): \
	$(LM75_SRC): \
	$(SIMPSONS_FPGA_PATH): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(Socrates): \
	$(SOPRANO_FPGA_PATH): \
	$(SPIDEY_FPGA_PATH): \
	$(SPIDEY_PLD): \
	$(SPIDEY_T3_UCODE): \
	$(SPIDEY_UCODE): \
	$(TDM_PLL_SRC): \
	$(USB_SRC): \
	$(MONTALVO_FW_PATH): \
	$(MARIO_FPGA_PATH): \
	$(PSE_FPGA_PATH): \
	$(PRECISION_IMAGE_PATH): \
	$(SQUIER_IMAGE_PATH): \
	$(TRIPLE_SEC_FPGA_PATH): \
	$(PATH_OF_BCM_FW): \
	$(TROI_FPGA_PATH): \
	$(CX98124): \
	$(TROI_BASE): \
	$(FPGA_PATH): \
	$(PLDOVLD): \
	$(GOOFY): \


vpath %.h \
	$(UTAHDIR): \
	$(SRCDIR): \
	$(OVERLORDDIR): \
	$(PROCINCDIR): \
	$(INCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	$(CHIPINC): \
	$(CLK1_SRC): \
	$(CLK2_SRC): \
	$(CLKBUF_SRC): \
	$(DEV_DRV)/dev_98dxn17_marvell: \
	$(DEV_DRV)/dev_ltc4215: \
	$(DEV_DRV)/dev_3086_infineon: \
	$(DEV_DRV)/dev_82912_infineon: \
	$(DIMM_SRC): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(GE_QUAD_PHY_SRC): \
	$(I2CMX_SRC): \
	$(M29312DIR): \
	$(NEZHA_INCDIR): \
	$(PSE2DIR): \
	$(PSE2_PHY_SRC): \
	$(PSE2_SRC): \
	$(LM75_SRC): \
	$(SKYDIR): \
	$(SM_PCLK_SRC): \
	$(SNSR_SRC): \
	$(POE30_SRC): \
	$(TAMLIB):

# Override RCS stuff

#%: %,v

#%: RCS/%,v

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

all: $(NETTARGET_BZ2)

#
# do not delete the last line for object dump just in case
# we need to do this when debugging a problem. temp comment out
#
#install: $(SRECFILE) $(NETTARGET) install.sh
#
# temp comment out ROM version due to compiler update problem
#

install: $(NETTARGET) install.sh
	sh install.sh
	touch install.sh

clean:
	rm -f *.o  banner.c \
	tools temp.* *.dat .compile ID.temp

clobber: clean
	rm -f moncode* pfsi syms*

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
	sh $(IROOT)/utils/banner.sh VG400
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) \
	-Wl,--start-group $(BCM_PLATFORM_DEFINES_OBJ) $(BCM_BLD_VERSION_OBJ) $(BCM_LIBS) \
	-Wl,--end-group ${BCM_LDFLAGS} $(LSPCI_LDFLAGS)

	@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

	touch dependancies
	touch $(NETTARGET).symbols
	touch $(NETTARGET).bz2.symbols

$(NETTARGET_BZ2): $(NETTARGET)
	bzip2 -9 -f -k $(NETTARGET)
	mv $(NETTARGET) $(NETTARGET).orig
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
# Revision 1.3  2018/09/03 04:23:51  haohsu
# Changing daily build server from sjc-ads-1686 to sjc-ads-9106
#
# Revision 1.2  2018/08/30 06:43:56  haohsu
# Collapse Vg400-branch to Main Trunk
#
# Revision 1.1.2.4  2018/08/01 02:17:32  haohsu
# Vg400 code change for branch
#
# Revision 1.1.2.3  2018/03/06 09:07:16  haohsu
# Fixed platform sku check fail
#
# Revision 1.1.2.2  2018/02/24 05:52:07  haohsu
# Code Change for VG400
#
# Revision 1.1.2.1  2018/01/26 08:20:35  haohsu
# *** empty log message ***
#
# Revision 1.4  2017/07/28 07:49:43  harrchan
# Collapse Oakenshield-branch to Main Trunk.
#
# Revision 1.3  2017/03/28 05:57:49  olin2
# Fix building issue (CSCvd69112)
#
# Revision 1.2  2016/10/16 12:28:20  iachang
# Supported Goldbeach Platform.
#
#
# $Endlog$


