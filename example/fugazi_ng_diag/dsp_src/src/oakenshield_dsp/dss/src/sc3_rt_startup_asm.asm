; $Id: sc3_rt_startup_asm.asm,v 1.2 2017/07/28 07:58:36 harrchan Exp $
; $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/dss/src/sc3_rt_startup_asm.asm,v $
;------------------------------------------------------------------
;
; sc3_rt_startup_asm.asm
;
; May 2012 
;
; Copyright (c) 2012-2017 by Cisco Systems, Inc.
; All rights reserved.
;
;------------------------------------------------------------------
;

;*****************************************************************************
;
;   (c) Copyright LSI Corporation 2006-2011. All rights reserved.
;
;*****************************************************************************

;************************************************************************HEAD*
;
; Description :  StarCore Compiler C/C++ Runtime Library Startup Code
;
;************************************************************************HEAD*

;************************************************************************INCS*
; Includes, Preprocessor and Global Definitions
;************************************************************************INCS*

  define REG_INIT '$7ead'              ; Word value used to intialize registers.

  ; Undefine the CACHE_SUPPORT symbol if no cache initialization is needed.
  ; This saves some bytes of code size.
  ; Note: Even if the CACHE_SUPPORT is defined, cache initialization can be
  ; enabled/disabled in the linker command file.
  ;
  define CACHE_SUPPORT '1'

  IF @DEF('__SC3400E__') | @DEF('__SC4000__')
    ; Undefine the ENABLE_LOOPCACHE symbol if the loop cache should not
	; be enabled.
	;
    define ENABLE_LOOPCACHE '1'
  ENDIF

  ; Undefine the CPP_SUPPORT symbol if no C++ support is needed.
  ; This saves some bytes of code size.
  ;
  define CPP_SUPPORT '1'

  IF @DEF('__SC1000__')
    define syncio ''
  ENDIF

; 
; The IntvecDefined symbol must be defined by a module, which implements the interrupt
; vector in the .intvec section.
; If no such module is provided, the default interrupt vector is taken from the runtime
; library.
; To define another interrupt vector, a separate assembler file has to be created, which
; is linked to the executable and which defines the IntvecDefined symbol.
;
  use IntvecDefined

;************************************************************************CONS*
; Constants
;************************************************************************CONS*
ARGV_LENGTH equ 256	                   ; Length of the input parameter 
                                       ;  vector (___argv) in bytes


; defined to get no linker error if a custom lcf is used, which does not
; define this symbols.
;
	weak _cpp_staticinit_start
_cpp_staticinit_start equ 0
	weak _cpp_staticinit_end
_cpp_staticinit_end equ 0

	weak _SR_ESM
	IF @DEF('__ESM_ENDIAN__')
_SR_ESM equ 0x04000000
	ELSE
_SR_ESM equ 0x00000000
	ENDIF

	weak _SR_CFM

;************************************************************************VARS*
; Data Sections
;************************************************************************VARS*

  section .argv 
  sectype progbits
  secflags noexecinstr

; ARGC/ARGV buffer is defined for use by the debugger/host program
;  to pass in arguments using the following format.
;
;       argc               4 bytes 
;  +--  argv[0]            4 bytes
;  |    argv[1]            4 bytes
;  |    ..
;  |    argv[argc-1]       4 bytes
;  +--> zero terminated    string #0 pointed to by argv[0]
;       zero terminated    string #1 pointed to by argv[1]
;       ...
;     
; ARGC/ARGV buffer is ARGV_LENGTH in size.  Debugger/host program passed
;  data should not overflow this buffer.

  global ___argv

  align 4
___argv
  ds ARGV_LENGTH

  endsec

;************************************************************************FCDC*
; Code Sections
;************************************************************************FCDC*

  section .text

  global ___crt0_start
  global ___crt0_end

; The following labels state that each function concluding
;  the label after the point adheres the ABI 4 calling convention.
;  This allows the linker to check whether the function definition
;  or the function call in another file adheres the specified
;  calling convention, too.
;
  IF @DEF('__ABI4__')
