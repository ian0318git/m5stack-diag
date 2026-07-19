# $Id: init.S,v 1.3 2012/09/10 06:44:54 srane Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/sp27dsp/ppb/src/init.S,v $
#------------------------------------------------------------------
# init.S
#
# Mar 2012, Smita Rane 
#
# Copyright (c) 2012 by Cisco Systems, Inc.
# All rights reserved.
#  
#------------------------------------------------------------------ 
#  

#############################################################################
#                             NOTIFICATION
#
# Copyright (c) 2011 LSI Inc.
# All Rights Reserved
#
#  This is unpublished proprietary information of LSI Inc.  This
#  copyright notice does not evidence publication.
# 
#  The use of the software, documentation, methodologies, and other information
#  contained herein is governed solely by the associated license agreements.
#  Any inconsistent use shall be deemed to be a misappropriation of the
#  intellectual property of LSI Inc. and treated accordingly.
# ----------------------------------------------------------------------------
# 
#  $RCSfile: init.S,v $
# 
#  Description:	PPB (GNUARM) startup code for SP27XX evaluation boards
# 
#  $Log: init.S,v $
#  Revision 1.3  2012/09/10 06:44:54  srane
#  Code for ARM11 CPU1.
#
#  Revision 1.2  2012/06/07 22:50:59  srane
#  TDM external loopback, ECC memory test
#
#  Revision 1.1  2012/05/31 06:36:53  srane
#  Initial checkin.
#
# 
#############################################################################
.ifdef __CLIB_SUPPORT__
#.include "newlib.h"

.ifdef HAVE_INITFINI_ARRAY
.set _init,	__libc_init_array
.set _fini,	__libc_fini_array
.endif
.endif 

# .set LONG_CALLS,1

.section .first, "ax"
 
#.section .rsthdlr
	.code	32
	.align	0

#.section .diag_exception, "ax"
#############################################################################
#
# Start function linked to application start address.
#
#############################################################################

/* Moving allocation of stacks to linkscrip.ld */

# Now some standard definitions...

.equ Mode_Bits, 0x1F
.equ Mode_USR,  0x10
.equ Mode_IRQ,  0x12
.equ Mode_FIQ,  0x11
.equ Mode_SVC,  0x13
.equ Mode_ABT,  0x17

.equ I_Bit,  0x80
.equ F_Bit,  0x40

    .globl	_errrtn
    .globl	_fiqrtn
    .globl	_irqrtn

	.globl	_start
	.globl	odo_exception_handler
/* This is the exception handler which should placed at either 0x0 or
 * 0xFFFF0000
 */

#.section .odo_exception, #alloc, #execinstr
odo_exception_handler:
     B       _start
     B       Undefined_Handler
     B       SWI_Handler
     B       Prefetch_Handler
     B       Abort_Handler
     # Reserved vector
     NOP
     B       IRQ_Handler
     B       FIQ_Handler

# The following handlers do not do anything useful in this example.

.section .rsthdlr
odo_exception_notused:
     B	Error
Undefined_Handler:
     B       Error
SWI_Handler:
     B       Error
Prefetch_Handler:
# *******************************************************************************
# * Function Name  : PrefetchAbort_Handler
# * Description    : This function called when program abort exception is encountered.
# * Input          : none
# * Output         : none
# *******************************************************************************	
# R1 - prefetch FSR value
	MRC p15,0,R1,c5,c0,1
	B	.
	  
Abort_Handler:
# *******************************************************************************
# * Function Name  : DataAbort_Handler
# * Description    : This function called when data abort exception is encountered.
# * Input          : none
# * Output         : none
# *******************************************************************************	
# R0 - abort status
	MRC p15,0,R0,c5,c0,0
# R1 - abort data adress
	MRC p15,0,R1,c6,c0,0
	#SRB	.
# go to the Data Abort Handler function
        SUBS    R14, R14, #4
        STMFD  R13!, {R0-R3,R12,R14}
        mov             lr, pc
        ldr     pc, =data_abort_handler
        LDMFD  R13!, {R0-R3,R12,R15}^

          
