/* $Id: moncmds.c,v 1.4 2014/06/03 10:53:33 erwu2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/moncmds.c,v $
 *------------------------------------------------------------------
 *
 * moncmds.c - Monitor commands for Informers
 *
 * May 2008, Shih-Nan Huang adapted from Xformers.
 *
 * Copyright (c) 2013-2014 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include "endians.h"
#include "setjmps.h"
#include "monitor.h"
#include "types.h"
#include "dev_print.h"

extern int addrloop(), setalias(), alt_mem(), berrscan(), mon_boot();
extern int break_point(), call(), cat(), memory_checksum();
extern int clrerrlog(), cmp_mem(), confreg(), sys_cont();
extern int show_context(), toss_cookie(), cpu_config(), hw_cycles();
extern int devs(), directory(), disasm(), dnld(), dram_verify();
extern int dis_mem(), echo(), dump_n_banner(), dumperrlog(),cat(), fil_mem();
extern int show_frame(), help(), history(), fil_mem(), initfs();
extern int jump(), sys_launch(), memdebug(), meminfo();
extern int memloop(), memtest(), diag_menu(), mov_mem(), paritytest();
extern int repeat(), do_reset(), setalias(), msleep_cmd(), speed_lp();
extern int show_stack(), sync(), sysretdump(), tcal(), tlbdump();
extern int tlbflush(), tlbmap(), tlbpid(), tlbphy(), tlbtest();
extern int tlbscan(), tlbvir(), unset(), unset();
extern int xmodem(), test_watchdog(), priv(), return_to_roms();
extern int display_tlb_mapping(), show_intr_info();
extern int show_nm_pci_cli(), show_nm_info();
extern int progwdc(), testwdc(), rmadelete(); 
extern int disflag(), setflag(), settime(), setdate(), discovery();
extern int cookie(), test(), disrtc(), auth(), quack(), basic_util();
extern int volt_freq(), vltmrgn(), freqmrgn();
extern int clrdblog(),dumpdblog();

struct monitem moncmd[] = {  /* in alphabetical order */
  {"addrloop",addrloop,"walk 1 thru range of addresses"},
  {"alias",setalias,"NOT supported - set and display aliases command"},
  {"alter",alt_mem,"alter locations in memory"},
  {"auth",auth,"authentication smart chip screen"},
  {"berrscan",berrscan,"NOT supported - scan range of addresses for bus errors"},
  {"basic_util",basic_util,"Show basic utility menu"},
  //  {"call",call,"call a subroutine at address with converted hex args"},
  //  {"checksum",memory_checksum,"checksum a block of memory"},
  {"clrerr",clrerrlog,"clear the error log"},
  {"clrdebug",clrdblog,"clear debug info been used via dbprint"},
  {"cookie", cookie, "modify/display cookie"},
  {"compare",cmp_mem,"compare two blocks of memory"},
#ifdef NOTDEF
  {"confreg",confreg,"configuration register utility"},
  {"cpu", cpu_config, "cpu / system information and control"},
  {"dis", disasm, "disassemble instruction stream"},
#endif
  {"discovery", discovery, "Show every information on the system"},
  {"disflag", disflag, "NOT supported - display all the diag flag"},
  {"disrtc", disrtc, "display RTC"},
  {"dump",dis_mem,"display a block of memory"},
  {"echo",echo,"monitor echo command"},
  {"errlog", dumperrlog,"display the error log"},
  {"fill",fil_mem,"fill a block of memory"},
  //{"gdb",gdb_cntrl,"NOT supported - control gdb source level debugging"},
  {"help",help,"monitor builtin command help"},
  {"history",history,"monitor command history"},
  {"ifill",fil_mem,"NOT supported - fill a block of memory w/incrementing pattern"},
#if NOTDEF
  {"intr",show_intr_info,"show main board interrupt information"},
#endif
  //  {"jump",jump,"call a subroutine at address with argc/argv"},
  {"memdebug",memdebug,"write/read/verify scope loop"},
  //  {"meminfo",meminfo,"main memory information"},
  {"memloop",memloop,"write or read scope loop"},
  {"memtest",memtest,"simple memory test"},
  {"menu",diag_menu,"main diagnostic menu"},
  {"move",mov_mem,"NOT supported - move a block of memory"},
  {"msleep",msleep_cmd,"millisecond msleep command"},
#if NOTDEF
  {"pci",show_nm_pci_cli,"show NM PCI configuration"},
