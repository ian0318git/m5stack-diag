# $Id: Makefile,v 1.9 2020/05/22 02:28:40 qingcwan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/x86/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Neptune x86 diag image
#
# Adapted from overlord/x86.
#
# Copyright (c) 2016-2019 by Cisco Systems, Inc.
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
IROOT		= ../../../../..
INCDIR		= $(IROOT)/common/include
FPGA_PATH	= /auto/sp-engops/diags/pld/fpga_image

#
# The platform specific source directory
#

SRCDIR		= .
OVERLORDDIR     = ../..
NEPDIR = $(IROOT)/common/src/overlord/neptune
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
CANIS_SRC_PATH  = $(COMSRCDIR)/canis
WOODLAWN_SRC_PATH  = $(COMSRCDIR)/woodlawn
GOLDSCHLAGER_SRC_PATH = $(COMSRCDIR)/goldschlager
TIMINGCARD_SRC_PATH  = $(COMSRCDIR)/timingcard
SHEDIR_SRC_PATH = $(COMSRCDIR)/shedir
DRACO_SRC_PATH = $(COMSRCDIR)/draco
RBCP_LIB_PATH = $(COMSRCDIR)/rbcp_lib
PLDOVLD = /nfs/sp-engops/diags/pld/overlord
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/ISR4K_2099_SUDI
DREAMLINER_SRC_PATH  = $(COMSRCDIR)/dreamliner
DREAMLINER_PHY_PATH  = $(COMSRCDIR)/dreamliner/phy
DREAMLINER_CPSS_PATH  = $(COMSRCDIR)/dreamliner/cpss
DREAMLINER_PLD_PATH = /auto/sp-engops/diags/pld/dreamliner
AQUILA_SRC_PATH = $(COMSRCDIR)/aquila
KALAMATA_SRC_PATH = $(COMSRCDIR)/kalamata
OAKENSHIELD_SRC_PATH = $(COMSRCDIR)/oakenshield
NIGHTWATCH_SRC_PATH = $(COMSRCDIR)/nightwatch

#
# Path to fpga images
#
PLDDIR			= /auto/sp-engops/diags/pld
PLDLIB			= $(PLDDIR)/lib
PATRIOT_FW_PATH         = /auto/sp-engops/diags/pld/patriot


# BCM GE switch related defines
#
BCM_SDK_ROOT = /auto/neptune/vendor-src/broadcom/bcm-gesw-sdk
BCM_SDK_INCDIR = $(BCM_SDK_ROOT)/include
BCM_SYSTEMS_DIR = $(BCM_SDK_ROOT)/systems

#
# The BCM *.a lib files are copied from the build directory of the BCM tree
# to the /auto/sp-engops/diags/pld/overlord/bcm_lib automatically when
# user run the build-for-ovld script in the BCM tree.
# BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
#
# BCM_LIB_DIR = /auto/sp-engops/diags/pld/overlord/bcm_lib
# pfix-orig BCM_LIB_DIR = /auto/utah/bcm-lib
BCM_LIB_DIR = $(BCM_SDK_ROOT)/build/unix-user/csco-ovld-2_6
BCM_BLD_VERSION_OBJ = $(BCM_LIB_DIR)/version.o
BCM_PLATFORM_DEFINES_OBJ = $(BCM_LIB_DIR)/platform_defines.o

BCM_LDFLAGS =  -lpthread -lm -lrt -Wl,--relax
BCM_DEFS = \
	-DSYS_BE_PIO=0 -DSYS_BE_PACKET=0 -DSYS_BE_OTHER=0 \
	-DBCM_ESW_SUPPORT \
	-DUNIX \
	-DBCM_WARM_BOOT_SUPPORT_SW_DUMP \


# Cavium remote lib
#
CVMX_SDK_ROOT = /auto/neptune/vendor-src/cavium/cvm-sdk-3.1.2/Cavium_Networks/OCTEON-SDK/
CVMX_REMOTE_LIB_PATH = $(CVMX_SDK_ROOT)/host/remote-lib
CVMX_REMOTE_LIB = $(CVMX_REMOTE_LIB_PATH)/libocteon-remote.a

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(SRCDIR) \
	-I$(INCDIR) \
	-I$(OVERLORDDIR) \
	-I$(NEPDIR) \
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
	-I$(TIMINGCARD_SRC_PATH) \
	-I$(SHEDIR_SRC_PATH) \
	-I$(DRACO_SRC_PATH) \
	-I$(RBCP_LIB_PATH) \
	-I$(AQUILA_SRC_PATH) \
	-I$(KALAMATA_SRC_PATH) \
	-I$(OAKENSHIELD_SRC_PATH) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(TAMLIB)

