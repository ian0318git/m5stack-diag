#!/bin/bash
# $Id: banner.sh,v 1.22 2021/09/24 01:27:55 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/utils/banner.sh,v $
#------------------------------------------------------------------
#
# banner.sh - Sell script to create the Monitor banner.
#
# Copyright (c) 2012-2019 by Cisco Systems, Inc.
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

# If making Overlord (ISR4451/K9) or Juno platform
#
if [ "$1" = OVLD ]; then
	BOX_TYPE='Overlord'

elif [ "$1" = OVLD_DP ]; then
        BOX_TYPE='Overlord Data Plane'

# else if making Utah, Sword, Dagger, Goldbeach platforms
#
elif [ "$1" = UTAH ]; then
	BOX_TYPE='Utah/Sword/Dagger/Goldbeach'

# else if making Neptune platforms
#
elif [ "$1" = NEPTUNE ]; then
	BOX_TYPE='Neptune'

elif [ "$1" = NEP_DP ]; then
        BOX_TYPE='Neptune Data Plane'

# else if making Curie 1RU platforms
#
elif [ "$1" = CURIE_1RU ]; then
	BOX_TYPE='Curie-1RU'

# else if making Curie 2RU platforms
#
elif [ "$1" = CURIE_2RU ]; then
	BOX_TYPE='Curie-2RU'

# else could be Freescale-based Xformers
#
elif [ "$1" = FSL_XFORMERS ]; then
        BOX_TYPE='Freescale-based Xformers'

elif [ "$1" = FORTITUDE ]; then
	BOX_TYPE='Fortitude NGWIC'

elif [ "$1" = PRINCE ]; then
	BOX_TYPE='Prince NGWIC'

elif [ "$1" = REVA ]; then
        BOX_TYPE='Reva NGWIC'

elif [ "$1" = REVA_SM ]; then
        BOX_TYPE='Reva SM'

elif [ "$1" = WOODLAWN ]; then
	BOX_TYPE='Woodlawn NGSM'

elif [ "$1" = WALLANDER ]; then
	BOX_TYPE='Wallander NGWIC'

elif [ "$1" = CSX_TACHI ]; then
	BOX_TYPE='CSX-Tachi'
    
elif [ "$1" = TACHI_NIM_PCIE ]; then
	BOX_TYPE='Tachi NIM PCIE'
    
elif [ "$1" = TSN ]; then
	BOX_TYPE='TSN'

elif [ "$1" = VIPER_INTEL ]; then
	BOX_TYPE='Viper'

elif [ "$1" = BETELGEUSE ]; then
	BOX_TYPE='BETELGEUSE'

elif [ "$1" = HIGHRISE ]; then
	BOX_TYPE='Highrise'
        
elif [ "$1" = TABEI_L_INTEL ]; then
	BOX_TYPE='Tabei-L/Promethium'

elif [ "$1" = PHOENIX_INTEL ]; then
	BOX_TYPE='Phoenix'

elif [ "$1" = ELIXIR ]; then
	BOX_TYPE='ELIXIR'

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
	echo "char *version_str = \\" >> banner.c
	echo "\"v$VERSION\n\";" >> banner.c

#
# Exit from banner.sh
#
exit 0

#------------------------------------------------------------------
$Log: banner.sh,v $
Revision 1.22  2021/09/24 01:27:55  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.21  2021/04/14 09:11:24  achiu2
[PRRQ:CSCvx56970-2] Phoenix code review for ER

Revision 1.20  2020/08/19 09:51:43  markzha
*** empty log message ***

Revision 1.19  2020/08/17 07:18:07  kehuang2
Banner update

Revision 1.18  2020/03/06 08:10:19  haohsu
add if case for BETELGEUSE

Revision 1.17  2020/01/09 01:03:05  jiajliu
Merge Curie 2RU to main trunk

Revision 1.16  2019/08/06 06:56:18  alpeng
merge curie, switzer and nightwatch to trunk

Revision 1.15  2018/08/25 00:59:17  ptong
Corrected diag banner for Neptune

Revision 1.14  2018/08/06 02:30:59  harrchan
Merge viper E2E to the main trunk (CSCvk28469)

Revision 1.13  2018/05/18 09:25:02  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.12  2017/08/10 10:12:57  iachang
CSCvf44161: Merge Goldbeach into USD platform as one image

Revision 1.11  2017/08/02 14:21:58  steja
Support TSN-H/M platform code

Revision 1.10.8.1  2017/07/29 03:41:26  steja
tsn-branch5 synced with Maintrunk repositories

Revision 1.10  2017/03/16 10:55:48  umlin
Reva-SM: Commit Reva-SM platform side code to main trunk. RevaSM controller type is 0x0D77.

Revision 1.9  2016/05/09 05:51:57  umlin
Reva:
common/src/reva/diag.c        => Change wording in comment
common/src/reva/reva_ge_dma.c => Change wording in comment
common/src/reva/i2c_util.c    => Change wording for function name
common/src/reva/i2c_util.h    => Change wording for function name
common/src/reva/reva_ge_phy.c => Polling to check copper link status
utils/banner.sh               => Add banner BOX_TYPE for Reva
common/src/reva/Makefile      => Update FPGA bin file: reva_sb_mboot_rel

Revision 1.8.2.3  2018/05/17 10:51:19  alpeng
 sync with trunk <trunk-051618>

Revision 1.8.2.2  2017/04/05 06:46:21  leschen
Sync with <ng_diag-tag-032917>

Revision 1.8.2.1  2016/10/31 23:34:51  ptong
Support Neptune and NEP_DP

Revision 1.8  2016/04/20 07:06:41  benchen2
merge tachi branch into main trunk

Revision 1.7.4.2  2016/01/29 09:15:07  alpeng
include nim pcie test while complie

Revision 1.7.4.1  2015/06/11 02:01:11  tirawan
Add files for Tachi BMC project

Revision 1.7  2015/02/26 07:27:14  xiaoyizh
Add Wallander support.

Revision 1.6  2013/10/08 08:48:32  tirawan
Woodlawn collapsed to main trunk

Revision 1.5  2013/05/28 18:18:13  ptong
Add Utah/Sword/Dagger platforms

Revision 1.4  2013/04/23 07:32:58  xiaoyizh
Add Prince support.

Revision 1.3  2012/11/09 22:36:51  ywen
Add Fortitude support.

Revision 1.2  2012/08/10 22:21:43  ptong
Add overlord data plane diag version support

Revision 1.1  2012/08/07 23:42:12  mcharon
add banner.sh for versioning


#------------------------------------------------------------------
$Endlog$