IRQ_Handler:
    # make R0-R3 available for use locally
    SUBS    R14, R14, #4
    STMFD  R13!, {R0-R3,R12,R14}
    mov		lr, pc
    ldr 	pc, =irqrtn
    LDMFD  R13!, {R0-R3,R12,R15}^

FIQ_Handler:
    # make R0-R3 available for use locally
    SUBS   R14,   R14,   #4
    STMFD  R13!, {R0-R3,R14}
    mov		lr, pc
    ldr 	pc, =fiqrtn
    LDMFD  R13!, {R0-R3,R15}^

Error:
    # make R0-R3 available for use locally
    SUBS    R14, R14, #4
    STMFD  R13!, {R0-R3,R12,R14}
    mov		lr, pc
    ldr 	pc, =errrtn
    # Now make sure it returns in supervisor mode
    MRS    R0,   SPSR
    BIC    R0,   R0, #Mode_Bits
    ORR    R0,   R0, #Mode_SVC
    MSR    SPSR_c, R0
    LDMFD  R13!, {R0-R3,R12,R15}^

# The RESET entry point
#_DReset_Handler:
_start:
# Make sure that we are in supervisor mode and with interrupts disabled
	msr cpsr_c, #0xD3

# Invalidate both the ARM caches and flush the branch target cache
    MOV         R0, #0
    MCR   p15, 0, R0, c7, c7, 0

       # check if coreid != 0 write MAGIC to fied location */
        mrc p15, 0, r0, c0, c0, 5 
        ands r0, r0, #0xF
        bne go_core1 
    

# --- Initialise stack pointer registers
# Enter IRQ mode and set up the IRQ stack pointer
    MOV     R0, #Mode_IRQ|I_Bit|F_Bit
    MSR     CPSR_c, R0
    LDR     R13, =__IRQ_SP_INIT

# --- Initialise stack pointer registers
# Enter FIQ mode and set up the FIQ stack pointer
    MOV     R0, #Mode_FIQ|I_Bit|F_Bit
    MSR     CPSR_c, R0
    LDR     R13, =__FIQ_SP_INIT

# --- Initialise stack pointer registers
    MOV     R0, #Mode_ABT|I_Bit|F_Bit
    MSR     CPSR_c, R0
    LDR     R13, =__ABT_SP_INIT


# Set up other stack pointers if necessary
    # ...

# Set up the USR stack pointer - not used for PPB
#    MOV     R0, #Mode_USR|I_Bit|F_Bit
#    MSR     CPSR_c, R0
#    LDR     R13, =USR_Stack

# Set up the SVC stack pointer last and return to SVC mode
    MOV     R0, #Mode_SVC|I_Bit|F_Bit
    MSR     CPSR_c, R0
#   LDR     R13, =__SVC_SP_INIT

	# Initialize stack pointer
	ldr	r1, =__SP_INIT
	mov 	sp, r1

# exeption vector remapping flag

.equ V_Bit, 0x2000

# select_normal_vectors
    MRC     p15,0,R0,c1,c0,0           /* read core configuration register */
    BIC       R0,R0,#V_Bit             /* clear V bit to select normal exception vectors */
                                       /* at addresses from 0x00000000 to 0x0000001C */
    MCR     p15,0,R0,c1,c0,0           /* write modified value to core configuration register */


	# Initialize frame pointer
	mov	fp, #0

	# The GNU linker, correctly, expects the ELF loader to use the
	# physical addresses in the ELF file when loading the image. However,
	# many ELF loaders, erroneously, use the virtual addresses in the ELF
	# file when loading the image.
	# In most cases the virtual address will be the same as the physical
	# address for a section. However, they will be different when a
	# section is explicitly instructed in a linker command file to be
	# loaded at a physical address different from its virtual address, as
	# is done for initialized data in the OSE GCC linker command file.
	# When an ELF loader that uses virtual addresses instead of physical
	# addresses loads a GNU linked ELF file such sections will be loaded
	# to their virtual address instead of their physical address, which
	# is wrong.
	# In order to still support such ELF loaders we have to determine if
	# __DATA_ROM (physical address of initialized data) really contains
	# the initialized data. If not, we have to initially copy it from
	# __DATA_RAM (virtual address of initialized data). Note that this
	# copying is only done once when the ELF file is first loaded. After
	# a warm start everything will work as usual, i.e. __DATA_ROM will
	# contain the initialized data which will then be copied to
	# __DATA_RAM.