INC     += \
	-I$(DREAMLINER_SRC_PATH) \
	-I$(DREAMLINER_PHY_PATH) \
	-I$(DREAMLINER_CPSS_PATH)/include/ \
	-I$(DREAMLINER_CPSS_PATH)/include/cisco \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpssCommon/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/driver/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDr \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/extDrv/drivers \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/os/gtOs \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/extServices/trace \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/events/private \
	-I$(DREAMLINER_CPSS_PATH)/include/common/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxCh3/policer/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cos/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/cutThrough \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/fabric \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipfix \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ipLpmEngine/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/l2mll \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/logicalTarget \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/mirror \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/multiPortGroup \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/oam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/PizzaArbiter/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/SliceManipulation/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/private/routerTunnelTermTcam \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/ptp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/resourceManager\
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/catchUp/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tti \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/dxCh/dxChxGen/vnt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/bridge/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/config/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cos \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cpssHwInit/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/cscd \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/diag/private/serdesOpt/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/dragonite/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/extMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/i2c/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/init \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/interrupts/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/ip \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/mirror/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/networkIf/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/nst \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/pcl \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/phy/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/policer \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/configElementDb \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/common/siliconIf \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/ddr3/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/cgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/geMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/hglMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/ilknMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/tcamMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/mac/xlgMac \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/cgPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/gPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/hglPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/ilknPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/mmPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/tcamPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/pcs/xPcs \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyH \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/serdes/comPhyHRev2 \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/port/private/portHwConfig/port/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/smi \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/systemRecovery/hsu/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/trunk/private \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/tunnel \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpss/generic/version \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/config/generic \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/hardware \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/dxExMx \
	-I$(DREAMLINER_CPSS_PATH)/include/mainPpDrv/h/cpssDriver/pp/interrupts/generic \

INC	+= -I$(BCM_SDK_INCDIR) -I$(BCM_SYSTEMS_DIR) -I$(CVMX_REMOTE_LIB_PATH)

#
# defines for compiler
#  added -DX86 to pick up WDC support for Intel platform.
# add -DSREC2BIN for power sequencer fw upgrade
#

# SUDI_2099 support notes:
# 1. If TAM lib. version newer than V3.1.1, add "-DSUDI_2099_2029" to use the new TAM lib which support 2099 SUDI.
# 2. If MFG need to program 2099 SUDI, add "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.
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
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI 

DREAMLINER_DEFS = \
	-DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS

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
CPPOPTS     	= -Wall -Werror $(INC) ${DEFS} ${BCM_DEFS} $(DREAMLINER_DEFS)
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
ASFLAGS		= $(TOOLS_FLAGS) -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}

#test, try to compile library in big-endian by ICC
NEW_CLI_OBJS = cli_cmd.o
NEW_CLI_BASE = cli_cmd.c
NEW_CLI_HFILES = cli_cmd.h

#LIBDIR		= $(IROOT)/lib
#LIBSRCDIR	= $(LIBDIR)/src
#LIB_AUTH	= smart_cookie_auth_c2_95_3-p11a-linux.a

# overlord is 64 bit
LIB_AUTH_DIR = /auto/sp-engops/diags/pld/lib/intel/Linux

# achanta is 32 bit lib, overlord is 64 bit
PLDLIB_INTEL_LINUX_ACHANTA = /auto/sp-engops/diags/pld/lib/intel/Linux

LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a

#LIB_CRYPTO = $(PLDLIB_INTEL_LINUX_ACHANTA)/cryptoc_rsa_toolkit.a
#LIB_CERTC = $(PLDLIB_INTEL_LINUX_ACHANTA)/certc_rsa_toolkit.a
LIB_QUACK = $(PLDLIB_INTEL_LINUX_ACHANTA)/ec_rsa_toolkit.a

LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