#endif
  {"quack",quack,"Program and Authenticate digital signature"},
  //  {"reload",do_reset,"system reset (just like POR)"},
  //  {"rel",do_reset,"system reset (just like POR)"},
  {"repeat",repeat,"repeat a monitor command"},
  //  {"reset",do_reset,"system reset (just like POR)"},
  {"rmadelete",rmadelete,"NOT supported - delete all CSL licenses"},
  {"setdate",setdate,"set date for IC2 Real Time Clock"},
  {"setflag",setflag,"NOT supported - modify all the diag flag"},
  {"settime",settime,"set time for IC2 Real Time Clock"},
  {"showdebug",dumpdblog,"show debug info via dbprint"},
#if NOTDEF
  {"shownm",show_nm_info,"show NM HW signals"},
#endif
  {"sync",sync,"NOT supported - write monitor environment to NVRAM"},
  //  {"stack", show_stack, "produce a stack trace"},
  {"test", test, "Perform Diag test"},
  //  {"tcal",tcal,"timer calibration test"},
#if NOTDEF /* No TLB entries in Intel CPU */
  {"tlbdisp", display_tlb_mapping, "display the Red Baron TLB"},
  {"tlbdump", tlbdump, "display the cpu TLB"},
  {"tlbflush", tlbflush, "flush the TLB"},
  {"tlbmap", tlbmap, "initialize a TLB mapping"},
  {"tlbpid", tlbpid, "set/display process ID number"},
  {"tlbphy", tlbphy, "search TLB for physical translation"},
  {"tlbscan", tlbscan, "scan for TLB exceptions"},
  {"tlbtest", tlbtest, "test the TLB"},
  {"tlbvir", tlbvir, "search TLB for a virtual translation"},
#endif
  {"unalias",unset,"NOT supported - unset an alias"},
  {"unset",unset,"NOT supported - unset a monitor variable"},
  //  {"watchdog", test_watchdog, "test watchdog rebooting of the box"},
  //  {"showdebug",dumpdevprint,"show debug info via db_print"},
  //  {"clrdebug",clrdevprint,"clear debug info been used via db_print"},
  {"progwdc",progwdc,"program watchtower device certificate"},
  {"testwdc",testwdc,"Currently NOT available - read out and verify WDC"},
  {"voltfreq", volt_freq, "display voltage and frequency margin"},
  {"vmrgn", vltmrgn, "do voltage margin with different levels"},
  {"fmrgn", freqmrgn, "do frequency margin with different levels"},
#ifdef MAKETHISPUPPYBOOT
  MON_BOOT_CMD,mon_boot,"boot up an external process",
  "break", break_point, "set/show/clear the breakpoint",
  "cycles", hw_cycles, "excercise the hardware with all possible cycles",
  "cat",cat,"concatenate files",
  "cont", sys_cont, "continue executing a downloaded image",
  "context", show_context, "display the context of a loaded image",
  "dev",devs,"list the device table",
  "dir",directory,"list files in file system",
  "dnld",dnld,"serial download a program module",
  "dram", dram_verify, "verify DRAM",
  "fdump",cat,"file dump utility",
  "frame", show_frame, "print out a selected stack frame",
  "initfs",initfs,"re-initialize the file system access structures",
  "launch", sys_launch, "launch a downloaded image",
  "partest",paritytest,"memory parity test",
  "priv",priv,"enter the privileged command state",
  "stack", show_stack, "produce a stack trace",
  "speed", speed_lp, "timed performance loop",
  "sysret", sysretdump, "print out info from last system return",
  "xmodem", xmodem, "x/ymodem image download", 
#endif /* MAKETHISPUPPYBOOT */

};
#define MONCMDSIZ (sizeof(moncmd)/sizeof(struct monitem))

int moncmdsiz = MONCMDSIZ;

/******** History ******** 
$Log: moncmds.c,v $
Revision 1.4  2014/06/03 10:53:33  erwu2
python menu collapsed to main trunk

Revision 1.3  2014/02/18 09:11:12  alpeng
CSCul88171-3: remove useless files: mon_boot.h, c82576_ethmap.h, eth_frames.c, stack.h, sys_regs.h

Revision 1.2  2013/10/08 11:14:27  erwu2
enhanced err msg first check-in

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.7  2013/01/31 10:48:45  alpeng
supported CLI cmds for voltage and freq margin

Revision 1.6  2012/10/03 10:54:22  danchung
Support CLI cmd clrerr and errlog

Revision 1.5  2012/08/16 00:58:30  alpeng
indicate the CLI cmds which are not supported

Revision 1.4  2012/08/14 09:46:39  alpeng
support CLI cmd addrloop

Revision 1.3  2012/04/18 07:20:42  alpeng
remove unsupported CLI commands: sync, unset, alias, gdb, and unalias

Revision 1.2  2012/03/28 00:38:21  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:08  ptong
Initial archive of ng_diag module


$Endlog$
*/