init_global_var:
	ldr	r1, =__DATA_ROM
	ldr	r2, =__DATA_RAM
	ldr	r3, =__DATA_END
	adr	r4, dummyconst
	ldr	r4, [r4]
	cmp	r1, r2
	beq	skip_data
	cmp	r2, r3
	beq	skip_data
	ldr	r5, [r1]
	cmp	r4, r5
	beq	skip_fix_data
loop_fix_data:
	ldrb	r6, [r2]
	strb	r6, [r1]
	add	r1, r1, #1
	add	r2, r2, #1
	cmp	r2, r3
	bne	loop_fix_data
	b	skip_data
skip_fix_data:

	# Copy initialized data from ROM to RAM
	ldr	r1, =__DATA_ROM
	ldr	r2, =__DATA_RAM
	ldr	r3, =__DATA_END
	cmp	r1, r2
	beq	skip_data
	cmp	r2, r3
	beq	skip_data
loop_data:
	ldrb	r4, [r1]
	strb	r4, [r2]
	add	r1, r1, #1
	add	r2, r2, #1
	cmp	r2, r3
	bne	loop_data
skip_data:

	# Clear uninitialized data
	ldr	r1, =__BSS_START
	ldr	r2, =__BSS_END
	cmp	r1, r2
	beq	skip_bss
	mov	r0, #0
loop_bss:
	strb	r0, [r1]
	add	r1, r1, #1
	cmp	r1, r2
	bne	loop_bss
skip_bss:
# was in .ifdef __CLIB_SUPPORT__
#.ifdef LONG_CALLS
	# Need to set up standard file handles
#	mov lr, pc
#	ldr pc, =initialise_monitor_handles

	# call _init function and arrange for _fini to be called at program exit
#	ldr	r0, .Lfini
#	mov lr, pc
#	ldr pc, =atexit
#	mov lr, pc
#	ldr pc, =_init
#.else
	# Need to set up standard file handles
	# was in bl	initialise_monitor_handles

	# call _init function and arrange for _fini to be called at program exit
	# was in ldr	r0, .Lfini
	# was in bl	atexit
	# was in bl	_init
#.endif
# was in .endif
	# Call _diag_ppb_start() with argc set to 0 and argv ignored
	mov	r0, #0
	mov	r1, #0
#.ifdef LONG_CALLS
#	mov lr, pc
#	ldr pc, =_diag_ppb_start
#	# Call exit() with the return value from _diag_ppb_start() as argument
#	mov lr, pc
#	ldr pc, =exit
#.else
	bl	_diag_ppb_start
	# Call exit() with the return value from _diag_ppb_start() as argument
	bl	exit
#.endif

	/* added for SWT14740 - core crashes executing invalid code after main() returns */
infinite_loop:
	b	infinite_loop

.ifdef __CLIB_SUPPORT__
.Lfini:
	.word	_fini
.endif

dummyconst:
	.word	0xCAFE4DAD

#############################################################################
#
# Dummy function called by main() function if GCC is used.
#
#############################################################################

	.globl	__gccmain
__gccmain:
	tst	lr, #1
	moveq	pc, lr
	.word	0xE12FFF1E	/* bx lr */

/* For CPU1 testing */
go_core1:
        ldr     r1, =0xC02FD600
        ldr     r2, =0x2B
        str     r2, [r1]
        b go_core1

# /******** History ********/
# $Log: init.S,v $
# Revision 1.3  2012/09/10 06:44:54  srane
# Code for ARM11 CPU1.
#
# Revision 1.2  2012/06/07 22:50:59  srane
# TDM external loopback, ECC memory test
#
# Revision 1.1  2012/05/31 06:36:53  srane
# Initial checkin.
#
#
# $Endlog$