# BCM GE Switch LIB
BCM_LIBS = $(wildcard $(BCM_LIB_DIR)/lib*.a)

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= nepx86_diag
NETELF		= $(NETTARGET).elf
NETTARGET_BZ2	= $(NETTARGET).bz2

#
# Use -Qoption,bepostld,-verbose to turn on postld verbose 
# to see whether postld do the necessary big-endian global pointers swap
# POSTLDVERBOSE	= -Qoption,bepostld,-verbose
#
POSTLDVERBOSE	=
#RAMLDFLAGS	= -Xlinker -e -Xlinker -Tinformersram.link \

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
	linux_pci.o \
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
	tam_act2_utils.o \
        srec2bin.o \
        adapter_fpga.o

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
	$(INCDIR)/adapter_fpga.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h

# COM_BASE += $(COMSRCDIR)/bcm56321.c
# COM_BASE += $(COMSRCDIR)/example_fw_dnld.c

#########################################
# !!!!! From Xformers for reference !!!!!
#########################################
#
# Platform Based Object files : main_net.o must be first.
# This will ensure that _start is first code in .text section.
#

PLAT_OBJS = \
	platform_stub.o \
	diag.o \
	VMON_MCU_01_01.o \
	igb_eeprom.o \
	ich9_i2c.o \
	platform_i2c.o \
        platform_124mux.o\
	platform_i2c_usb.o \
	i2c_api.o \
	platform_margin_utils.o \
	platform_pwr_seq.o \
	platform_vtg_mntr.o \
        platform_vr.o \
	platform_sys_clk.o \
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
	platform_pcie_sw.o \
	platform_plx_pcie_sw.o \
	platform_pi_pcie_sw.o \
	platform_idprom_utils.o \
	tam_act2_api_drv_support.o \
	platform_dimm.o \
	mb_tests.o \
	platform_cookie.o \
	platform_barometer.o \
	moncmds.o \
	dash_fpga.o \
	dash_reg_test.o \
	dash_get_addr.o \
	dash_mem.o \
	eth_pkt_utils.o \
        platform_temp_sensor.o \
        platform_fan.o \

PLAT_OBJS += \
	bcm_gesw_init.o \
	bcm_gesw_tests.o \
	bcm_gesw_utils.o \
	bcm_gesw.o \
	bcm_gesw_api.o \
	platform_eth_pkt_txrx.o \
	cavecreek_sgmii.o

PLAT_BASE = \
	linux_main.c \
	diag.c \
	mb_tests.c \
	platform_i2c.c \
	platform_idprom_utils.c \
	platform_stub.c \
	platform_led.c \
	$(OVERLORDDIR)/linux_ntwk.c \
	platform_margin_utils.c \
	platform_sys_clk.c \
	platform_vtg_mntr.c \
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
	$(OVERLORDDIR)/platform_plx_pcie_sw.c \
	$(NEPDIR)/platform_pi_pcie_sw.c \
	$(OVERLORDDIR)/octeon_test.c \
	$(OVERLORDDIR)/host_mbox.c \
	$(OVERLORDDIR)/platform_intr_test.c \
	$(OVERLORDDIR)/tam_act2_api_drv_support.c \
	$(OVERLORDDIR)/platform_cookie.c \
	$(OVERLORDDIR)/platform_barometer.c \
	$(OVERLORDDIR)/moncmds.c \
	$(OVERLORDDIR)/i2c_drv.c \
	$(OVERLORDDIR)/ngio.c \
	$(OVERLORDDIR)/dash_fpga.c \
	$(OVERLORDDIR)/dash_reg_test.c \
	$(OVERLORDDIR)/dash_get_addr.c \
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
        $(OVERLORDDIR)/platform_slot.c \
        platform_124mux.c \
	platform_fan.c \
	platform_pwr_seq.c \
        platform_vr.c \


PLAT_BASE += \
	$(OVERLORDDIR)/cavecreek_sgmii.c

