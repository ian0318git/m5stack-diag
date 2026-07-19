# $Id: Makefile,v 1.12 2021/05/13 08:49:58 kodko Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tabei-l/Makefile,v $
#------------------------------------------------------------------
#
# Makefile - Makefile to build Platform NET Diagnostic Monitor.
#
#
# Copyright (c) 2019-2021 by Cisco Systems, Inc.
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


TOOLS_VER	= c4.7.0-p3
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

TABEIDIR		= .
KLMDIR		= $(IROOT)/klm

#
# The common source directory
#

DEV_DRV		= $(IROOT)/common/chips
COMSRCDIR	= $(IROOT)/common/src
CHIPINC		= $(DEV_DRV)/include
ENV_SRC     = $(DEV_DRV)/dev_csco_10698
MRV_88E151X_SRC = $(DEV_DRV)/dev_88e151x_marvell
NXP_LM75B_SRC   = $(DEV_DRV)/dev_nxp_lm75b
PCA9557_SRC     = $(DEV_DRV)/dev_pca9557
PCA9555_SRC     = $(DEV_DRV)/dev_pca9555
TMPX75_SRC      = $(DEV_DRV)/dev_tmpx75
TELIT_LTE_SRC   = $(DEV_DRV)/dev_lte_telit
PLUG_COMMON_SRC = $(COMSRCDIR)/plug_common
PLUG_TC_SRC     = $(COMSRCDIR)/plug_testcard
PLUG_LTE_SRC    = $(COMSRCDIR)/plug_lte
PLUG_LTE_TELIT_SRC = $(PLUG_LTE_SRC)/plug_lte_telit
GOLDSCHLAGER_SRC_PATH = $(COMSRCDIR)/goldschlager
DREAMLINER_SRC_PATH  = $(COMSRCDIR)/dreamliner
DREAMLINER_PHY_PATH  = $(COMSRCDIR)/dreamliner/phy
DREAMLINER_CPSS_PATH  = $(COMSRCDIR)/dreamliner/cpss
DREAMLINER_PLD_PATH = /auto/sp-engops/diags/pld/dreamliner
SWITZER_SRC_PATH  = $(COMSRCDIR)/switzer
KALAMATA_SRC_PATH = $(COMSRCDIR)/kalamata
RBCP_LIB_PATH = $(COMSRCDIR)/rbcp_lib
SHEDIR_SRC_PATH = $(COMSRCDIR)/shedir
M2_TC_SRC     = $(COMSRCDIR)/m2_testcard

# Broadcom PHY EPDM lib
BCM_PHY_PATH := /auto/neptune/vendor-src/broadcom/epdm
BCM_MIURA_ROOT := $(BCM_PHY_PATH)/MIUR_1_8
BCM_MIURA_LIB := $(BCM_MIURA_ROOT)/build/libphymodepil-x86_64.a
BCM_QUADRA28_ROOT := $(BCM_PHY_PATH)/QUADRA28_1_2
BCM_QUADRA28_LIB := $(BCM_QUADRA28_ROOT)/build/libphymodepil-x86_64.a
BCM_EPDM_ROOT := $(BCM_PHY_PATH)/epdm_1_7_4
BCM_EPDM_LIB := $(BCM_EPDM_ROOT)/libepdm-x86_64.a
BCM_EPDM_INC := $(BCM_EPDM_ROOT)
BCM_PHY_LIBS := $(BCM_EPDM_LIB) $(BCM_QUADRA28_LIB) $(BCM_MIURA_LIB)


# TAM lib. request : ENGTAM-1223
TAMLIB  = /auto/sp-engops/diags/pld/act2lite/x86/isr9k_curie1ru/aikido

#
# Include paths for compiler
#

INC	= \
	-I$(KLMDIR) \
	-I$(INCDIR) \
	-I$(TABEIDIR) \
	-I$(DEV_DRV) \
	-I$(CHIPINC) \
	-I$(TAMLIB) \
	-I$(COMSRCDIR) \
	-I$(NXP_LM75B_SRC) \
	-I$(PCA9557_SRC) \
	-I$(PCA9555_SRC) \
	-I$(TMPX75_SRC) \
	-I$(MRV_88E151X_SRC) \
	-I$(PLUG_COMMON_SRC) \
	-I$(PLUG_TC_SRC) \
	-I$(PLUG_LTE_SRC) \
	-I$(PLUG_LTE_TELIT_SRC) \
	-I$(TELIT_LTE_SRC) \
	-I$(DEV_DRV)/dev_ltc4215 \
	-I$(ENV_SRC) \
	-I$(KALAMATA_SRC_PATH) \
	-I$(RBCP_LIB_PATH) \
	-I$(SHEDIR_SRC_PATH) \
	-I$(M2_TC_SRC)

INC += \
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

INC     += -I$(BCM_EPDM_INC)

#
# defines for compiler
#

