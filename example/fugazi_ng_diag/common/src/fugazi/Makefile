# $Id: Makefile,v 1.2 2021/06/02 08:22:34 iachang Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/fugazi/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
#
# Copyright (c) 2018-2020 by Cisco Systems, Inc.
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


TOOLS_VER	= c4.7.0-p5
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

#
# The platform specific source directory
#

FUGAZIDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
NXP_LM75B_SRC = $(DEV_DRV)/dev_nxp_lm75b
EEPROM_SRC  = $(DEV_DRV)/dev_at24c0n_atmel
ENV_SRC     = $(DEV_DRV)/dev_csco_10698
USB_SRC     = $(DEV_DRV)/dev_cy7c64215_cypress
I2CMX_SRC   = $(DEV_DRV)/dev_pca9545a_ti
PCA9557_SRC = $(DEV_DRV)/dev_pca9557
M2_TC_SRC   = $(COMSRCDIR)/m2_testcard


# TAM lib. request : ENGTAM-1223
#TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/isr9k_curie1ru/aikido
#TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/WNBU/Katar_WLC/Aikido_v3_4_24/
TAMLIB  = $(IROOT)/common/lib/tam/
LIB_PCI_UTILS = $(IROOT)/common/lib/pci/libpci_gcc.c4.5.3-p1.x86_64.a
#LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