PLAT_HFILES = \
	mb_tests.h \
	cookie_plat.h \
        platform_idprom_utils.h \
        platform_idprom.h \
	platform_i2c.h \
	platform_fru.h \
	i2c_address.h \
	platform_pwr_seq.h \
	platform_led.h \
        platform_vr.h \
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
	$(OVERLORDDIR)/octeon_test.h \
	$(OVERLORDDIR)/pcmap.h \
	$(OVERLORDDIR)/platform_30w_poe.h \
	$(OVERLORDDIR)/platform_pcie_sw.h \
	$(OVERLORDDIR)/platform_plx_pcie_sw.h \
	$(NEPDIR)/platform_pi_pcie_sw.h \
	$(OVERLORDDIR)/platform_barometer.h \
	$(OVERLORDDIR)/platform_cookie.h \
	$(OVERLORDDIR)/platform_debug.h \
	$(OVERLORDDIR)/platform_eeprom_access.h \
	$(OVERLORDDIR)/platform_eth_pkt_txrx.h \
	$(OVERLORDDIR)/platform_i2c_usb.h \
	$(OVERLORDDIR)/platform_intr_test.h \
	$(OVERLORDDIR)/platform_pci.h \
	$(OVERLORDDIR)/platform_pcie_clk.h \
	$(OVERLORDDIR)/platform_poe_psu.h \
	$(OVERLORDDIR)/platform_prom.h \
	$(OVERLORDDIR)/platform_psu.h \
	$(OVERLORDDIR)/platform_sensor.h \
	$(OVERLORDDIR)/platform_smi.h \
	$(OVERLORDDIR)/platform_usb_fw.h \
	platform_vtg_mntr.h \
	platform_margin_utils.h \
	$(OVERLORDDIR)/dash_fpga.h \
	$(OVERLORDDIR)/eth_pkt_utils.h \
	$(OVERLORDDIR)/ngio.h \
	$(OVERLORDDIR)/plat_defs.h \
	$(OVERLORDDIR)/platform_dimm.h \
        $(OVERLORDDIR)/platform_slot.h \
	$(OVERLORDDIR)/uart_fpga.h

PATRIOT_OBJS = \
	sm_patriot.o \
	sm_patriot_test.o \
	sm_patriot_utils.o \

PATRIOT_BASE = \
	$(COMSRCDIR)/sm_patriot.c \
	$(COMSRCDIR)/sm_patriot_test.c \
	$(COMSRCDIR)/sm_patriot_utils.c \

PATRIOT_HFILES = \
	$(INCDIR)/sm_patriot.h

LEBOWSKI_OBJS = \
	lebowski_host.o

LEBOWSKI_BASE = \
	$(COMSRCDIR)/lebowski_host.c

LEBOWSKI_HFILES = \
	$(INCDIR)/lebowski_host.h

NIGHTWATCH_OBJS = \
	nightwatch_host.o \
	nightwatch_diag_shell_api.o \
	pty.o

NIGHTWATCH_BASE = \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_host.c \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_diag_shell_api.c \
	$(NIGHTWATCH_SRC_PATH)/pty.c

NIGHTWATCH_HFILES = \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_host.h \
	$(NIGHTWATCH_SRC_PATH)/nightwatch_diag_shell_api.h \
	$(NIGHTWATCH_SRC_PATH)/pty.h

FORTITUDE_OBJS = ngwic_fortitude.o
GRIMLOCK_OBJS = nim_grimlock.o

FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c 
GRIMLOCK_BASE = $(COMSRCDIR)/nim_grimlock.c

AQUILA_OBJS = \
	ngsm_aquila.o \
	aquila_rbcp_main.o

AQUILA_BASE = \
	$(AQUILA_SRC_PATH)/ngsm_aquila.c \
	$(AQUILA_SRC_PATH)/aquila_rbcp_main.c

KALAMATA_OBJS = \
	nim_kalamata.o \
	kalamata_rbcp_main.o

KALAMATA_BASE = \
	$(KALAMATA_SRC_PATH)/nim_kalamata.c \
	$(KALAMATA_SRC_PATH)/kalamata_rbcp_main.c

OAKENSHIELD_OBJS = \
	ngsm_oakenshield.o 

OAKENSHIELD_BASE = \
	$(OAKENSHIELD_SRC_PATH)/ngsm_oakenshield.c \

DREAMLINER_OBJS = \
	ngwic_dreamliner.o \
	dreamliner_phy.o \
	dreamliner_fpga.o \
	dreamliner_poe.o \
	dreamliner_ge_switch.o \
	dreamliner_xcat2_init.o \
	dreamliner_xcat2_utils.o \
	dreamliner_fpga_fw.o \
	nim_dm_cpss_extdrv.o \
	nim_dm_cpss_extserv.o \
	nim_dm_cpss_os.o \
	nim_dm_cpss_trace.o \