DEFS = \
	-DTABEIL \
	-DLINUX_APP \
	-DTFTP_CLIENT  \
	-DSUDI_2099_2029 \
	-DENABLE_SUDI_2099_CLI \
	-DAIKIDO_ACT2 \
	-DAIKIDO_SUPPORT_AIK \
	-DAIKIDO_DEV_KEY \
	-DPLAT_HAS_PLUG \
	-DAIKIDO_CT_DEV_KEY \

DREAMLINER_DEFS = \
        -DDL_LINUX -DCPU_LE -DGT_PCI -DINCL_EXDXMX_DRIVER -DINTEL64_CPU -DIS_64BIT_OS

# Step1 :If TAM lib. version newer than V3.1.1, add this define "-DSUDI_2099_2029" to support new TAM lib. function.
# Step2 :When IOS supports SUDI_2099, user can add this define "-DENABLE_SUDI_2099_CLI" to enable "Toggle SUDI 2099 Flag" and program SUDI_2099.

#
# un-comment the line below to compile an image for 2-NM and 0-HWIC
# need to 'make clean all' to build this image
#

CPPOPTS     = -Wall -Werror -Wno-unused-but-set-variable $(INC) ${DEFS} $(DREAMLINER_DEFS)
LDFLAGS     = -lpthread
CCOPTS		= -g -fsigned-char -pipe  -Wa, 
ASOPTS		= -fsigned-char -float 
CFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${CCOPTS}
CFLAGS		+= -DBOARD_P0
#Add "D_DEFAULT_SOURCE" to fix warning _BSD_SOURCE and _SVID_SOURCE are deprecated
CFLAGS      += -D_DEFAULT_SOURCE
ASFLAGS		= -O0 ${OPTIONS} ${CPPOPTS} ${ASOPTS}


#
# Build ACT2 lib into TABEI project
#
#
# SUDI_2099
LIB_TAM_ACT2 = $(TAMLIB)/tam_library_w_standalone.a
LIB_TAM_ACT2_MFG = $(TAMLIB)/tam_lib_manufacturing_diags.a
LIB_TAM_OBJNAME = $(TAMLIB)/tam_lib_objname.a


LIB_PCI_UTILS=/auto/sword/Utils/pciutils/3.2.0/libpci_gcc.c4.5.3-p1.x86_64.a

#
# TARGET is the stripped ELF binary file, ELFFILE is unstripped
#
#
NETTARGET	= tabeil_prome_lnx

#
# Common device driver objects
#
DEV_OBJS = \
		dev_object.o \
		dev_nxp_lm75b.o \
		dev_ltc4215.o \
		dev_pca9557.o \
		dev_pca9555.o \
		dev_tmpx75.o \
		dev_88e151x.o \
		dev_csco_10698.o \
        dev_lte_telit.o \
    	dev_lte_telit_at.o \


DEV_BASE = \
		$(DEV_DRV)/dev_object.c \
		$(DEV_DRV)/dev_ltc4215/dev_ltc4215.c \
		$(MRV_88E151X_SRC)/dev_88e151x.c \
		$(PCA9557_SRC)/dev_pca9557.c \
		$(PCA9555_SRC)/dev_pca9555.c \
		$(NXP_LM75B_SRC)/dev_nxp_lm75b.c \
		$(TMPX75_SRC)/dev_tmpx75.c \
		$(ENV_SRC)/dev_csco_10698.c \
	    $(TELIT_LTE_SRC)/dev_lte_telit.c \
	    $(TELIT_LTE_SRC)/dev_lte_telit_at.c\


DEV_HFILES = \
$(DEV_DRV)/dev_object.h \
        $(DEV_DRV)/dev_ltc4215/dev_ltc4215.h \
    	$(PCA9557_SRC)/dev_pca9557.h \
    	$(PCA9555_SRC)/dev_pca9555.h \
    	$(NXP_LM75B_SRC)/dev_nxp_lm75b.h \
    	$(TMPX75_SRC)/dev_tmpx75.h \
    	$(MRV_88E151X_SRC)/dev_88e151x.h \
    	$(ENV_SRC)/dev_csco_10698.h \
    	$(TELIT_LTE_SRC)/dev_lte_telit.h \
    	$(TELIT_LTE_SRC)/dev_lte_telit_at.h\


FORTITUDE_OBJS = ngwic_fortitude.o
GRIMLOCK_OBJS = nim_grimlock.o
FORTITUDE_BASE = $(COMSRCDIR)/ngwic_fortitude.c
GRIMLOCK_BASE = $(COMSRCDIR)/nim_grimlock.c 

REVA_OBJS = ngwic_reva.o
REVA_BASE = $(COMSRCDIR)/ngwic_reva.c

ARKENSTONE_OBJS = nim_arkenstone.o
ARKENSTONE_BASE = $(COMSRCDIR)/nim_arkenstone.c


WALLANDER_OBJS = nim_wallander.o
WALLANDER_BASE = $(COMSRCDIR)/nim_wallander.c


