#!/bin/bash
# $Id: katar_banner.sh,v 1.2 2019/06/14 05:24:49 mikech2 Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/katar/x86/katar_banner.sh,v $
#------------------------------------------------------------------
#
# katar_banner.sh - Sell script to create the Katar banner.
#
# Copyright (c) 2012-2019 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#
#
#
#set -x
#set -v

#
# Setup the path and version stuff
#
PATH=/usr/local/bin:/usr/5bin:/bin:/usr/bin:/router/bin:/usr/ucb

if [ ! -r version ]; then 
	echo "No version file available ... user = Bozo" 2>&1
	exit 2
fi

. ./version

if [ x$MAJOR_VERSION = x -o x$MINOR_VERSION = x -o x$RELEASE = x ]
then
	echo "Bad version numbers, fix version file" 2>&1
	exit 2
fi

# If the .compile file does not exist or the version file is newer
# than the .compile file, reset the COMPILE variable to zero.
# This means that each time the version file has been changed the
# COMPILE is reset to zero (duh!).

if [ ! -r .compile ]
then 
	COMPILE=0
elif [ x`find version -newer .compile -print` = xversion ]
	then
		COMPILE=0
	else 
		COMPILE=`cat .compile`
fi

COMPILE=`expr $COMPILE + 1`

echo $COMPILE >.compile

USER_NAME=`whoami`
PRODUCT='Diagnostic'
BOX_TYPE='Katar'

#
# Setup banner.c
#
echo  "/* banner.c : Created for $BOX_TYPE by $USER_NAME on `date +'%a %d-%h-%y %H:%M'` */" > banner.c

#
# Write the banner string out to banner.c
#
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$RELEASE\($COMPILE\)


	echo "char *banner_string = \\" >> banner.c
	echo $ECHO_ARGS "\"\n$PRODUCT for $BOX_TYPE, Version $VERSION\nCompiled by $USER_NAME on `date +'%a %d-%h-%y %H:%M'`\n\";" >>banner.c

#
# Exit from banner.sh
#
exit 0

#------------------------------------------------------------------
# $Log: katar_banner.sh,v $
# Revision 1.2  2019/06/14 05:24:49  mikech2
# Collapse katar-branch00 to Main Trunk
#
# Revision 1.1.2.1  2018/10/22 08:02:22  mikech2
# Move project folder to common/src/katar/x86
#
# Revision 1.1.2.3  2018/09/14 06:11:53  mikech2
# Add mem info to system info
#
# Revision 1.1.2.2  2018/09/12 08:32:48  mikech2
# Fix userlogic FPGA update & system info version issue
#
# Revision 1.1.2.1  2018/06/25 08:24:53  mikech2
# Add interupt test menu
#
#
#
#------------------------------------------------------------------
# $Endlog$