DREAMLINER_BASE = \
	$(DREAMLINER_SRC_PATH)/ngwic_dreamliner.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_phy.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_fpga.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_poe.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_ge_switch.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_init.c \
	$(DREAMLINER_SRC_PATH)/dreamliner_xcat2_utils.c \
	$(DREAMLINER_PLD_PATH)/dreamliner_fpga_fw.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extdrv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_extserv.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_os.c \
	$(DREAMLINER_SRC_PATH)/nim_dm_cpss_trace.c \

DREAMLINER_LIB = \
	$(DREAMLINER_PLD_PATH)/libdreamlinersdk.a

PRINCE_OBJS = ngwic_prince.o

PRINCE_BASE = $(COMSRCDIR)/ngwic_prince.c 

REVA_OBJS = ngwic_reva.o

REVA_BASE = $(COMSRCDIR)/ngwic_reva.c

THULE_OBJS = ngsm_thule.o

THULE_BASE = $(COMSRCDIR)/ngsm_thule.c

OVERDRIVE_OBJS = ngwic_overdrive.o

OVERDRIVE_BASE = $(COMSRCDIR)/ngwic_overdrive.c 

CANIS_OBJS = \
	sm_canis.o \
	canis_rbcp_main.o \
	canis_rbcp_lib.o \
	canis_rbcp_platform.o

CANIS_BASE = \
	$(CANIS_SRC_PATH)/sm_canis.c \
	$(CANIS_SRC_PATH)/canis_rbcp_main.c \
	$(CANIS_SRC_PATH)/canis_rbcp_lib.c \
	$(CANIS_SRC_PATH)/canis_rbcp_platform.c

WOODLAWN_OBJS = \
	sm_woodlawn.o \
	sm_woodlawn_uart.o \
	sm_woodlawn_comm.o \
	sm_woodlawn_fpga_lib.o

WOODLAWN_BASE = \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_uart.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_comm.c \
	$(WOODLAWN_SRC_PATH)/sm_woodlawn_fpga_lib.c

GOLDSCHLAGER_OBJS = \
	ngwic_goldschlager.o \
	ngwic_goldschlager_comm.o \
	bcm63268_lib.o

