#$Id: Makefile,v 1.2 2014/06/03 10:53:28 erwu2 Exp $
#$Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/lebowski_python/host/Makefile,v $
#------------------------------------------------------------------------------
#Makefile - Makefile to build c executable in lebowski_python for python menu
#
#April 2014 - erwu2
#
#Copyright (c) 2013-2014 by Cisco Systems, Inc.
#
#All rights reserved.
#
#------------------------------------------------------------------------------
default: all

#
# Tool Chain
#
CC = gcc
PY = python2.7

#
# platform folder
#
PLAT_TARGET = o2_python_example

#
# The SRG environment common source directory
#
IROOT       = ../../../..
INC_DIR     = ../../../include
# tar cmd needs related path which started from current folder
SRC_DIR     = ../..

#
# The SRG platform specific source directory
#
O2_SRC_DIR = $(SRC_DIR)/$(PLAT_TARGET)
O2_X86_DIR = $(SRC_DIR)/overlord/x86
O2_UTAH_DIR = $(SRC_DIR)/overlord/utah

ifeq ($(PLAT_TYPE),UTAH)
    PLAT_DIR = $(O2_UTAH_DIR)
else ifeq ($(PLAT_TYPE),OVLD)
    PLAT_DIR = $(O2_X86_DIR)
else
    PLAT_DIR = $(PLAT_TYPE)
endif


#
# The python-specific source directory
#
PY_BIN         = $(IROOT)/../py_interpreter_v2_7_3/py_interpreter
PY_DIR         = $(PY_BIN)/include/$(PY)
PY_SRC_DIR     = $(SRC_DIR)/Python
PY_INC_DIR     = $(PY_SRC_DIR)/include
PY_SCRIPT_DIR  = $(PY_SRC_DIR)/script
PY_COM_SRC_DIR = $(PY_SRC_DIR)/src
PY_CFG_DIR     = $(PY_SRC_DIR)/config

#
# Include paths for compiler
#
INC	= \
	-I$(PY_DIR) \
	-I$(PY_INC_DIR) \
	-I$(PY_SRC_DIR) \
	-I$(INC_DIR) \
	-I$(O2_SRC_DIR) \
	-I$(PLAT_DIR)

#
# GCC FLAGS
#
LDFLAGS            = -L$(PY_BIN)/lib -l$(PY) -lutil -ldl -lnsl -lpthread -lm \
-lrt -Wl,--relax

#
# python-specific source files
#
PY_BASE = \
	$(PY_COM_SRC_DIR)/python_error.c \
	$(PY_COM_SRC_DIR)/python_util.c

PY_DIAG_PCFG = \
	$(PY_CFG_DIR)/diag_flag.pcfg

PY_DIAG_HFILE = \
	$(PY_INC_DIR)/diag_flag_create_from_py.h

#
# SRG environment Source files
#
PLAT_BASE = \
	$(O2_SRC_DIR)/o2_platform_fru.c

NET_BASE = \
	$(PY_BASE) \
	$(PLAT_BASE)


############## modify the lines below for other module's host side test #######
#
# The SRG platform specific source directory
#
LEB_SRC_DIR  = ./

PLAT_BASE += \
	$(O2_SRC_DIR)/o2_util.c

#
# Include paths for compiler
#
INC	+= \
	-I$(LEB_SRC_DIR)

#
# lebowski SM module host side source code example
#
LEB_OIR_LED = \
	leb_oir_led_test.c

LEB_OIR_REG = \
	leb_oir_LTC4215_reg_test.c

LEB_OIR_INIT = \
	leb_oir_init.c

#
# lebowski SM module host side obj file example
#
OBJ_LEB_OIR_LED  = leb_oir_led_test
OBJ_LEB_OIR_REG  = leb_oir_LTC4215_reg_test
OBJ_LEB_OIR_INIT = leb_oir_init
OBJ_LEB = \
	$(OBJ_LEB_OIR_LED) \
	$(OBJ_LEB_OIR_REG) \
	$(OBJ_LEB_OIR_INIT)

#
# lebowski SM module host side make rule example
#
$(OBJ_LEB_OIR_LED) :$(LEB_OIR_LED)
$(OBJ_LEB_OIR_REG) :$(LEB_OIR_REG)
$(OBJ_LEB_OIR_INIT):$(LEB_OIR_INIT)
$(OBJ_LEB):
	$(CC) $(NET_BASE) $^ -o $@ $(INC) $(LDFLAGS)


all:$(PY_DIAG_HFILE) $(OBJ_LEB)

$(PY_DIAG_HFILE):$(PY_DIAG_PCFG)
#generate the diag_flag_create_from_py.h by executing following python cmd
#note that the arg[1] after create_diag_flag_to_C.py is diag_flag.pcfg path
#arg[2] is diag_flag_create_from_py.h path
#$@ : Target name
#$^ : All dependencies/prerequisites
	$(PY_BIN)/bin/python $(PY_SCRIPT_DIR)/create_diag_flag_to_C.py $^ $@

clean:
	rm -f $(OBJ_LEB)

.PHONY: all clean

#**********History**********
#$Log: Makefile,v $
#Revision 1.2  2014/06/03 10:53:28  erwu2
#python menu collapsed to main trunk
#
#Revision 1.1.2.3  2014/04/29 11:40:40  erwu2
#update python file structure
#
#Revision 1.1.2.2  2014/04/24 08:53:52  erwu2
#merge makefile and add flag example to test
#
#Revision 1.1.2.1  2014/04/19 07:14:33  erwu2
#modularized makefile updated
#
#
#
#$Endlog$
