/* $Id: cpu.h,v 1.2 2012/03/28 00:38:10 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/include/cpu.h,v $
 *------------------------------------------------------------------
 * cpu.h -- define the cpu types and associated strings
 *
 * August 1986, Greg Satz
 * Michael Beesley, January 1994
 *
 * Copyright (c) 2007-2012 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

extern int cpu_type;

#define	CPU_SMI8	0	/* never used -- predates cisco */
#define	CPU_CADLINC	1	/* never used -- predates cisco */
#define	CPU_SMI10	2	/* never used -- predates cisco */
#define	CPU_FTI10	3	/* never used -- predates cisco */
#define	CPU_CSC1	4	/* also called P20, 68000 @ 10MHz */
#define	CPU_UNIX	5	/* we are a UNIX user mode process */
#define	CPU_CSC2	6	/* also called PIPPIN, 68020 @ 12MHz */
#define	CPU_MPU12	7	/* never used */
#define	CPU_CSC3	8	/* 68020 @ 30 MHz */
#define CPU_STS1	9	/* 68010 in CMC Small Terminal Server */
#define CPU_PAN		10	/* IGS (was called Pancake, or HYBRIDGE II),
                                   16 MHz 68020 */
#define CPU_MERLOT	11	/* Merlot (Token ring IGS), 20 Mhz 68030 */
#define CPU_LEMONADE	12	/* Lemonade (16 line terminal server) */
#define	CPU_CSC4	13	/* 68EC040 @ 25 MHz */
#define	CPU_XX  	14	/* XX (the 4000), 40 Mhz 68030 */
#define CPU_IGS_BRUT    15	/* IGS-BRUT,68030 20 MHz, w/ no CPU memory */
#define CPU_RP1		16	/* 68EC040 @ 25 MHz */
#define CPU_BASS	17	/* Bass and Pinot */
#define CPU_CRISTAL	18	/* Cristal (3 port merlot) */
#define CPU_CANCUN	19	/* Cancun like Cristal with ASIC's and BRUT */
				/* Timers */
#define CPU_SIERRA      20      /* The C4000 with the Orion chip (R4600) */
#define CPU_RSP         21      /* integrated route-switch processor */
#define CPU_SAPPHIRE    22      /* 68360 aka "QUICC chip" */
#define CPU_SYNALC      23      /* Synergy ATM Line Card (25MHz 68EC030) */
#define CPU_VIP		24	/* R4600 based IP running IOS */
#define CPU_C7100	25	/* Predator */
#define CPU_i86		26	/* Intel x86    */
#define CPU_RHINO       27      /* LS1010 ATM Switch*/
#define CPU_BRASIL      28      /* The AS5200, a Cancun superset. Define used */
                                /* only for Brasil family type creation. */
#define CPU_VOLCANO     29      /* 68360 (C100X) + Wan Module slot */
#define CPU_C3600       30      /* R4700-based Modular Access Router (MARs) */
#define CPU_NP1         31      /* LightStream NP1 - 25Mhz 68040 */
#define CPU_ASP         32      /* LightStream ASP - 75Mhz PowerPC 603 */
#define CPU_MALIBU      33      /* Grand Junction FE Switch - PowerPC 403 */
#define CPU_RINCON      34      /* StrataCom RINCON Access Switch - 33MHz
                                   MC68360 */
