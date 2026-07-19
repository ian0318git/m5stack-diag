#!/bin/sh
# $Id: sample_install.sh,v 1.1 2014/05/05 22:20:34 mcharon Exp $
# Copyright (c) 2014 by Cisco Systems, Inc.
#rename this file to install.sh
#sample usage:
# make install
set -x
bzip2 -k -9 -f utah_lnx
cp utah_lnx.bz2 /tftpboot/$(whoami)