@label "__abi4._main"
@label "__abi4._exit"
@label "__abi4.___crt0_start"
@label "__abi4.___crt0_end"
@label "__abi4.__init_heap"
@label "__abi4.__init_bss"
@label "__abi4.__init_rom"
@label "__abi4.___exec_staticinit"
  ENDIF


;************************************************************************FCHD*
;
; Function:      ___crt0_start
;
; Description:   Reset function for the C/C++ RT lib.  Initialized environment
;                to default settings.
;
; C-Callable:    NO
;
; Parameters:    n/a
;
; Return Value:  n/a
;
; Regs Used:     d0-d15, r0-r15, n0-n3, m0-m3
;
; Algorithm:      
;
; Cautions/
;  Assumptions:  
;
;************************************************************************FCHD*
___crt0_start:				type func

; Initialize all registers to a known state.  This, in general, is not
; required but instead is good programming practice and is needed if this
; code is run on a Verilog simulator.
  [ move.w #REG_INIT,d0 
    move.w #REG_INIT,d1
  ] 
  [ tfr d0,d2
    move.w #REG_INIT,r0 
    move.w #REG_INIT,r1
  ]    
  [ tfr d0,d3
    move.w #REG_INIT,r2 
    move.w #REG_INIT,r3
  ]    
  [ tfr d0,d4
    tfra r0,r4
    tfra r0,r5
  ]    
  [ tfr d0,d5
    tfra r0,r6
    tfra r0,r7
  ]    
  [ tfr d0,d6
    tfra r0,r8
    tfra r0,r9
  ]    
  [ tfr d0,d7
    tfra r0,r10
    tfra r0,r11
  ]    
  [ tfr d0,d8
    tfra r0,r12
    tfra r0,r13
  ]    
  [ tfr d0,d9
    tfra r0,r14
    tfra r0,r15
  ]    
  [ tfr d0,d10
    tfra r0,n0
    tfra r0,n1
  ]    
  [ tfr d0,d11
    tfra r0,n2
    tfra r0,n3
  ]    
  [ tfr d0,d12
    move.l d0,m0
    move.l d0,m1
  ]    
  [ tfr d0,d13
    move.l d0,m2
    move.l d0,m3
  ]    
  [ tfr d0,d14
    tfra r0,sp                         ; This will be reinitialized to the 
                                       ;  correct value after the data 
                                       ;  initialization from ROM is done.
    tfra r0,osp
  ]    
  [ tfr d0,d15
    move.l #(_SR_Setting | _SR_ESM | _SR_CFM),sr             ; Initialized to the value defined in 
                                       ;  the LCF.
  ]    
  IF @DEF('__SC3000E__')
    weak __arbase__

    move.l #__arbase__,r0
    tfra r0,ar
  ENDIF

  
; NOTE:  SP still doesn't have correct value in it.  Cannot use subroutine
;  calls until it is set.


  ; Initialize the cache.
  ; The cache control register address and value are defined in the linker
  ; command file.
  ;
  IF @DEF('CACHE_SUPPORT')
    weak ___PCC_ENABLE
    weak ___PCC_ENABLE_MASK
    weak ___PCC_CTRL
    weak ___PCC_CTRL_MASK
    weak ___DCC_ENABLE
    weak ___DCC_ENABLE_MASK
 
	IF @DEF('__ESM_ENDIAN__')
      bmclr #0x0400, sr.h
	ENDIF

    move.l #___PCC_ENABLE, r0
    nop
    tsteqa.l r0  ; check if PCC should be invalidated and enabled

    ; invalidate PCC
    ;
    move.l #3,d0
    IF @DEF('__SC3000__')
        iff move.l d0,(r0+0x10) syncio
	ELSE
        iff move.l d0,(r0+0xa0) syncio
    ENDIF

    ; enable PCC
    ;
    move.l #___PCC_ENABLE_MASK, d0
    iff move.l d0, (r0) syncio

	; set fetch-ahead in PCC_CTRL (only for SP28xx)
	;
	move.l #___PCC_CTRL, r0
	nop
	tsteqa.l r0  ; check if PCC_CTRL should be written
	move.l #___PCC_CTRL_MASK,d0
	iff move.l d0, (r0) syncio

    move.l #___DCC_ENABLE, r0
    nop
    tsteqa.l r0  ; check if DCC should be invalidated and enabled

    ; enable DCC
    ;
    move.l #___DCC_ENABLE_MASK, d0
    iff move.l d0, (r0) syncio

    ; invalidate DCC
    ;
    move.l #7,d0
    IF @DEF('__SC3000__')
        iff move.l d0,(r0+0x40) syncio
	ELSE
        iff move.l d0,(r0+0xa0) syncio
    ENDIF

	IF @DEF('__ESM_ENDIAN__')
      bmset #0x0400, sr.h
	ENDIF

  ENDIF

  IF @DEF('ENABLE_LOOPCACHE')
      ; enable the loop cache if it is a 1.1 device or higher
      move.l 0x80000030,d0
      cmpgt.w #1,d0
      ift bmset #$0004,idcr.l
  ENDIF