#define CPU_JANEIRO     35      /* R4700-based JANEIRO */
#define CPU_BFRP        36      /* BFR Route Processor */
#define CPU_BFLC        37      /* BFR Line Card */
#define CPU_C2KATM      38      /* SIBU ATM module - PowerPC 403 */
#define CPU_AMAZON      39      /* Mica Board used on AS5300 */
#define CPU_HMM         40      /* Mica Modems */
#define CPU_MCOM_56K    41      /* Microcom 56K Modems on AS5200/AS5300 */
#define CPU_NITRO       42      /* Dialup aggregation card - Nitro */
#define CPU_QUAKE	43	/* Quake - MPC860 Based */
#define CPU_MC3810      44      /* MC3810 PowerQUICC based */
#define CPU_C6200       45      /* Dagaz Jera DSL Mux - Mips 4640 */
#define CPU_MILAN       46      /* Milan */
#define CPU_MILANLC     47      /* Milan LC */
#define CPU_SOHOCM      48      /* SOHO Cable Modem */
#define CPU_NRP1        49      /* Santa xDSL aggregator NRP router card */
#define CPU_RPM         50      /* RPM (7200 blade in popeye chassis) */
#define CPU_FERRARI	51	/* Ferrari - MPC860 Based */
#define CPU_LEOPARD     52      /* Leopard a MARs chassis with r5k */
#define CPU_COUGAR	53	/* cougar chassis with r5k  */
#define CPU_SABRECAT	54	/* sabrecat chassis with r5k  */
#define CPU_JAGUAR	55	/* jaguar chassis with r4k  */
#define CPU_HNG         56      /* Home Network Gateway */
#define CPU_DRACO       57      /* Constellation Draco router card */
#define CPU_C1400       58      /* Porsche/c1400 - 68360 based */
#define CPU_REGEN48     59	/* Mantis/c800 - PowerQuicc */
#define CPU_ATLANTIS    60      /* R7000 based Atlantis */
#define CPU_ARIES       61      /* Constellation (Cat6k) RSM with 5k */
#define CPU_C800        62      /* Mantis/c800 - PowerQuicc */
#define CPU_COPERNICO   63      /* Cat5k router daughter card with r5k */
#define CPU_CYGNUS      64      /* Draco running Constellation "native" */
#define CPU_WILDCAT     65      /* wildcat chassis with r5k  */
#define CPU_C10K        66      /* Omega - leased line aggregator */
#define CPU_UBR920      67      /* Stinger/ubr920 - powerQuicc based */
#define CPU_EGR         68      /* Enterprise Gateway Router */
#define CPU_DOMINO      69      /* Domino box with 5k */
#define CPU_NEXPORT     70      /* AS5400 & AS5800 NextPort Architecture */
#define CPU_DRACO2      71      /* Cat6K router card with Mistral ASIC */
#define CPU_C6100       72      /* Netspeed Looprunner */
#define CPU_RAIKO       73      /* R7k based Nitro generic DFC carrier card */
#define CPU_VG200       74      /* LesPaul: 2600 Variant   */
#define CPU_TUCANA      75      /* ATM uplink for cat6k- r5k based */
#define CPU_CWPA        76      /* Constellation WAN Port Adapter linecard */
#define CPU_CWTLC       77      /* Constellation WAN Toaster linecards */
#define CPU_IRONFIST    78      /* R7k based IronFist RSC card for Marvel */
#define CPU_UBR910      79      /* Manhattan/ubr910 - powerQuicc based */
#define CPU_MARINA      80      /* Cat4k router card - Marina */
#define CPU_UBR925      81      /* Hornet - ubr925 cable-modem/router */
#define CPU_UBR905      82      /* Blue Hawaiian - ubr905 cable-modem/router */
#define CPU_CHOPIN      83      /* Chopin voice gateway card for cat4k */
#define CPU_CWAN        84      /* Constellation WAN linecards shared rommon */
#define CPU_VSR         85      /* VPN Security Router */
#define CPU_NRP2        86      /* c6400 aggregator NRP2 router card */
#define CPU_URM         87      /* IGX-based Router blade for URM */
#define CPU_PIPER_ARROW 97      /* R7K Based 3600 (PIPER) series 2 NM slots */
#define CPU_C950        98      /* Fireplace/c950-powerQuicc based wireless */
#define CPU_AS5XX0_TRUNK_DFC  99 /* For all trunk DFC cards (E1/T1/CT3) */
#define CPU_C806        100     /* Zwicky: SOHO E2E router, MPC855T based */
#define CPU_TWSISTER    101     /* R7K Telco 3600 (PIPER) series   */
#define CPU_PIPER_CUB   102     /* R7K Based 3600 (PIPER) series 1 NM slot  */
#define CPU_WILMA       103     /* R7K Based 3600 (PIPER) series */
#define CPU_NSP2        104     /* c6400 NSP2 switch with R7K */
#define CPU_TOMAHAWK    105     /* R7K Based 3600 series platform */
#define CPU_JOBIM       106     /* Jobim CPU type */
#define CPU_C10K2       107     /* Omega (c10000) PRE2 */
#define CPU_SOHO71      117     /* Whirlpool: SOHO E2E router, MPC855T based */
#define CPU_CAMR        118     /* c10720, access metro router with 5k */
#define CPU_NP405       119     /* Nextport IBM 405 based DFC boards */
#define CPU_PROCYON     120     /* Procyon dual cpu for Constellation 2 */
#define CPU_CWPA2       121     /* FlexWAN2 linecard for Constellation2 */
#define CPU_SIP2        122     /* SPA Interface Processor-2 for OSR */
#define CPU_SWELL       123     /* R5K Based iad2400 series platform */
#define CPU_DCMTS       124     /* Distributed CMTS (DCMTS) */
#define CPU_C3200       125     /* Hercules-A:  MPC8250 based */
#define CPU_SIMPSON     126     /* CAT6K SSL Termination Engine Linecard */
#define CPU_C837        127     /* Milkyway ADSL over POTS, MPC 857DSL based */
#define CPU_SOHO97      128     /* Milkyway SOHO ADSL over POTS, MPC 857DS */
#define CPU_C831        129     /* Milkyway E2E rotuer, MPC 857DSL based */
#define CPU_SOHO91      130     /* Milkyway SOHO E2E router, MPC 857DSL based */
#define CPU_JUMPGATE    131     /* JumpGate ARM/Xscale platform */
#define CPU_C836        132     /* Milkyway ADSL over ISDN, MPC 857T based */
#define CPU_SOHO96      133     /* Milkyway SOHO ADSL over ISDN, MPC 857T */
#define CPU_GIOVE       134     /* Low end router based on R5k */
#define CPU_CAVIUM_EVAL 135     /* FIXME: (until we get official word) just use same as fellowship */
#define CPU_SIP1        136     /* SPA Interface Processor-1 for OSR */
#define CPU_VEGAS       137     /* Vegas/Miami (3550) platform */
#define CPU_BELLAGIO    138     /* Vegas/PPC405 platform */
#define CPU_CALHOUN     139     /* Calhoun/R3K (295x) platform */
#define CPU_HULC        140     /* HULC/Yeti (3750) platform */
#define CPU_C10K3       141     /* Omega (c10000) PRE-3 */
#define CPU_C7301       142     /* TAZ-II: 1RU Router based on 7200/NPE-G1 */
#define CPU_C3220       143     /* Twin:Mobile Access Router based on MPC8250 */
#define CPU_PALMBEACH   144     /* Palm Beach video Line card (cat4k chassis) */
#define CPU_SOPWITH     145     /* a 2 nm sopwith midrange chassis with bcm */
#define CPU_CURTISS     146     /* a 4 nm Curtiss midrange chassis with bcm */
#define CPU_CUISINART   147     /* Low end fixed config routers based on PQ3 */
#define CPU_UNUSED_148  148     /* see comment below */
#define CPU_UNUSED_149  149     /* see comment below */
#define CPU_REDBARON    150     /* Red Baron midrange chassis with bcm */
#define CPU_UNUSED_151  151     /* see comment below */
#define CPU_UNUSED_152  152     /* see comment below */
#define CPU_UNUSED_153  153     /* see comment below */
#define CPU_UNUSED_154  154     /* see comment below */
#define CPU_UNUSED_155  155     /* see comment below */
#define CPU_UNUSED_156  156     /* see comment below */
#define CPU_UNUSED_157  157     /* see comment below */
#define CPU_UNUSED_158  158     /* see comment below */
#define CPU_UNUSED_159  159     /* see comment below */
#define CPU_UNUSED_160  160     /* see comment below */
#define CPU_UNUSED_161  161     /* see comment below */
#define CPU_UNUSED_162  162     /* see comment below */
#define CPU_UNUSED_163  163     /* see comment below */
#define CPU_UNUSED_164  164     /* see comment below */
#define CPU_UNUSED_165  165     /* see comment below */
#define CPU_C3270       166     /* see comment below */
#define CPU_C3240       167     /* see comment below */
//#define CPU_UNUSED_168  168     /* see comment below */
#define CPU_ORBITTY     168     /* Orbitty MPC8323E */
#define CPU_UNUSED_169  169     /* see comment below */
#define CPU_UNUSED_170  170     /* see comment below */
#define CPU_UNUSED_171  171     /* see comment below */
#define CPU_UNUSED_172  172     /* see comment below */
#define CPU_UNUSED_173  173     /* see commmnet below */
#define CPU_C200        174     /* see comment below */
#define CPU_IFR         210     /* see comment below */

/****************************************************************
 * Always reserve new cpu types in the latest mainline branch
 * (eg. florida, georgia, etc).
 *
 * When adding a new cpu type please do the following:
 * 1. replace one of the above CPU_UNUSED_### values
 * 2. update cputypes in cpu.c
 *
 * Also don't forget to update the chassis MIB definitions in
 * your development branch.
 *
 * If there are no CPU_UNUSED_### values above, please contact
 * interest-os-boot to coordinate adding more.
 ****************************************************************/

#define CPU_UNKNOWN     211     /* LAST VALUE - unknown processor */

/****************************************************************
 * Platform supporting functions
 ****************************************************************/
/* init.c of Fellowship and RedBaron. mb_utils.c of Giove */
extern int get_cpu_type(void);

/* End of file */


/******** History ******** 
$Log: cpu.h,v $
Revision 1.2  2012/03/28 00:38:10  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:03  ptong
Initial archive of ng_diag module


$Endlog$
*/