GOLDSCHLAGER_BASE = \
	$(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager.c \
	$(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager_comm.c \
	$(GOLDSCHLAGER_SRC_PATH)/bcm63268_lib.c 

TIMINGCARD_OBJS = \
	jbicomp.o \
	jbijtag.o \
	jbimain.o \
	jbistub.o \
	vm_timingcard.o \
	vm_timingcard_cpld_diag.o \
	vm_timingcard_cpld_lib.o \
	vm_timingcard_cpld_fw.o \
	vm_timingcard_pca9557_diag.o \
	vm_timingcard_pca9557_lib.o \
	vm_timingcard_zl3036x_lib.o \
	vm_timingcard_zl3036x_diag.o

TIMINGCARD_BASE = \
	$(TIMINGCARD_SRC_PATH)/jbicomp.c \
	$(TIMINGCARD_SRC_PATH)/jbijtag.c \
	$(TIMINGCARD_SRC_PATH)/jbimain.c \
	$(TIMINGCARD_SRC_PATH)/jbistub.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_diag.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_cpld_fw.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_pca9557_diag.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_pca9557_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_zl3036x_lib.c \
	$(TIMINGCARD_SRC_PATH)/vm_timingcard_zl3036x_diag.c 
SHEDIR_OBJS = \
	ngwic_shedir.o \
	shedir_rbcp_main.o \
	rbcp_platform.o \
	rbcp_lib.o 

SHEDIR_BASE = \
	$(SHEDIR_SRC_PATH)/ngwic_shedir.c \
	$(SHEDIR_SRC_PATH)/shedir_rbcp_main.c \
	$(SHEDIR_SRC_PATH)/rbcp_platform.c \
	$(RBCP_LIB_PATH)/rbcp_lib.c

DRACO_OBJS = \
	ngsm_draco.o \
	draco_rbcp_main.o

DRACO_BASE = \
	$(DRACO_SRC_PATH)/ngsm_draco.c \
	$(DRACO_SRC_PATH)/draco_rbcp_main.c

GRAFFHAM_OBJS = ngvm_graffham.o

GRAFFHAM_BASE = $(COMSRCDIR)/ngvm_graffham.c

F35_OBJS = nim_f35.o

F35_BASE = $(COMSRCDIR)/nim_f35.c 

WALLANDER_OBJS = nim_wallander.o

WALLANDER_BASE = $(COMSRCDIR)/nim_wallander.c

ARKENSTONE_OBJS = nim_arkenstone.o

ARKENSTONE_BASE = $(COMSRCDIR)/nim_arkenstone.c 

#
# Test Card related files
#
TESTCARD_OBJS = \
	ngio_testcard.o \
	testcard_fpga.o \
	testcard_eth.o \
	testcard_xaui.o \
	testcard_pcie.o \
        testcard_plx_pcie_sw.o \
        testcard_tlk_10232.o \
	testcard_uart.o

TESTCARD_BASE = \
	$(OVERLORDDIR)/ngio_testcard.c \
	$(OVERLORDDIR)/testcard_fpga.c \
	$(OVERLORDDIR)/testcard_eth.c \
	$(OVERLORDDIR)/testcard_xaui.c \
	$(OVERLORDDIR)/testcard_pcie.c \
	$(OVERLORDDIR)/testcard_plx_pcie_sw.c \
	$(OVERLORDDIR)/testcard_tlk_10232.c \
	$(OVERLORDDIR)/testcard_uart.c

TESTCARD_HFILES = \
	$(OVERLORDDIR)/ngio_testcard.h \
	$(OVERLORDDIR)/testcard_fpga.h \
	$(OVERLORDDIR)/testcard_eth.h \
	$(OVERLORDDIR)/testcard_plx_pcie_sw.h \
	$(OVERLORDDIR)/testcard_tlk_10232.h \
	$(OVERLORDDIR)/testcard_xaui.h \
	$(OVERLORDDIR)/testcard_pcie.h

#
# Dynamo NIM
#
DYNAMO_OBJS = nim_dynamo.o
DYNAMO_BASE = $(COMSRCDIR)/nim_dynamo.c

#
# ShrinkRay SM
#
SHRINKRAY_OBJS = \
	shrinkray_host.o \
	shrinkray_utils.o

SHRINKRAY_BASE = \
	$(COMSRCDIR)/shrinkray_host.c \
	$(COMSRCDIR)/shrinkray_utils.c

SHRINKRAY_HFILES = \
	$(INCDIR)/shrinkray_host.h

#
# Skye SM
#
SKYE_OBJS = \
	skye_host.o \
	skye_utils.o

SKYE_BASE = \
	$(COMSRCDIR)/skye_host.c \
	$(COMSRCDIR)/skye_utils.c

SKYE_HFILES = \
	$(INCDIR)/skye_host.h

#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	octeon_test.o \
	host_mbox.o \
	$(PATRIOT_OBJS) \
	$(CANIS_OBJS) \
	$(WOODLAWN_OBJS) \
	$(GOLDSCHLAGER_OBJS) \
	$(SHEDIR_OBJS) \
	$(DRACO_OBJS) \
	$(FORTITUDE_OBJS) \
	$(GRIMLOCK_OBJS) \
	$(DREAMLINER_OBJS) \
	$(DREAMLINER_LIB) \
	$(PRINCE_OBJS) \
	$(REVA_OBJS) \
	$(OVERDRIVE_OBJS) \
	$(THULE_OBJS) \
	$(CVMX_REMOTE_LIB) \
	$(CRYPTO_OBJS) \
	$(LIB_PCI_UTILS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_QUACK) \
	$(GRAFFHAM_OBJS) \
	$(TESTCARD_OBJS) \
	$(LEBOWSKI_OBJS) \
	$(NIGHTWATCH_OBJS) \
	$(DYNAMO_OBJS) \
	$(SHRINKRAY_OBJS) \
	$(F35_OBJS) \
	$(WALLANDER_OBJS) \
	$(TIMINGCARD_OBJS) \
	$(SKYE_OBJS) \
	$(AQUILA_OBJS) \
	$(KALAMATA_OBJS) \
	$(OAKENSHIELD_OBJS) \
	$(ARKENSTONE_OBJS)

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
	$(PATRIOT_BASE) \
	$(FORTITUDE_BASE) \
	$(GRIMLOCK_BASE) \
	$(DREAMLINER_BASE) \
	$(PRINCE_BASE) \
	$(REVA_BASE) \
	$(OVERDRIVE_BASE) \
	$(THULE_BASE) \
	$(CANIS_BASE) \
	$(WOODLAWN_BASE) \
	$(GOLDSCHLAGER_BASE) \
	$(SHEDIR_BASE)      \
	$(DRACO_BASE)      \
	$(GRAFFHAM_BASE) \
	$(TESTCARD_BASE) \
	$(LEBOWSKI_BASE) \
	$(NIGHTWATCH_BASE) \
	$(DYNAMO_BASE) \
	$(SHRINKRAY_BASE) \
	$(F35_BASE) \
	$(WALLANDER_BASE) \
	$(TIMINGCARD_BASE) \
	$(SKYE_BASE) \
	$(AQUILA_BASE) \
	$(KALAMATA_BASE) \
	$(ARKENSTONE_BASE)

#
#########   Header Files #########
#

HFILES  = \
        $(COM_HFILES) \
        $(DEV_HFILES) \
	$(PATRIOT_HFILES) \
	$(TESTCARD_HFILES) \
        $(PLAT_HFILES) \
	$(LEBOWSKI_HFILES) \
	$(NIGHTWATCH_HFILES) \
	$(SHRINKRAY_HFILES) \
	$(SKYE_HFILES)

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
	$(NEPDIR): \
	$(OVERLORDDIR): \
	$(COMSRCDIR): \
	$(PROCSRCDIR): \
	$(GS_4WIRE): \
	$(DEV_DRV): \
	\
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
	$(PATRIOT_FW_PATH): \
	$(CANIS_SRC_PATH): \
	$(WOODLAWN_SRC_PATH): \
	$(GOLDSCHLAGER_SRC_PATH): \
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PLD_PATH): \
	$(DREAMLINER_PHY_PATH): \
	$(DREAMLINER_CPSS_PATH)/src: \
	$(PLDOVLD): \
	$(GOOFY): \
	$(SHEDIR_SRC_PATH): \
	$(AQUILA_SRC_PATH): \
	$(KALAMATA_SRC_PATH): \
	$(OAKENSHIELD_SRC_PATH): \
	$(DRACO_SRC_PATH): \
	$(RBCP_LIB_PATH): \
	$(TIMINGCARD_SRC_PATH): \
	$(NIGHTWATCH_SRC_PATH):

vpath %.h \
	$(SRCDIR): \
	$(OVERLORDDIR): \
	$(NEPDIR): \
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
	$(DREAMLINER_SRC_PATH): \
	$(DREAMLINER_PHY_PATH): \
	$(RBCP_LIB_PATH): \
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
	$(OBJDUMP) -l --disassemble --source $(NETELF) > $(NETTARGET).dis


clean:
	rm -f *.o  banner.c \
	tools temp.* *.dat .compile ID.temp

clobber: clean
	rm -f moncode* pfsi syms* $(NETTARGET)*

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
	sh $(IROOT)/utils/banner.sh NEPTUNE
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) \
	-Wl,--start-group $(BCM_PLATFORM_DEFINES_OBJ) $(BCM_BLD_VERSION_OBJ) $(BCM_LIBS) \
	-Wl,--end-group ${BCM_LDFLAGS}  $(LSPCI_LDFLAGS)

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
	$(shell which etags) $(DEPEND_LST)
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
# Revision 1.9  2020/05/22 02:28:40  qingcwan
# Merge switzer-carrier code into main chunk.
#
# Revision 1.8  2020/03/13 12:14:52  letsai
# Support Grimlock NIM
#
# Revision 1.7  2019/08/06 06:56:15  alpeng
# merge curie, switzer and nightwatch to trunk
#
# Revision 1.6  2018/08/28 18:00:56  ptong
# Move the cleaning of NETTARGET from clean: to clobber:
#
# Revision 1.5  2018/08/25 01:01:01  ptong
# Turn on 2099 SUDI support. Bump diag version to 4.3.0
#
# Revision 1.4  2018/07/19 23:32:59  ptong
# Correct a neptune branch->main merge mistake: bring Makefile and version file up to match neptune-branch0-tagV4_2_1-052218 which change back to use 2029-SUDI TAM lib from 2099-SUDI TAM lib
#
# Revision 1.3  2018/05/22 02:31:12  alpeng
# fixed compiler warning, CSCvj57934
#
# Revision 1.2  2018/05/18 09:24:58  alpeng
#  Neptune merge to trunk with tag <neptune-branch-0518>
#
# Revision 1.1.2.27  2018/05/08 03:54:27  alpeng
# update make depend method
#
# Revision 1.1.2.26  2018/05/01 22:16:41  ptong
# Add -DTAM_2099_SUDI compilation flag
#
# Revision 1.1.2.25  2018/04/20 08:45:40  alpeng
# support kalamata on Neptune
#
# Revision 1.1.2.24  2018/02/02 06:47:42  leschen
# Support IR3570 voltage regulator utility.
#
# Revision 1.1.2.23  2017/09/19 10:18:51  alpeng
# support oakenshield; fix oakenshield andf2w uart issue
#
# Revision 1.1.2.22  2017/07/05 23:30:13  ptong
# Move TOOLS_VER from c4.5.3-p1 to c4.7.0-p1. Need to add -Wno-error=unused-but-set-variable flag to aviod massive changes in the diag tree.
#
# Revision 1.1.2.21  2017/07/05 06:31:15  alpeng
# update fan info, update PSU and remove pem files
#
# Revision 1.1.2.20  2017/07/05 03:13:22  alpeng
# redirect show temp func, add d0 setup on i2c scan
#
# Revision 1.1.2.19  2017/05/08 01:32:15  meho
# Removed idt fw file.
#
# Revision 1.1.2.18  2017/04/05 08:27:54  leschen
# Sync with <ng_diag-tag-032917>
#
# Revision 1.1.2.17  2017/02/18 03:37:50  meho
# Added voltage margin utility.
#
# Revision 1.1.2.16  2017/01/04 22:56:07  ptong
# Turn off engineering banner flag for P1B release
#
# Revision 1.1.2.15  2016/12/26 12:50:47  alpeng
# add sys clk into i2c utility
#
# Revision 1.1.2.14  2016/12/06 23:10:54  ptong
# Turn on engineer banner until HW is completer
#
# Revision 1.1.2.13  2016/11/03 08:26:54  alpeng
# merge octeon_test.c with o2
#
# Revision 1.1.2.12  2016/10/14 21:54:52  alpeng
# point to right sdk 3.1.2
#
# Revision 1.1.2.11  2016/09/02 06:42:42  alpeng
# update Makefile to put NEP dir before the O2 dir on vpath
#
# Revision 1.1.2.10  2016/08/30 08:23:39  leschen
# Support PSU and PSU MUX.
#
# Revision 1.1.2.9  2016/08/22 06:54:21  leschen
# Support Neptune LED utility.
#
# Revision 1.1.2.8  2016/08/10 09:05:09  leschen
# Support power sequencer utility
#
# Revision 1.1.2.7  2016/06/21 21:39:07  jskow
# Add SM4 skeleton code, add eUSB/emmc check, add msata test
#
# Revision 1.1.2.6  2016/06/06 09:38:05  leschen
# Support Neptune fan utility.
#
# Revision 1.1.2.5  2016/06/03 09:49:35  leschen
# Support Neptune temperature sensor utilites.
#
# Revision 1.1.2.4  2016/06/01 23:14:17  jskow
# Update Makefile for Neptune, add mb_test structures for PCIe IF test and PCIe register check
#
# Revision 1.1.2.3  2016/05/09 05:07:20  ptong
# Modify BCM_SDK_ROOT to use the new tree for Neptune
#
# Revision 1.1.2.2  2016/05/06 00:32:04  jskow
# Add emmc and eusb tests, update banner to say Neptune, modify Makefiles and move files to boot nepx86_diag on Overlord
#
# Revision 1.1.2.1  2016/04/29 19:40:48  ptong
# Creating neptune directory and initial Makefiles for x86 and cavium
#
# $Endlog$