; Initialize Stack and VBA.  Stack definition comes from RT library variable
;  ___size which is set by default to the LCF defined symbol _TopOfStack.
;  VBA is defined to the LCF defined symbol _VBAddr.
;
; SP and ___size should be aligned to an 8-byte bound.  To guarentee this,
;  we have code to mask off the lower 

; Initialize r0 wirth stack start address from linker configuration file.
  move.l #_StackStart,r0
  move.l #>_VBAddr,vba
  bmclr #$07,r0.l
  nop
  tfra    r0,sp
  move.l r0,>___size

; Initialize RAM data sections that were not initialized on download.  This
;  includes the BSS sections and ROMed data variables.
    
  jsr __init_bss

  jsr __init_rom
  
; Initialize heap

  jsr __init_heap
  
; Initialize C++ domain
  IF @DEF('CPP_SUPPORT')
    jsr ___exec_staticinit
  ENDIF

; Setup ARGC and ARGV for main().  The ABI2 calling convention put
;  ARGC in d0 and ARGV in r1. The ABI4 calling convention put
;  ARGC in d0 and ARGV in r0. Note the definitions of ARGC and 
;  ARGV from the comments in the "Memory Declarations" section.

Sc3RtStartup:

  IF @DEF('__ABI4__')
    move.l #___argv,r0
    nop
    move.l (r0)+,d0
  ELSE
    move.l #___argv,r1
    nop
    move.l (r1)+,d0
  ENDIF

; Call user application.  If the user application calls exit(), 
; exit will call ___crt0_end.
  jsr _main           

; User application did not call exit() (instead it returned) so need
;  to call exit() to handle RT shutdown.

  jsr _exit

___crt0_end:				type func
                                       ; exit() will explicitly call 
                                       ;  ___crt0_end.  ___crt0_end is
                                       ;  the default end breakpoint
                                       ;  symbol for the SC debugger.
  jmp Sc3RtLibHalt                     ; Explicitly jump to halt function
                                       ;  if no breakpoint set at ___crt0_end

;************************************************************************FCHD*
;
; Function:      Sc3RtLibHalt
;
; Description:   Stopping point for execution.
;
; C-Callable:    NO
;
; Parameters:    n/a
;
; Return Value:  n/a
;
; Regs Used:     
;
; Algorithm:                     
;
; Cautions/
;  Assumptions:  
;
;************************************************************************FCHD*
Sc3RtLibHalt:				type func
  IF @DEF('__SC3000E__')
    wait
  ELSE
    stop
  ENDIF
  jmp Sc3RtLibHalt

Sc3RtLibHaltEnd:

  endsec  ; .text

;######## HISTORY ########
; $Log: sc3_rt_startup_asm.asm,v $
; Revision 1.2  2017/07/28 07:58:36  harrchan
; Collapse Oakenshield-branch to Main Trunk.
;
; Revision 1.1.2.1  2017/06/29 08:14:30  harrchan
; Initial commit code for Oakenshield
;
; Revision 1.1  2012/04/18 18:08:36  srane
; Initial checkin
;
;
; $Endlog$

