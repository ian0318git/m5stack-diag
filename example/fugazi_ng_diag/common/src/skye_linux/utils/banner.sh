#!/bin/bash
# $Id: banner.sh,v 1.2 2015/05/25 03:59:27 steja Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/utils/banner.sh,v $
#------------------------------------------------------------------
#
# banner.sh - Sell script to create the Monitor banner.
#
# Copyright (c) 2015 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#
#
# banner.sh
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

# If making Shrinkray module
#
if [ "$1" = SHRINKRAY ]; then
	BOX_TYPE='Shrinkray'

# else could be Skye
#
elif [ "$1" = SKYE ]; then
	BOX_TYPE='Skye'

else
	BOX_TYPE='TEST'
fi


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
#$Log: banner.sh,v $
#Revision 1.2  2015/05/25 03:59:27  steja
#Add Support Skye SM
#
#Revision 1.1.4.2  2015/04/29 11:36:49  steja
#Code check-in to skye-branch2 for ER code review
#
#
#------------------------------------------------------------
#Revision 1.1.2.1  2014/07/21 01:56:46  palin2
#Initial check-in Skye module side Diag code.
#
#Revision 1.1.2.1  2014/06/04 09:41:17  palin2
#Add script to generate banner.
#
#------------------------------------------------------------------
#$Endlog$
