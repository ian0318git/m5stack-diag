# $Id: jump_start.S,v 1.2 2017/07/28 07:58:51 harrchan Exp $
# $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/src/jump_start.S,v $
#------------------------------------------------------------------
# jump_start.S
#
# Mar 2012, Smita Rane
#
# Copyright (c) 2012-2017 by Cisco Systems, Inc.
# All rights reserved.
#
#------------------------------------------------------------------
#

##* COPYRIGHT-ENEA-SRC-R2 *
##*************************************************************************
## Copyright (C) 2004-2010 by Enea Software AB.
## All rights reserved.
##
## This Software is furnished under a software license agreement and
## may be used only in accordance with the terms of such agreement.
## Any other use or reproduction is prohibited. No title to and
## ownership of the Software is hereby transferred.
##
## PROPRIETARY NOTICE
## This Software consists of confidential information.
## Trade secret law and copyright law protect this Software.
## The above notice of copyright on this Software does not indicate
## any actual or intended publication of such Software.
##*************************************************************************
##* COPYRIGHT-END *

# ARM11MP_SINGLE_CORE build option is defined here and in bsp_config.h.
# It should be defined when dual-ARM support is not required.
##define ARM11MP_SINGLE_CORE

#############################################################################
#
# ARM startup code for application.
#
#############################################################################
.extern       bsp_mmu_l1_table
	
	.text
	.code	32
	.align	0

#############################################################################
#
# Start function linked to application start address.
#
#############################################################################
# Now some standard definitions...
.equ Mode_SVC,  0x13
.equ Mode_Bits, 0x1F

    .globl      _errrtn
    .globl      _fiqrtn
    .globl      _irqrtn

	.globl	_start
#.section .first
.section .text._start
        .code   32
        .align  0

_start:
        B       Reset_Handler
        B       Undefined_Handler
        B       SWI_Handler
        B       Prefetch_Handler
        B       Abort_Handler
        # Reserved vector 
        NOP
        B       IRQ_Handler
        B       FIQ_Handler
# The following handlers do not do anything useful in this example.

.section .text._start
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
                B       .

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
                B       .

IRQ_Handler:
        # make R0-R3 available for use locally
        SUBS    R14, R14, #4
        STMFD  R13!, {R0-R3,R12,R14}
        mov             lr, pc
        ldr     pc, =irqrtn
        LDMFD  R13!, {R0-R3,R12,R15}^

FIQ_Handler:
        # make R0-R3 available for use locally
        SUBS   R14,   R14,   #4
        STMFD  R13!, {R0-R3,R14}
        mov             lr, pc
        ldr     pc, =fiqrtn
        LDMFD  R13!, {R0-R3,R15}^

Error:  # ROUT
        # make R0-R3 available for use locally
        SUBS    R14, R14, #4
        STMFD  R13!, {R0-R3,R12,R14}
        mov             lr, pc
        ldr     pc, =errrtn
        # Now make sure it returns in supervisor mode
        MRS    R0,   SPSR
        BIC    R0,   R0, #Mode_Bits
        ORR    R0,   R0, #Mode_SVC
        MSR    SPSR_c, R0
        LDMFD  R13!, {R0-R3,R12,R15}^

###############################
#.section .text._start
Reset_Handler:
	# Make sure that we are in supervisor mode and with IRQ disabled
	msr cpsr_c, #0x93
        # Disable CPU timer
	ldr     r1, =__KRN_TIMER_CTRL_REG
	mov     r0, #0
	strb    r0, [r1]

	# Initialize stack pointer
	#ldr	r1, =__SP_INIT 
	# FIXME? how do we handle this for multiple cores?
	#mov 	sp, r1
	
	# Set stck == 0 for all modes but svc
	#cps #1
	#mov sp, #0
	#cps #2
	#mov sp, #0
	#cps #7
	#mov sp, #0
	#cps #0xB
	#mov sp, #0
	#cps #0xF
	#mov sp, #0
	#cps #0x3
 
	# Setup a default 64 kB stack limit in case the code has been
	# compiled with -mapcs-stack-check.
	#sub	sl, sp, #64 << 10

	# Initialize frame pointer
	#mov	fp, #0  @ null frame pointer
	#mov	r7, #0  @ null frame pointer for thumb

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

	# invalidate entire instruction cache
	mrc p15, 0, r0, c1, c0, 0
	mcr p15, 0, r0, c7, c5, 0

/*
	# check if coreid != 0 skip some initializations that core0 will do for core1
	mrc p15, 0, r0, c0, c0, 5
	ands r0, r0, #0xF
	bne not_core0 

#ifndef ARM11MP_SINGLE_CORE
	# This will copy SRAM2 to SRAM3 (local data and bss from core0 to core1)
	ldr     r1, =__SRAM_2_START
	ldr     r2, =__SRAM_2_END
	# Jump over if equal
	cmp     r1, r2
	beq     normal_boot

	add     r2, r2, #3
	bic     r2, r2, #3
	add     r3, r1, #0x20000
copy_loop:	
	ldr     r4, [r1]
	add     r1, r1, #4
	str     r4, [r3]
	add     r3, r3, #4
		
	cmp     r1, r2
	bne     copy_loop
#endif
*/

normal_boot:	
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
		

	# Call main() with argc set to 0 and argv ignored
	mov	r0, #0
	mov	r1, #0
#ifdef LONG_CALLS
	mov lr, pc
	ldr pc, =_diag_ppb_start
#else
	bl	_diag_ppb_start
#endif
	# Call exit() with the return value from main() as argument
	#bl	exit

dummyconst:
	.word	0xDEADBABE

					
#############################################################################
#
# Dummy function called by main() function if GCC is used.
#
#############################################################################

	.globl	__gccmain
.section .text.__gccmain
__gccmain:
	tst	lr, #1
	moveq	pc, lr
	.word	0xE12FFF1E	/* bx lr */


#Setup L1 page table
#Set L2 page table

not_core0:
	/* Ok, we are core 1 */
	ldr r1, =bsp_mmu_l1_table
	ldr r1, [r1, #4]
	/* Set new (hopefully initialized L1 table) */
	mcr p15, 0, r1, c2, c0, 0
	
	#Set us as manager of domain 0
	mov r1, #3
	mcr p15, 0, r1, c3, c0, 0
	
	mrc p15, 0, r1, c1, c0, 0
	/* Clear cache bits */
	bic r1, r1, #0x4
	bic r1, r1, #0x1000
	/* enable XP bit! */
        mov r0, #1	
        mov r0, r0, LSL #23
        /* enable MMU! */
        add r0, r0, #0x1 
	orr r1, r1, r0
	mcr p15, 0,  r1, c1, c0, 0
	
	/* resume boot */
	b normal_boot

# /******** History ********/
# $Log: jump_start.S,v $
# Revision 1.2  2017/07/28 07:58:51  harrchan
# Collapse Oakenshield-branch to Main Trunk.
#
# Revision 1.1.2.1  2017/06/29 08:14:39  harrchan
# Initial commit code for Oakenshield
#
# Revision 1.1  2012/04/18 09:44:02  srane
# Initial checkin
#
# 
# $Endlog$