GOLDSCHLAGER_OBJS = \
        ngwic_goldschlager.o \
        ngwic_goldschlager_comm.o \
        bcm63268_lib.o

GOLDSCHLAGER_BASE = \
        $(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager.c \
        $(GOLDSCHLAGER_SRC_PATH)/ngwic_goldschlager_comm.c \
        $(GOLDSCHLAGER_SRC_PATH)/bcm63268_lib.c


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

#
## Dynamo NIM
#
DYNAMO_OBJS = nim_dynamo.o
DYNAMO_BASE = $(COMSRCDIR)/nim_dynamo.c

GRAFFHAM_OBJS = ngvm_graffham.o
GRAFFHAM_BASE = $(COMSRCDIR)/ngvm_graffham.c

SWITZER_OBJS = \
        switzer_common.o \
        switzer_miura.o \
        switzer_fpga.o \
        switzer_10g.o \
        switzer_test.o \
        switzer_traf.o \
        switzer_10g_test.o \
        switzer_manhattan.o \
        switzer_tps23881_sram_parity.o \
        switzer_manhattan_upoe.o \
        switzer_manhattan_test.o \
        switzer_manhattan_bcm54194_api.o \
        switzer_manhattan_bcm54194_test.o

SWITZER_BASE = \
        $(SWITZER_SRC_PATH)/switzer_common.c \
        $(SWITZER_SRC_PATH)/switzer_miura.c \
        $(SWITZER_SRC_PATH)/switzer_fpga.c \
        $(SWITZER_SRC_PATH)/switzer_10g.c \
        $(SWITZER_SRC_PATH)/switzer_test.c \
        $(SWITZER_SRC_PATH)/switzer_traf.c \
        $(SWITZER_SRC_PATH)/switzer_10g_test.c \
        $(SWITZER_SRC_PATH)/switzer_manhattan.c \
        $(SWITZER_SRC_PATH)/switzer_tps23881_sram_parity.c \
        $(SWITZER_SRC_PATH)/switzer_manhattan_upoe.c \
        $(SWITZER_SRC_PATH)/switzer_manhattan_test.c \
        $(SWITZER_SRC_PATH)/switzer_manhattan_bcm54194_api.c \
        $(SWITZER_SRC_PATH)/switzer_manhattan_bcm54194_test.c

KALAMATA_OBJS = \
        nim_kalamata.o \
        kalamata_rbcp_main.o \

KALAMATA_BASE = \
        $(KALAMATA_SRC_PATH)/nim_kalamata.c \
        $(KALAMATA_SRC_PATH)/kalamata_rbcp_main.c \

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

F35_OBJS = nim_f35.o

F35_BASE = $(COMSRCDIR)/nim_f35.c


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
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.c \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.c \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.c \
    $(PLUG_COMMON_SRC)/plug_common_host.c \
    $(PLUG_COMMON_SRC)/plug_common_lib.c \
    $(TABEIDIR)/plug_common_host_impl.c \
    $(TABEIDIR)/plug_host_slot_modules.c \

PLUG_COMMON_HFILES = \
    $(PLUG_COMMON_SRC)/plug_slot.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_test.h \
    $(PLUG_COMMON_SRC)/plug_gpio_exp_lib.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_test.h \
    $(PLUG_COMMON_SRC)/plug_temp_sensor_lib.h \
    $(PLUG_COMMON_SRC)/plug_common_host.h \
    $(PLUG_COMMON_SRC)/plug_common_lib.h \
    $(TABEIDIR)/plug_common_host_impl.h \
    $(TABEIDIR)/plug_host_slot_modules.h \

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
    $(TABEIDIR)/plug_testcard_host_impl.c \

PLUG_TC_HFILES = \
    $(PLUG_TC_SRC)/plug_testcard_gpio_exp_lib.h \
    $(PLUG_TC_SRC)/plug_testcard_test.h \
    $(PLUG_TC_SRC)/plug_testcard_util.h \
    $(PLUG_TC_SRC)/plug_testcard_phy.h \
    $(PLUG_TC_SRC)/plug_testcard_host.h \
    $(PLUG_TC_SRC)/plug_testcard_usb_lib.h \
    $(TABEIDIR)/plug_testcard_host_impl.h \

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
    $(TABEIDIR)/plug_lte_host_impl.c \

PLUG_LTE_HFILES = \
    $(PLUG_LTE_SRC)/plug_lte_test.h \
    $(PLUG_LTE_SRC)/plug_lte_util.h \
    $(PLUG_LTE_SRC)/plug_lte_lib.h \
    $(PLUG_LTE_SRC)/plug_lte_at.h \
    $(PLUG_LTE_SRC)/plug_lte_host.h \
    $(TABEIDIR)/plug_lte_host_impl.h \

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


PLUG_OBJS = \
	$(PLUG_COMMON_OBJS) \
	$(PLUG_TC_OBJS) \
	$(PLUG_LTE_OBJS) \
	$(PLUG_LTE_TELIT_OBJS) \

PLUG_BASE = \
	$(PLUG_COMMON_BASE) \
	$(PLUG_TC_BASE) \
	$(PLUG_LTE_BASE) \
	$(PLUG_LTE_TELIT_BASE) \


PLUG_HFILES = \
	$(PLUG_COMMON_HFILES) \
	$(PLUG_TC_HFILES) \
	$(PLUG_LTE_HFILES) \
	$(PLUG_LTE_TELIT_HFILES) \

#
# M.2 Testcard
#
M2_TC_OBJS = \
    m2_testcard.o \

M2_TC_BASE = \
    $(M2_TC_SRC)/m2_testcard.c \

M2_TC_HFILES = \
    $(M2_TC_SRC)/m2_testcard.h \


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
    linux_pci.o \
    linux_memory_tester.o \
    linux_memory_tester_utils.o \
    linux_uart.o \
    linux_pciutils.o \
    linux_usb_test.o \
    linux_block_test.o \
    menu.o \
    common_utils.o \
    print_hex.o \
    query.o \
    linux_pcie.o \
    memops.o \
    adapter_fpga.o \
    testmem.o \
    bit_swaps.o \
    tftpclient.o \
    cookie_4_core.o \
	tam_act2_utils.o \
	linux_ntwk.o \
	pca.o \
	linux_coretest.o \
	oir_ltc4215_api.o \
	rtc.o

COM_BASE = \
    $(COMSRCDIR)/linux_i2c.c \
    $(COMSRCDIR)/linux_error.c \
    $(COMSRCDIR)/linux_stub.c \
    $(COMSRCDIR)/linux_mem.c \
    $(COMSRCDIR)/linux_api.c \
    $(COMSRCDIR)/linux_pci.c \
    $(COMSRCDIR)/linux_memory_tester.c \
    $(COMSRCDIR)/linux_memory_tester_utils.c \
    $(COMSRCDIR)/linux_uart.c \
    $(COMSRCDIR)/linux_pciutils.c \
    $(COMSRCDIR)/linux_usb_test.c \
    $(COMSRCDIR)/linux_block_test.c \
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
    $(COMSRCDIR)/linux_ntwk.c \
    $(COMSRCDIR)/linux_popen.c \
    $(COMSRCDIR)/linux_coretest.c \
    $(COMSRCDIR)/slot.c \
    $(COMSRCDIR)/adapter_fpga.c \
    $(COMSRCDIR)/tam_aikido_upgrade.c \
    $(COMSRCDIR)/rtc.c

COM_HFILES = \
	$(INCDIR)/menu.h \
	$(INCDIR)/setjmps.h \
	$(INCDIR)/nvsysvars.h \
	$(INCDIR)/monitor.h \
	$(INCDIR)/byteswap.h \
	$(INCDIR)/linux_api.h \
	$(INCDIR)/linux_memory_tester.h \
	$(INCDIR)/linux_memory_tester_utils.h \
	$(INCDIR)/linux_pciutils.h \
	$(INCDIR)/common_utils.h \
	$(INCDIR)/linux_usb_test.h \
	$(INCDIR)/linux_block_test.h \
	$(INCDIR)/endians.h \
	$(INCDIR)/i2c_api.h \
	$(INCDIR)/i2c_dev.h \
	$(INCDIR)/n2g_api_rc.h \
	$(INCDIR)/cross_platform.h \
	$(INCDIR)/cli_cmd.h \
	$(INCDIR)/adapter_fpga.h \
	$(INCDIR)/linux_ntwk.h \
	$(INCDIR)/linux_pci.h \
     $(INCDIR)/list.h \
	$(TAMLIB)/tam_lib_manufacturing.h \
	$(TAMLIB)/tam_library.h \
       





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
	linux_popen.o \
	diag_boot_flash_test.o \
	diag_spi_flash_util.o \
	diag_fpga_test.o \
	diag_gephy_test.o \
	diag_gephy_lib.o \
	diag_gephy_util.o \
	diag_fpga_lib.o \
	diag_cpld_lib.o \
	diag_led_test.o \
	diag_led_util.o \
    diag_eth_pkt_txrx.o \
    diag_eth_pkt_txrx_api.o \
    diag_eth_pkt_txrx_utils.o \
	diag_ddr4_lib.o \
	diag_i2c_lib.o \
	diag_i2c_test.o \
	i2c_api.o \
	diag_fpga_i2c_lib.o \
	diag_usb_lib.o \
	diag_usb_util.o \
	diag_usb_test.o \
	diag_temp_snsr_test.o \
	diag_cpu_util.o \
	platform_stub.o \
	platform_cookie.o \
	platform_fan.o \
	legacy_smart_cookie.o \
	tam_act2_api_drv_support.o \
	tam_aikido_upgrade.o \
	diag_emmc_test.o \
	diag_emmc_util.o \
	dnv_gpio_lib.o \
	diag_uart_lib.o \
	dnv_eth_lib.o \
	diag_fpga_upgrade.o \
	diag_rtc_lib.o \
	diag_rtc_test.o \
	diag_rtc_util.o \
	platform_cpu.o \
	tabei_comm.o \
	full_load_util.o \
	diag_i350_test.o \
	ngio.o \
	slot.o \
	platform_slot.o \
	platform_barometer.o \
	$(PLUG_OBJS) \
	diag_hdd_test.o \
	diag_m2_test.o \
	diag_tpm_test.o \
	platform_pwr_seq.o \
	platform_spi.o \
	fpga_smartfan.o \
	m2_testcard_host_impl.o \
	$(M2_TC_OBJS)



PLAT_BASE = \
	$(TABEIDIR)/linux_main.c \
	$(TABEIDIR)/diag.c \
	$(TABEIDIR)/mb_tests.c \
	$(TABEIDIR)/diag_boot_flash_test.c \
	$(TABEIDIR)/diag_spi_flash_util.c \
	$(TABEIDIR)/diag_fpga_test.c \
	$(TABEIDIR)/diag_gephy_test.c \
	$(TABEIDIR)/diag_gephy_lib.c \
	$(TABEIDIR)/diag_gephy_util.c \
	$(TABEIDIR)/diag_fpga_lib.c \
	$(TABEIDIR)/diag_cpld_lib.c \
	$(TABEIDIR)/diag_led_test.c \
	$(TABEIDIR)/diag_led_util.c \
	$(TABEIDIR)/diag_eth_pkt_txrx.h \
	$(TABEIDIR)/diag_eth_pkt_txrx_api.h \
	$(TABEIDIR)/diag_eth_pkt_txrx_utils.h \
	$(TABEIDIR)/diag_ddr4_lib.c \
	$(TABEIDIR)/diag_i2c_lib.c \
	$(TABEIDIR)/diag_i2c_test.c \
	$(TABEIDIR)/i2c_api.c \
	$(TABEIDIR)/diag_fpga_i2c_lib.c \
	$(TABEIDIR)/diag_fpga_upgrade.c \
	$(TABEIDIR)/diag_usb_lib.c \
	$(TABEIDIR)/diag_usb_util.c \
	$(TABEIDIR)/diag_usb_test.c \
	$(TABEIDIR)/diag_temp_snsr_test.c \
	$(TABEIDIR)/diag_cpu_util.c \
	$(TABEIDIR)/platform_stub.c \
	$(TABEIDIR)/platform_cookie.c \
	$(TABEIDIR)/platform_fan.c \
	$(TABEIDIR)/legacy_smart_cookie.c \
	$(TABEIDIR)/tam_act2_api_drv_support.c \
	$(TABEIDIR)/diag_emmc_test.c \
	$(TABEIDIR)/diag_emmc_util.c \
	$(TABEIDIR)/dnv_gpio_lib.c \
	$(TABEIDIR)/diag_uart_lib.c \
	$(TABEIDIR)/dnv_eth_lib.c \
	$(TABEIDIR)/diag_rtc_lib.c \
	$(TABEIDIR)/diag_rtc_test.c \
	$(TABEIDIR)/diag_rtc_util.c \
	$(TABEIDIR)/platform_cpu.c \
	$(TABEIDIR)/tabei_comm.c \
	$(TABEIDIR)/full_load_util.c \
	$(TABEIDIR)/diag_i350_test.c \
	$(TABEIDIR)/ngio.c \
	$(TABEIDIR)/platform_slot.c \
	$(TABEIDIR)/platform_barometer.c \
	$(PLUG_BASE) \
	$(TABEIDIR)/diag_hdd_test.c \
	$(TABEIDIR)/diag_m2_test.c \
	$(TABEIDIR)/diag_tpm_test.c \
	$(TABEIDIR)/platform_pwr_seq.c \
	$(TABEIDIR)/platform_spi.c \
	$(TABEIDIR)/fpga_smartfan.c \
	$(TABEIDIR)/m2_testcard_host_impl.c \
	$(M2_TC_BASE)



PLAT_HFILES = \
    $(TABEIDIR)/diag_common.h \
    $(TABEIDIR)/platform_i2c.h \
    $(TABEIDIR)/mb_tests.h \
    $(TABEIDIR)/diag_eth_info.h \
    $(TABEIDIR)/diag_eth_pkt_txrx.h \
    $(TABEIDIR)/diag_eth_pkt_txrx_api.h \
    $(TABEIDIR)/diag_eth_pkt_txrx_utils.h \
    $(TABEIDIR)/diag_fpga.h \
    $(TABEIDIR)/diag_ddr4_lib.h \
    $(TABEIDIR)/diag_i2c_lib.h \
    $(TABEIDIR)/diag_i2c_test.h \
    $(TABEIDIR)/diag_i2c_addr.h \
    $(TABEIDIR)/diag_boot_flash_test.h \
    $(TABEIDIR)/diag_spi_flash_util.h \
    $(TABEIDIR)/diag_fpga_i2c_lib.h \
    $(TABEIDIR)/diag_storage_lib.h \
    $(TABEIDIR)/diag_usb_lib.h \
    $(TABEIDIR)/diag_usb_util.h \
    $(TABEIDIR)/diag_usb_test.h \
    $(TABEIDIR)/diag_temp_snsr_test.h \
    $(TABEIDIR)/diag_gephy_test.h \
    $(TABEIDIR)/diag_gephy_lib.h \
    $(TABEIDIR)/diag_gephy_util.h \
    $(TABEIDIR)/diag_cpu_util.h \
    $(TABEIDIR)/platform_stub.h \
    $(TABEIDIR)/platform_cookie.h \
    $(TABEIDIR)/legacy_smart_cookie.h \
    $(TABEIDIR)/tam_act2_api_drv_support.h \
	$(TABEIDIR)/diag_emmc_test.h \
	$(TABEIDIR)/diag_emmc_util.h \
    $(TABEIDIR)/dnv_gpio_lib.h \
    $(TABEIDIR)/diag_uart_lib.h \
    $(TABEIDIR)/dnv_eth_lib.h \
    $(TABEIDIR)/diag_fpga_upgrade.h \
    $(TABEIDIR)/diag_rtc_lib.h \
    $(TABEIDIR)/diag_rtc_test.h \
    $(TABEIDIR)/diag_rtc_util.h \
    $(TABEIDIR)/platform_cpu.h \
    $(TABEIDIR)/tabei_comm.h \
    $(TABEIDIR)/full_load_util.h \
	$(TABEIDIR)/diag_i350_test.h \
	$(TABEIDIR)/ngio.h \
	$(TABEIDIR)/platform_slot.h \
	$(TABEIDIR)/platform_barometer.h \
	$(TABEIDIR)/diag_hdd_test.h \
	$(TABEIDIR)/diag_m2_test.h \
	$(TABEIDIR)/diag_tpm_test.h \
	$(TABEIDIR)/platform_pwr_seq.h \
	$(TABEIDIR)/fpga_smartfan.h \
	$(TABEIDIR)/platform_margin_utils.h \
	$(TABEIDIR)/bcm_gesw_defs.h \
	$(TABEIDIR)/m2_testcard_host_impl.h \
	$(M2_TC_HFILES) \



#
# Test Card related files
#
TESTCARD_OBJS = \
        ngio_testcard.o \
        testcard_fpga.o \
        testcard_xaui.o \
        testcard_plx_pcie_sw.o \
        testcard_tlk_10232.o \
        testcard_uart.o

TESTCARD_BASE = \
        $(TABEIDIR)/ngio_testcard.c \
        $(TABEIDIR)/testcard_fpga.c \
        $(TABEIDIR)/testcard_xaui.c \
        $(TABEIDIR)/testcard_plx_pcie_sw.c \
        $(TABEIDIR)/testcard_tlk_10232.c \
        $(TABEIDIR)/testcard_uart.c

TESTCARD_HFILES = \
        $(TABEIDIR)/ngio_testcard.h \
        $(TABEIDIR)/testcard_fpga.h \
        $(TABEIDIR)/testcard_plx_pcie_sw.h \
        $(TABEIDIR)/testcard_tlk_10232.h \
        $(TABEIDIR)/testcard_xaui.h 



#
# RAM Based Object files : PLAT_OBJS must be first.
# This will ensure that _start is first code in .text section.
#


NET_OBJS = \
	$(PLAT_OBJS) \
	$(COM_OBJS) \
	$(DEV_OBJS) \
	$(LIB_PCI_UTILS) \
	$(LIB_TAM_ACT2_MFG) \
	$(LIB_TAM_ACT2) \
	$(LIB_TAM_OBJNAME) \
	$(FORTITUDE_OBJS) \
	$(GRIMLOCK_OBJS) \
    $(TESTCARD_OBJS) \
	$(GOLDSCHLAGER_OBJS) \
	$(REVA_OBJS) \
    $(DREAMLINER_OBJS) \
    $(DREAMLINER_LIB) \
	$(ARKENSTONE_OBJS) \
	$(WALLANDER_OBJS) \
	$(PRINCE_OBJS) \
	$(DYNAMO_OBJS) \
	$(GRAFFHAM_OBJS) \
	$(SWITZER_OBJS) \
	$(KALAMATA_OBJS) \
	$(SHEDIR_OBJS) \
	$(F35_OBJS) 

#
#########   CBASE #########
#

CBASE	= \
	$(COM_BASE) \
	$(DEV_BASE) \
	$(PLAT_BASE) \
    $(FORTITUDE_BASE) \
    $(GRIMLOCK_BASE) \
    $(TESTCARD_BASE) \
    $(GOLDSCHLAGER_BASE) \
    $(REVA_BASE) \
    $(DREAMLINER_BASE) \
    $(ARKENSTONE_BASE) \
	$(WALLANDER_BASE) \
	$(PRINCE_BASE) \
	$(DYNAMO_BASE) \
	$(GRAFFHAM_BASE) \
	$(SWITZER_BASE) \
	$(KALAMATA_BASE) \
	$(SHEDIR_BASE) \
	$(F35_BASE)


#
#########   Header Files #########
#

HFILES  = \
	$(COM_HFILES) \
	$(DEV_HFILES) \
	$(PLAT_HFILES) \
    $(TESTCARD_HFILES)


vpath %.c \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(TABEIDIR): \
	$(COMSRCDIR): \
	$(DEV_DRV): \
	$(ENV_SRC): \
	$(RTC_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(NXP_LM75B_SRC): \
	$(MRV_88E151X_SRC): \
	$(LTE_SWI_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
	$(TAMLIB): \
    $(DREAMLINER_SRC_PATH): \
    $(DREAMLINER_PLD_PATH): \
    $(DREAMLINER_PHY_PATH): \
    $(DREAMLINER_CPSS_PATH)/src: \
	$(GOLDSCHLAGER_SRC_PATH): \
    $(SWITZER_SRC_PATH): \
	$(PLUG_LTE_TELIT_SRC): \
	$(TELIT_LTE_SRC): \
	$(KALAMATA_SRC_PATH): \
	$(RBCP_LIB_PATH): \
	$(SHEDIR_SRC_PATH): \
	$(M2_TC_SRC): \

vpath %.h \
	$(GE_PHY_SRC): \
	$(PCA9557_SRC): \
	$(PCA9555_SRC): \
	$(TMPX75_SRC): \
	$(TABEIDIR): \
	$(INCDIR): \
	$(DEV_DRV): \
	$(ENV_SRC): \
	$(RTC_SRC): \
	$(NXP_LM75B_SRC): \
	$(DEV_DRV)/dev_ltc4215: \
	$(MRV_88E151X_SRC): \
	$(LTE_SWI_SRC): \
	$(PLUG_COMMON_SRC): \
	$(PLUG_TC_SRC): \
	$(PLUG_LTE_SRC): \
    $(DREAMLINER_SRC_PATH): \
    $(DREAMLINER_PHY_PATH): \
	$(TAMLIB): \
	$(PLUG_LTE_TELIT_SRC): \
	$(TELIT_LTE_SRC): \
	$(RBCP_LIB_PATH): \
	$(SHEDIR_SRC_PATH): \
	$(M2_TC_SRC): \

.SUFFIXES:
.SUFFIXES: .o .lis .S .c .s .dis

%.o : %.S
	$(CC) -c $(ASFLAGS) -Wp,-MD,.$@.d $<
%.o : %.c
	$(CC) -c $(CFLAGS) -Wp,-MD,.$@.d $<
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
	@echo "TABEI_DIAG_COMPILE: gcc." $(TOOLS_VER).$(TOOLS_ARCH)
	@echo "IROOT: " $(IROOT)
	@echo "TABEIDIR: " $(TABEIDIR)
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
	sh $(IROOT)/utils/banner.sh TABEI_L_INTEL
	$(CC) $(CFLAGS) -c banner.c
	$(CC) $(CFLAGS) $(TARGET_OBJS) banner.o -o $(NETTARGET) ${LDFLAGS} \
        -Wl,-whole-archive $(BCM_PHY_LIBS) -Wl,-no-whole-archive 

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
# Revision 1.12  2021/05/13 08:49:58  kodko
# Support M.2 testcard.
#
# Revision 1.11  2021/04/12 13:38:05  xiaolaya
# *** empty log message ***
#
# Revision 1.10  2020/08/25 07:16:48  kehuang2
# CSCvv45776: Change image name to support Tabei-L and Promethium
#
# Revision 1.9  2020/08/21 03:08:30  kehuang2
# CSCvv45776: Change image name to support Tabei-L and Promethium
#
# Revision 1.8  2020/08/17 07:20:07  kehuang2
# Define for aikido consent token
#
# Revision 1.7  2020/08/06 07:54:55  kehuang2
# Collapse Promethium into main trunk
#
# Revision 1.6  2020/05/25 09:37:13  qingcwan
# CSCvu31200:Fix bug introduced by prev commit to merge in switzer-carrier code.
#
# Revision 1.5  2020/03/13 12:19:46  letsai
# Support Grimlock NIM
#
# Revision 1.4  2019/12/30 05:59:18  kehuang2
# CSCvs55860: Support Gaffham
#
# Revision 1.3  2019/11/25 08:55:51  kehuang2
# Collapse Tabei-L into main trunk
#
# Revision 1.2  2019/10/17 02:16:19  kehuang2
# Collapse Tabei-L into main trunk
#
# Revision 1.1.4.41  2019/10/01 02:53:05  kehuang2
# Clean up code
#
# Revision 1.1.4.40  2019/09/11 09:51:06  kehuang2
# Update after sync Main trunk
#
# Revision 1.1.4.39  2019/09/02 08:38:02  olin2
# support display smart fan info util
#
# Revision 1.1.4.38  2019/07/31 08:00:48  olin2
# Clean up code
#
# Revision 1.1.4.37  2019/07/31 03:45:08  kehuang2
# Merge platform_tam_cookie into platform_cookie
#
# Revision 1.1.4.36  2019/07/30 07:31:17  olin2
# Support Dynamo on Tabei-L
#
# Revision 1.1.4.35  2019/07/15 11:28:47  kehuang2
# Support Barometer test and utility
#
# Revision 1.1.4.34  2019/07/12 09:33:11  sherliu2
# Supported Hyperloop-PIM
#
# Revision 1.1.4.33  2019/07/02 01:25:31  olin2
# Support Switzer on Tabei-L
#
# Revision 1.1.4.32  2019/06/20 06:21:13  kehuang2
#
# 1. Support linux_block_test function
# 2. Update Diag menu item base on currently project information
#
# Revision 1.1.4.31  2019/05/22 07:52:43  olin2
# Support Prince on Tabei-L
#
# Revision 1.1.4.30  2019/04/18 09:18:13  olin2
# support program Aikido dev key
#
# Revision 1.1.4.29  2019/03/26 06:09:16  olin2
# Support Dreamliner on Tabei-L
#
# Revision 1.1.4.28  2019/03/07 06:38:09  olin2
# Support Arkentone on Tabei-L
#
# Revision 1.1.4.27  2019/02/22 08:20:06  harrchan
# Support 88E1543 utility
#
# Revision 1.1.4.26  2019/02/22 06:43:03  olin2
# Add Reva on Tabei-L
#
# Revision 1.1.4.25  2019/02/15 06:14:28  olin2
# Add Goldschlager on Tabei-L
#
# Revision 1.1.4.24  2019/01/25 03:21:06  wilbhuan
# 1. Added ESW(Ethernet Switch) test with 88E6390 PHY device.
# 2. The scope of ESW test as following:
#    (1) Register test
#    (2) MAC loopback test
#    (3) External loopback test
#    (4) Interrupt test
#
# Revision 1.1.4.23  2019/01/18 02:30:15  olin2
# Clean up code
#
# Revision 1.1.4.22  2019/01/16 04:03:45  harrchan
# Init phy1543 test
#
# Revision 1.1.4.21  2018/12/25 02:06:29  kodko
# Add TPM chip test that is verified by vendor provided tool.
#
# Revision 1.1.4.20  2018/12/21 07:09:47  olin2
# Update M.2 device menu
#
# Revision 1.1.4.19  2018/12/05 06:50:34  olin2
# initial commit for Aikido
#
# Revision 1.1.4.18  2018/11/16 05:42:09  olin2
# Clean up code
#
# Revision 1.1.4.17  2018/11/15 06:56:06  olin2
# initial commit for Fan utils
#
# Revision 1.1.4.16  2018/11/02 02:39:03  kodko
# Support cookie read for NIM and PIM modules.
#
# Revision 1.1.4.15  2018/10/26 08:40:49  kodko
# Add support for PIM LTE and test card modules.
#
# Revision 1.1.4.14  2018/10/25 09:55:23  harrchan
# Add MCU utility in I2C utility
#
# Revision 1.1.4.13  2018/10/25 02:37:57  harrchan
# eMMC Test
#
# Revision 1.1.4.12  2018/10/23 11:34:26  olin2
# Support Testcard test
#
# Revision 1.1.4.11  2018/10/22 11:29:32  harrchan
# Temperature sensor
#
# Revision 1.1.4.10  2018/10/18 08:32:33  olin2
# Support NIM testcard FPGA reg test
#
# Revision 1.1.4.9  2018/10/18 06:02:43  olin2
# Clean up unused function
#
# Revision 1.1.4.8  2018/10/16 07:58:11  harrchan
# CPU multi core test
#
# Revision 1.1.4.7  2018/10/15 12:30:12  kodko
# Add CPLD register read/write function.
#
# Revision 1.1.4.6  2018/10/15 11:48:28  olin2
# Update for using common slot.c
#
# Revision 1.1.4.5  2018/10/15 11:40:51  harrchan
# Add diag sata test
#
# Revision 1.1.4.4  2018/10/09 09:22:04  olin2
# Initial commit for NIM test
#
# Revision 1.1.4.3  2018/10/03 06:06:38  olin2
# Initial commit for I350 test
#
# Revision 1.1.4.2  2018/10/02 01:49:57  harrchan
# Initial commit for Tabei-L P1A bring up.
#
# $Endlog$