# Broadcom PHY EPDM lib
#BCM_PHY_PATH := /auto/neptune/vendor-src/broadcom/epdm
BCM_PHY_PATH := $(IROOT)/common/lib/epdm
BCM_MIURA_ROOT := $(BCM_PHY_PATH)/MIUR_1_8
BCM_MIURA_LIB := $(BCM_MIURA_ROOT)/build/libphymodepil-x86_64.a
BCM_QUADRA28_ROOT := $(BCM_PHY_PATH)/QUADRA28_1_2
BCM_QUADRA28_LIB := $(BCM_QUADRA28_ROOT)/build/libphymodepil-x86_64.a
BCM_EPDM_ROOT := $(BCM_PHY_PATH)/epdm_1_7_4
BCM_EPDM_LIB := $(BCM_EPDM_ROOT)/libepdm-x86_64.a
BCM_EPDM_INC := $(BCM_EPDM_ROOT)

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(FUGAZIDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(LTE_SWI_SRC) \
    -I$(DIMM_SRC) \
    -I$(EEPROM_SRC) \
    -I$(ENV_SRC) \
    -I$(USB_SRC) \
    -I$(BCM_EPDM_INC)\
	-I$(I2CMX_SRC) \
	-I$(PCA9557_SRC) \
	-I$(M2_TC_SRC)

#
# defines for compiler
#

DEFS = \
	-DFUGAZI \
	-DFUGAZI \
	-DLINUX_APP \
    -DTFTP_CLIENT  \
    -DAIKIDO_ACT2 \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DAIKIDO_SUPPORT_AIK \
	-DAIKIDO_DEV_KEY \
	-DAIKIDO_CT_DEV_KEY \
	-DRHEL8_GCC_4_7_0_P5 \
# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.
# AIKIDO_CT_DEV_KEY : CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue
# RHEL8_GCC_4_7_0_P5 : Migrate to RHEL8 Cisco IT gcc.c4.7.0-p5

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS         = -Wall -Werror -Wno-unused-but-set-variable -lrt $(INC) ${DEFS}
LDFLAGS         = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS          += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into FUGAZI project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJNAME = $(TAMLIB)/tam_lib_objname.a


#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= fugazilnx

#
# Common device driver objects
#
DEV_OBJS = \
		   dev_object.o \
		   dev_nxp_lm75b.o \
		   dev_at24c0n.o \
		   dev_csco_10698.o \
		   dev_cy7c64215.o \
		   dev_pca9545.o \
		   dev_pca9557.o \

DEV_BASE = \
		   $(DEV_DRV)/dev_object.c \
		   $(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		   $(DEV_DRV)/dev_object.c \
		   $(EEPROM_SRC)/dev_at24c0n.c \
		   $(ENV_SRC)/dev_csco_10698.c \
		   $(USB_SRC)/dev_cy7c64215.c \
		   $(I2CMX_SRC)/dev_pca9545.c \
		   $(PCA9557_SRC)/dev_pca9557.c \






DEV_HFILES = \
	$(DEV_DRV)/dev_object.h \
	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
	$(EEPROM_SRC)/dev_at24c0n.h \
    $(ENV_SRC)/dev_csco_10698.h \
	$(USB_SRC)/dev_cy7c64215.h \
	$(I2CMX_SRC)/dev_pca9545.h \
	$(PCA9557_SRC)/dev_pca9557.h \

#
# M.2 Testcard
#
M2_TC_OBJS = \
	m2_testcard.o \
	m2_testcard_host_impl.o \

M2_TC_BASE = \
	$(M2_TC_SRC)/m2_testcard.c \
	$(FUGAZIDIR)/m2_testcard_host_impl.c

M2_TC_HFILES = \
	$(M2_TC_SRC)/m2_testcard.h \
	$(FUGAZIDIR)/m2_testcard_host_impl.h


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
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
	linux_pciutils.o \
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
	linux_ntwk.o \
    linux_usb_test.o \
    linux_coretest.o \
	uio_utils.o \
	tam_aikido_upgrade.o \
	rtc.o \
	diag_eth_traf.o \
	platform_aikido_spi.o \
    diag_usb_test.o \
    diag_usb_api.o

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
	$(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
	$(COMSRCDIR)/linux_pciutils.c \
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
    $(COMSRCDIR)/linux_popen.c \
	$(COMSRCDIR)/linux_usb_test.c \
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/uio_utils.c \
	$(COMSRCDIR)/rtc.c \
    $(COMSRCDIR)/tam_aikido_upgrade.c


COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
    $(INCDIR)/common_utils.h \
    $(INCDIR)/linux_usb_test.h \
    $(INCDIR)/i2c_api.h \
    $(INCDIR)/i2c_dev.h \
    $(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/linux_ntwk.h \
	$(INCDIR)/linux_coretest.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/pca9545a.h \
	$(INCDIR)/tam_lib_manufacturing.h \
	$(INCDIR)/tam_lib_object_manager.h \
	$(INCDIR)/tam_library.h \

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files :
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	diag.o \
	mb_tests.o \
	platform_tam_cookie.o \
	legacy_smart_cookie.o \
	linux_popen.o \
	platform_stub.o \
	platform_i2c.o \
	i2c_api.o \
	i2c_drv.o \
	tam_act2_api_drv_support.o \
	platform_cpu.o \
	diag_i211_test.o \
	diag_bcm57412_test.o \
	diag_bcm57412_utils.o \
	dash_fpga.o \
    dash_reg_test.o \
    dash_get_addr.o \
    dash_mem.o \
    diag_boot_flash_test.o \
	platform_temp_sensor.o \
	platform_psu.o \
	platform_pwr_seq.o \
	platform_barometer.o \
	platform_i2c_usb.o \
	platform_prom.o \
	platform_fan.o \
	platform_led.o \
	platform_margin_utils.o \
	platform_intr_test.o \
	platform_vtg_mntr.o \
	diag_miura.o \
	diag_bnxt.o \
	diag_bcm_lib.o \
	diag_bcm82757_test.o \
	diag_lasi_test.o \
	diag_sideband_test.o \
	diag_common.o \
	platform_pem_utils.o \
	tam_lib_object_manager.o \
	platform_pem_monitor_read.o \
	platform_pem_eeprom_modify.o \
	platform_pem_fan.o \
	platform_idprom_utils.o \
	platform_124mux.o \
    linux_pci.o \
	platform_eth.o \
	eth_pkt_utils.o \
	platform_ext_lpbk.o \
	diag_bcm54194_api.o \
	diag_bcm54194_test.o \
	platform_synce_pll_utils.o \
	platform_sfp_ext_lpbk.o  \
	diag_aikido_test.o  \
	fpga_smartfan.o \
	tam_stubs.o \

PLAT_BASE = \
	$(TAMLIB)/tam_lib_object_manager.c \
	$(FUGAZIDIR)/linux_main.c \
	$(FUGAZIDIR)/diag.c \
	$(FUGAZIDIR)/mb_tests.c \
	$(FUGAZIDIR)/tam_act2_api_drv_support.c \
	$(FUGAZIDIR)/platform_tam_cookie.c \
	$(FUGAZIDIR)/legacy_smart_cookie.c \
	$(FUGAZIDIR)/platform_stub.c \
	$(FUGAZIDIR)/platform_i2c.c \
	$(FUGAZIDIR)/i2c_api.c \
	$(FUGAZIDIR)/i2c_drv.c \
	$(FUGAZIDIR)/platform_cpu.c \
	$(FUGAZIDIR)/diag_i211_test.c \
	$(FUGAZIDIR)/diag_bcm57412_test.c \
	$(FUGAZIDIR)/diag_bcm57412_utils.c \
	$(FUGAZIDIR)/dash_fpga.c \
    $(FUGAZIDIR)/dash_reg_test.c \
    $(FUGAZIDIR)/dash_get_addr.c \
    $(FUGAZIDIR)/dash_mem.c \
    $(FUGAZIDIR)/diag_boot_flash_test.c \
    $(FUGAZIDIR)/platform_temp_sensor.c \
    $(FUGAZIDIR)/platform_psu.c \
    $(FUGAZIDIR)/platform_pwr_seq.c \
    $(FUGAZIDIR)/platform_barometer.c \
    $(FUGAZIDIR)/platform_i2c_usb.c \
    $(FUGAZIDIR)/platform_prom.c \
    $(FUGAZIDIR)/platform_fan.c \
    $(FUGAZIDIR)/platform_led.c \
    $(FUGAZIDIR)/platform_margin_utils.c \
    $(FUGAZIDIR)/platform_vtg_mntr.c \
    $(FUGAZIDIR)/platform_intr_test.c \
	$(FUGAZIDIR)/diag_miura.c \
	$(FUGAZIDIR)/diag_bnxt.c \
	$(FUGAZIDIR)/diag_bcm_lib.c \
	$(FUGAZIDIR)/diag_bcm82757_test.c \
	$(FUGAZIDIR)/diag_lasi_test.c \
	$(FUGAZIDIR)/diag_sideband_test.c \
	$(FUGAZIDIR)/diag_common.c \
	$(FUGAZIDIR)/diag_eth_traf.c \
	$(FUGAZIDIR)/platform_idprom_utils.c \
	$(FUGAZIDIR)/platform_pem_utils.c \
	$(FUGAZIDIR)/platform_pem_monitor_read.c \
	$(FUGAZIDIR)/platform_pem_eeprom_modify.c \
	$(FUGAZIDIR)/platform_pem_fan.c \
	$(FUGAZIDIR)/platform_124mux.c \
	$(FUGAZIDIR)/platform_aikido_spi.c \
	$(FUGAZIDIR)/dash_get_addr.c \
	$(FUGAZIDIR)/linux_pci.c \
	$(FUGAZIDIR)/diag_usb_test.c \
	$(FUGAZIDIR)/diag_usb_api.c \
	$(FUGAZIDIR)/platform_eth.c \
	$(FUGAZIDIR)/platform_ext_lpbk.c \
	$(FUGAZIDIR)/eth_pkt_utils.c \
	$(FUGAZIDIR)/diag_bcm54194_api.c \
	$(FUGAZIDIR)/diag_bcm54194_test.c \
	$(FUGAZIDIR)/platform_synce_pll_utils.c \
	$(FUGAZIDIR)/platform_sfp_ext_lpbk.c \
	$(FUGAZIDIR)/diag_aikido_test.c \
	$(FUGAZIDIR)/fpga_smartfan.c \
	$(FUGAZIDIR)/tam_stubs.c \
	$(FUGAZIDIR)/linux_ntwk.c \

PLAT_HFILES = \
    $(FUGAZIDIR)/diag_common.h \
    $(FUGAZIDIR)/mb_tests.h \
    $(FUGAZIDIR)/tam_act2_api_drv_support.h \
    $(FUGAZIDIR)/platform_tam_cookie.h \
    $(FUGAZIDIR)/legacy_smart_cookie.h \
    $(FUGAZIDIR)/platform_stub.h \
    $(FUGAZIDIR)/platform_cpu.h \
    $(FUGAZIDIR)/platform_i2c.h \
	$(FUGAZIDIR)/diag_i211_test.h \
	$(FUGAZIDIR)/diag_bcm57412_test.h \
	$(FUGAZIDIR)/diag_bcm57412_utils.h \
    $(FUGAZIDIR)/i2c_address.h \
    $(FUGAZIDIR)/dash_fpga.h \
    $(FUGAZIDIR)/platform_mcu.h \
    $(FUGAZIDIR)/diag_boot_flash_test.h \
    $(FUGAZIDIR)/platform_temp_sensor.h \
    $(FUGAZIDIR)/platform_psu.h \
    $(FUGAZIDIR)/platform_pwr_seq.h \
    $(FUGAZIDIR)/platform_barometer.h \
    $(FUGAZIDIR)/platform_i2c_usb.h \
    $(FUGAZIDIR)/platform_prom.h \
    $(FUGAZIDIR)/platform_led.h \
    $(FUGAZIDIR)/platform_vtg_mntr.h \
    $(FUGAZIDIR)/platform_intr_test.h \
	$(FUGAZIDIR)/diag_miura.h \
	$(FUGAZIDIR)/diag_miura_reg.h \
	$(FUGAZIDIR)/diag_bnxt.h \
	$(FUGAZIDIR)/diag_bcm_lib.h \
	$(FUGAZIDIR)/diag_bcm82757_test.h \
	$(FUGAZIDIR)/diag_lasi_test.h \
	$(FUGAZIDIR)/diag_sideband_test.h \
	$(FUGAZIDIR)/diag_common.h \
	$(FUGAZIDIR)/platform_pem_utils.h \
	$(FUGAZIDIR)/platform_pem_fan.h \
    $(FUGAZIDIR)/platform_idprom_utils.h \
    $(FUGAZIDIR)/platform_idprom.h \
	$(FUGAZIDIR)/uart_fpga.h \
	$(FUGAZIDIR)/diag_usb_test.h \
	$(FUGAZIDIR)/platform_eth.h \
	$(FUGAZIDIR)/platform_ext_lpbk.h  \
	$(FUGAZIDIR)/eth_pkt_utils.h \
	$(FUGAZIDIR)/diag_bcm54194_api.h \
	$(FUGAZIDIR)/diag_bcm54194_test.h \
	$(FUGAZIDIR)/platform_synce_pll_utils.h \
	$(FUGAZIDIR)/diag_aikido_test.h \
	$(FUGAZIDIR)/fpga_smartfan.h \


#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_TAM_OBJNAME) \
    $(LIB_PCI_UTILS) \
	$(BCM_EPDM_LIB) \
	$(BCM_MIURA_LIB) \
	$(BCM_QUADRA28_LIB) \
	$(M2_TC_OBJS) \


#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE) \
	$(M2_TC_BASE)
#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
	$(M2_TC_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(FUGAZIDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(USB_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(I2CMX_SRC): \
	$(TAMLIB): \
	$(PCA9557_SRC): \
	$(M2_TC_SRC)

vpath %.h \
	$(GE_PHY_SRC): \
	$(FUGAZIDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(EEPROM_SRC): \
	$(ENV_SRC): \
	$(USB_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(LTE_SWI_SRC): \
	$(I2CMX_SRC): \
	$(TAMLIB): \
	$(PCA9557_SRC): \
	$(M2_TC_SRC)

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
	.compile ID.temp \

clobber: clean
	rm -f dependancies moncode* pfsi syms* $(NETTARGET)*

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
	@echo "FUGAZI_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "FUGAZIDIR: " $(FUGAZIDIR)
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
	sh $(IROOT)/utils/banner.sh FUGAZI_INTEL
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS}

	-@$(OBJDUMP) -t $(NETTARGET) > $(NETTARGET).symbols

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
	#/router/bin/etags -t $(DEPEND_LST)
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

######## HISTORY ########
# $Log: Makefile,v $
# Revision 1.2  2021/06/02 08:22:34  iachang
# CSCvo59196-33 : Merge Fugazi from ASR1K-main-branch to ISR main trunk
#
# Revision 1.1.8.8  2021/04/26 08:15:25  iachang
# CSCvy10910:Fugazi Diag supportted M.2 test card
#
# Revision 1.1.8.7  2021/04/14 06:59:45  iachang
# CSCvo59196-28 : Add Aikido RNG test
#                 Fixed tam_lib_device_open_mailbox() input NULL pointer issue.
#
# Revision 1.1.8.6  2021/02/03 06:28:07  iachang
# CSCvx12993-2: Fugazi migrate Diag App from RHEL7 to HREL8
#
# Revision 1.1.8.5  2020/09/04 08:20:22  iachang
# Modify make clean to fix daily build issue.
#
# Revision 1.1.8.4  2020/09/03 06:12:23  iachang
# Fixed daily build issue.
#
# Revision 1.1.8.3  2020/08/26 02:37:47  iachang
# Merge Fugazi code into main trunk
#
# Revision 1.1.8.2  2020/08/05 14:20:12  iachang
# Initial check in.
#
# Revision 1.1.6.16  2020/08/19 07:20:38  iachang
# CSCvu36133 : Fixed WDC install cause consent token install and remove DEV key more complicated issue
#
# Revision 1.1.6.15  2020/08/04 08:37:05  iachang
# Update Copyright to 2020
#
# Revision 1.1.6.14  2020/06/09 08:14:44  iachang
# Aikido FPGA DEV key utility.
# Fixed WDC install impact Consent token issue.
#
# Revision 1.1.6.13  2020/06/08 06:54:39  iachang
# Program Aikido FPGA DEV key utility
# Program I211 and BCM57412 MAC address utility.
#
# Revision 1.1.6.12  2020/04/24 07:13:16  iachang
# The block device access test, used fix size to replace malloc_usable_size()
#
# Revision 1.1.6.11  2020/03/19 06:31:40  iachang
# Support Fugazi Side Band test
#
# Revision 1.1.6.10  2020/03/18 06:51:44  iachang
# Create independent file for LASI test
#
# Revision 1.1.6.9  2019/10/16 06:12:31  letsai
# Modify file name
#
# Revision 1.1.6.8  2019/09/27 08:03:58  iachang
# Changed "BCM57412 SFP i2c Test" from Broadcom CDiag to Netlink.
#
# Revision 1.1.6.7  2019/07/19 07:35:28  letsai
# 1. Support LED control.
# 2. Support smart fan.
# 3. Change BCM 54194 phy reset bit.
#
# Revision 1.1.6.6  2019/06/17 06:02:18  iachang
# Rename eth_traf.c/eth_traf.h to diag_eth_traf.c/diag_eth_traf.h
# Add ethernet packet check message into cterr.
#
# Revision 1.1.6.5  2019/05/08 01:52:20  iachang
# Fixed compile "undefined reference to clock_gettime" issue.
#
# Revision 1.1.6.4  2019/04/06 01:36:14  letsai
# 1. Remove unused functions and files.
# 2. Fix BCM54194 SFP External loopback test.
# 3. Fix BCM54194 Register test.
# 4. Fix Voltage Margin Utility.
# 5. Add function to show system information.
#
# Revision 1.1.6.3  2019/03/26 17:16:53  iachang
# Add Aikido register test.
#
# Revision 1.1.6.2  2019/03/14 03:49:12  letsai
# Initial check in.
#
#
#
#
# $Endlog$



