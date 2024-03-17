//
//  ARM6809_mac.h
//  ARM6809
//
//  Created by Fredrik Ahlström on 2008-07-14.
//  Copyright © 2008-2024 Fredrik Ahlström. All rights reserved.
//

#include "ARM6809.i"
							;@ ARM flags
	.equ PSR_N, 0x00000008		;@ Negative
	.equ PSR_Z, 0x00000004		;@ Zero
	.equ PSR_C, 0x00000002		;@ Carry
	.equ PSR_V, 0x00000001		;@ Overflow

	.equ PSR_E, 0x00000080		;@ E Flag
	.equ PSR_F, 0x00000040		;@ Inhibit Fast Interrupt
	.equ PSR_H, 0x00000020		;@ Half carry
	.equ PSR_I, 0x00000010		;@ Inhibit Interrupt


							;@ M6809 flags
	.equ EF, 0x80				;@ E, flag that shows if all registers was pushed to the stack.
	.equ FF, 0x40				;@ Inhibit Fast Interrupt
	.equ HF, 0x20				;@ Half carry
	.equ IF, 0x10				;@ Inhibit Interrupt
	.equ NF, 0x08				;@ Negative
	.equ ZF, 0x04				;@ Zero
	.equ VF, 0x02				;@ Overflow
	.equ CF, 0x01				;@ Carry

;@----------------------------------------------------------------------------
	.equ CYC_SHIFT, 8
	.equ CYCLE, 1<<CYC_SHIFT	;@ One cycle
	.equ CYC_MASK, CYCLE-1		;@ Mask
;@----------------------------------------------------------------------------
;@ cycle flags- (stored in cycles reg for speed)

	.equ CYC_C,		0x01		;@ Carry bit
	.equ CYC_SYNC,	0x02		;@ Is Sync active?
	.equ CYC_CWAI,	0x04		;@ Is CWAI active?
	.equ CYC_II,	0x08		;@ IRQ mask
	.equ CYC_V,		0x40		;@ Overflow bit
;@----------------------------------------------------------------------------


	.macro loadLastBank reg
	ldr \reg,[m6809ptr,#m6809LastBank]
	.endm

	.macro storeLastBank reg
	str \reg,[m6809ptr,#m6809LastBank]
	.endm

	.macro encodePC				;@ Translate m6809pc from M6809 PC to rom offset
	and r1,m6809pc,#0xE000
	add r2,m6809ptr,#m6809MemTbl
	ldr r1,[r2,r1,lsr#11]
	storeLastBank r1
	add m6809pc,m6809pc,r1
	.endm

	.macro reEncodePC			;@ Translate m6809pc from M6809 PC to rom offset
	loadLastBank r0
	sub m6809pc,m6809pc,r0
	encodePC
	.endm

	.macro decodeFLG			;@ Unpack M6809 flags from r0, C & V changes place
	ands r1,r0,#VF|CF
	teqne r1,#VF|CF
	eorne r0,r0,#VF|CF
	.endm


	.macro fetch count
	subs cycles,cycles,#(\count)*CYCLE
#ifndef KONAMI6809
	ldrbpl r0,[m6809pc],#1
	ldrpl pc,[m6809ptr,r0,lsl#2]
#else
	bpl decodeOpCode
#endif
	b m6809OutOfCycles
	.endm


	.macro fetchForce
#ifndef KONAMI6809
	ldrb r0,[m6809pc],#1
	ldr pc,[m6809ptr,r0,lsl#2]
#else
	b decodeOpCode
#endif
	.endm


	.macro eatCycles count
	sub cycles,cycles,#(\count)*CYCLE
	.endm

	.macro clearCycles
	and cycles,cycles,#CYC_MASK	;@ Save CPU bits
	.endm

	.macro getAdrDir
	ldrb addy,[m6809pc],#1
	orr addy,addy,m6809dp,lsr#16
	.endm

	.macro getAdrIdx
	bl fetchIdxAdr
	mov addy,addy,lsr#16
	.endm

	.macro getAdrExt
	ldrb r1,[m6809pc],#1
	ldrb addy,[m6809pc],#1
	orr addy,addy,r1,lsl#8
	.endm

	.macro readMem8
	and r1,addy,#0xE000
	add r2,m6809ptr,#m6809ReadTbl
	adr lr,0f
	ldr pc,[r2,r1,lsr#11]	;@ In: addy,r1=addy&0xE000 (for rom_R)
0:							;@ Out: r0=val (bits 8-31=0 ), addy preserved for 16bit read/write
	.endm

	.macro readMem8NoLr
	and r1,addy,#0xE000
	add r2,m6809ptr,#m6809ReadTbl
	ldr pc,[r2,r1,lsr#11]	;@ In: addy,r1=addy&0xE000 (for rom_R)
							;@ Out: r0=val (bits 8-31=0 ), addy preserved for 16bit read/write
	.endm

	.macro readMemIM8
	ldrb r0,[m6809pc],#1
	.endm

	.macro readMemDP8
	bl readFromDP8
	.endm

	.macro readMemIDX8
	getAdrIdx
	readMem8
	.endm

	.macro readMemEXT8
	getAdrExt
	readMem8
	.endm

	.macro readMem16
	readMem8
	stmfd sp!,{r0}
	add addy,addy,#1
	readMem8
	ldmfd sp!,{r1}
	orr r0,r0,r1,lsl#8
	.endm

	.macro readMem16NoLr
	stmfd sp!,{lr}
	readMem8
	stmfd sp!,{r0}
	add addy,addy,#1
	readMem8
	ldmfd sp!,{r1,lr}
	orr r0,r0,r1,lsl#8
	.endm

	.macro readMemIM16
	ldrb r0,[m6809pc],#1
	ldrb addy,[m6809pc],#1
	orr r0,addy,r0,lsl#8
	.endm

	.macro readMemIM16S
	ldrsb r0,[m6809pc],#1
	ldrb addy,[m6809pc],#1
	orr r0,addy,r0,lsl#8
	.endm

	.macro readMemDP16
	bl readFromDP16
	.endm

	.macro readMemIDX16
	getAdrIdx
	readMem16
	.endm

	.macro readMemEXT16
	getAdrExt
	readMem16
	.endm

	.macro writeMem8
	and r1,addy,#0xE000
	add r2,m6809ptr,#m6809WriteTbl
	adr lr,0f
	ldr pc,[r2,r1,lsr#11]	;@ In: addy,r0=val(bits 8-31=?)
0:							;@ Out: r0,r1,r2,addy=?
	.endm

	.macro writeMem8NoLr
	and r1,addy,#0xE000
	add r2,m6809ptr,#m6809WriteTbl
	ldr pc,[r2,r1,lsr#11]	;@ In: addy,r0=val(bits 8-31=?)
							;@ Out: r0,r1,r2,addy=?
	.endm

	.macro writeMemDP8
	bl writeToDP8
	.endm

	.macro writeMemIDX8
	getAdrIdx
	writeMem8
	.endm

	.macro writeMemEXT8
	getAdrExt
	writeMem8
	.endm

	.macro writeMem16
	stmfd sp!,{r0}
	mov r0,r0,lsr#8
	writeMem8
	add addy,addy,#1
	ldmfd sp!,{r0}
	writeMem8
	.endm

	.macro writeMem16NoLr
	stmfd sp!,{r0,lr}
	mov r0,r0,lsr#8
	writeMem8
	add addy,addy,#1
	ldmfd sp!,{r0,lr}
	writeMem8NoLr
	.endm

	.macro writeMemDP16
	bl writeToDP16
	.endm

;@	MACRO
;@	writeMemIDX16_
;@	getAdrIdx
;@	writeMem16 r0
;@	.endm

;@----------------------------------------------------------------------------

	.macro opABX
	add m6809x,m6809x,m6809b,lsr#8
	.endm
;@---------------------------------------

	.macro opADC reg
	tst m6809f,m6809f,lsr#2		;@ Get C
	subcs r0,r0,#0x100
	eor r1,r0,\reg,lsr#24		;@ Prepare for check of half carry
	adcs \reg,\reg,r0,ror#8
	mrs r0,cpsr					;@ N,Z,V&C
	bic m6809f,m6809f,#0x2F		;@ Clear HNZVC
	orr m6809f,m6809f,r0,lsr#28
	eor r1,r1,\reg,lsr#24
	tst r1,#0x10				;@ H
	orrne m6809f,m6809f,#PSR_H
	.endm
;@---------------------------------------

	.macro opADD reg
	bic m6809f,m6809f,#0x2F		;@ Clear HNZVC
	mov r1,\reg,lsl#4			;@ Prepare for check of half carry
	cmn r1,r0,lsl#28
	orrcs m6809f,m6809f,#PSR_H
	adds \reg,\reg,r0,lsl#24
	mrs r0,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r0,lsr#28
	.endm

	.macro opADDD
	bic m6809f,m6809f,#0x0F			;@ Clear NZVC
	orr m6809a,m6809a,m6809b,lsr#8
	adds m6809a,m6809a,r0,lsl#16
	mrs r0,cpsr						;@ N,Z,V&C
	orr m6809f,m6809f,r0,lsr#28
	mov m6809b,m6809a,lsl#8
	and m6809a,m6809a,#0xFF000000
	.endm
;@---------------------------------------

	.macro opAND reg
	ands \reg,\reg,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm
;@---------------------------------------

	.macro opASRR reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_C	;@ Clear NZC
	movs \reg,\reg,asr#25
	orrmi m6809f,m6809f,#PSR_N
	orreq m6809f,m6809f,#PSR_Z
	orrcs m6809f,m6809f,#PSR_C
	mov \reg,\reg,lsl#24
	.endm

	.macro opASR
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_C	;@ Clear NZC
	ands r1,r0,#0x80
	orrne m6809f,m6809f,#PSR_N
	orrs r0,r1,r0,lsr#1
	orreq m6809f,m6809f,#PSR_Z
	orrcs m6809f,m6809f,#PSR_C
	.endm
;@---------------------------------------

	.macro opBIT reg
	tst \reg,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm
;@---------------------------------------

	.macro opBEQ_END
	ldrsb r0,[m6809pc],#1
	addeq m6809pc,m6809pc,r0
	fetch 3
	.endm

	.macro opBNE_END
	ldrsb r0,[m6809pc],#1
	addne m6809pc,m6809pc,r0
	fetch 3
	.endm

	.macro opBRANCH_EQ x
	tst m6809f,#(\x)
	opBEQ_END
	.endm

	.macro opBRANCH_NE x
	tst m6809f,#(\x)
	opBNE_END
	.endm

	.macro opLBEQ_END
	readMemIM16S
	addeq m6809pc,m6809pc,r0
	subeq cycles,cycles,#CYCLE
	fetch 4
	.endm

	.macro opLBNE_END
	readMemIM16S
	addne m6809pc,m6809pc,r0
	subne cycles,cycles,#CYCLE
	fetch 4
	.endm

	.macro opLBRANCH_EQ x
	tst m6809f,#(\x)
	opLBEQ_END
	.endm

	.macro opLBRANCH_NE x
	tst m6809f,#(\x)
	opLBNE_END
	.endm
;@---------------------------------------

	.macro opCLRR reg
	mov \reg,#0
	bic m6809f,m6809f,#PSR_N+PSR_V+PSR_C
	orr m6809f,m6809f,#PSR_Z
	.endm

	.macro opCLR
	opCLRR r0
	.endm
;@---------------------------------------

	.macro opCMP reg
	bic m6809f,m6809f,#0x2F		;@ Clear HNZVC
	mov r1,\reg,lsl#4 			;@ Prepare for check of half carry
	cmp \reg,r0,lsl#24
	mrs r2,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r2,lsr#28
	eor m6809f,m6809f,#PSR_C	;@ Invert C
	cmp r1,r0,lsl#28
	orrcc m6809f,m6809f,#PSR_H	;@ ???
	.endm

	.macro opCMP16 reg
	bic m6809f,m6809f,#0x0F		;@ Clear NZVC
	cmp \reg,r0,lsl#16
	mrs r2,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r2,lsr#28
	eor m6809f,m6809f,#PSR_C	;@ Invert C
	.endm

	.macro opCMPD
	orr r1,m6809a,m6809b,lsr#8
	opCMP16 r1
	.endm

	.macro opCMPU
	ldr r1,[m6809ptr,#m6809US]
	opCMP16 r1
	.endm
;@---------------------------------------

	.macro opCOMR reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V
	orr m6809f,m6809f,#PSR_C
	eors \reg,\reg,#0xFF000000
	orreq m6809f,m6809f,#PSR_Z
	orrmi m6809f,m6809f,#PSR_N
	.endm

	.macro opCOM
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V
	orr m6809f,m6809f,#PSR_C
	eor r0,r0,#0xFF
	movs r1,r0,lsl#24
	orreq m6809f,m6809f,#PSR_Z
	orrmi m6809f,m6809f,#PSR_N
	.endm
;@---------------------------------------

	.macro opDECR reg				;@ For A & B
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V
	subs \reg,\reg,#0x01000000
	orrmi m6809f,m6809f,#PSR_N
	orreq m6809f,m6809f,#PSR_Z
	orrvs m6809f,m6809f,#PSR_V
	.endm

	.macro opDEC					;@ For memory
	mov r0,r0,lsl#24
	opDECR r0
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opEOR reg
	eors \reg,\reg,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm
;@---------------------------------------

	.macro opINCR reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	adds \reg,\reg,#0x01000000
	orrmi m6809f,m6809f,#PSR_N
	orreq m6809f,m6809f,#PSR_Z
	orrvs m6809f,m6809f,#PSR_V
	.endm

	.macro opINC
	mov r0,r0,lsl#24
	opINCR r0
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opJMPEA
	and r1,addy,#0xE0000000
	add r2,m6809ptr,#m6809MemTbl
	ldr r1,[r2,r1,lsr#27]
	storeLastBank r1
	add m6809pc,r1,addy,lsr#16
	.endm
;@---------------------------------------

	.macro opLD8 reg
	movs \reg,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm

	.macro opLD16 reg
	movs \reg,r0,lsl#16
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V
	orrmi m6809f,m6809f,#PSR_N
	orreq m6809f,m6809f,#PSR_Z
	.endm

	.macro opLDU
	opLD16 r0
	str r0,[m6809ptr,#m6809US]
	.endm
;@---------------------------------------

	.macro opLSLR reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V+PSR_C	;@ Clear NZVC
	adds \reg,\reg,\reg
	mrs r1,cpsr								;@ N,Z,V&C
	orr m6809f,m6809f,r1,lsr#28
	.endm

	.macro opLSL
	mov r0,r0,lsl#24
	opLSLR r0
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opLSR
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V+PSR_C	;@ Clear NZVC
	movs r0,r0,lsr#1
	orreq m6809f,m6809f,#PSR_Z
	orrcs m6809f,m6809f,#PSR_C
	.endm

	.macro opLSRR reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V+PSR_C	;@ Clear NZVC
	movs \reg,\reg,lsr#25
	mov \reg,\reg,lsl#24
	orreq m6809f,m6809f,#PSR_Z
	orrcs m6809f,m6809f,#PSR_C
	.endm
;@---------------------------------------

	.macro opNEGR reg
	bic m6809f,m6809f,#0x0F		;@ Clear NZVC
	rsbs \reg,\reg,#0
	mrs r1,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r1,lsr#28
	eor m6809f,m6809f,#PSR_C	;@ Invert C
	.endm

	.macro opNEG
	mov r0,r0,lsl#24
	opNEGR r0
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opOR reg
	orrs \reg,\reg,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm
;@---------------------------------------

	.macro opROLR reg
	tst m6809f,#PSR_C
	orrne \reg,\reg,#0x00800000
	adds \reg,\reg,\reg
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V+PSR_C	;@ Clear NZVC
	mrs r1,cpsr						;@ N,Z,V&C
	orr m6809f,m6809f,r1,lsr#28
	.endm

	.macro opROL
	mov r0,r0,lsl#24
	opROLR r0
	mov r0,r0,lsr#24
	.endm
;@---------------------------------------

	.macro opRORR reg
	tst m6809f,#PSR_C
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_C	;@ Clear NZC
	orrne \reg,\reg,#1
	orrne m6809f,m6809f,#PSR_N
	movs \reg,\reg,ror#25
	orrcs m6809f,m6809f,#PSR_C
	movs \reg,\reg,lsl#24
	orreq m6809f,m6809f,#PSR_Z
	.endm

	.macro opROR
	tst m6809f,#PSR_C
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_C	;@ Clear NZC
	orrne r0,r0,#0x100
	orrne m6809f,m6809f,#PSR_N
	movs r0,r0,lsr#1
	orreq m6809f,m6809f,#PSR_Z
	orrcs m6809f,m6809f,#PSR_C
	.endm
;@---------------------------------------

	.macro opSBC reg
	and r1,m6809f,#PSR_C			;@ Get C
	subs r0,r0,r1,lsl#7				;@ Fix up r0, invert C.
	sbcs \reg,\reg,r0,ror#8
	mrs r0,cpsr
	bic m6809f,m6809f,#0x0F			;@ Clear NZVC
	orr m6809f,m6809f,r0,lsr#28
	eor m6809f,m6809f,#PSR_C		;@ Invert C
	.endm
;@---------------------------------------

	.macro opST8 reg
	mov r0,\reg,lsr#24
	tst \reg,#0xFF000000
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	writeMem8
	.endm

	.macro opST16 reg
	mov r0,\reg,lsr#16
	movs \reg,r0,lsl#16
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V
	orrmi m6809f,m6809f,#PSR_N
	orreq m6809f,m6809f,#PSR_Z
	writeMem16
	.endm

	.macro opSTD
	orr r1,m6809a,m6809b,lsr#8
	opST16 r1
	.endm

	.macro opSTU
	ldr r1,[m6809ptr,#m6809US]
	opST16 r1
	.endm
;@---------------------------------------

	.macro opSUB reg
	bic m6809f,m6809f,#0x0F		;@ Clear NZVC
	subs \reg,\reg,r0,lsl#24
	mrs r2,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r2,lsr#28
	eor m6809f,m6809f,#PSR_C	;@ Invert C
	.endm

	.macro opSUBD
	orr r2,m6809a,m6809b,lsr#8
	bic m6809f,m6809f,#0x0F		;@ Clear NZVC
	subs r2,r2,r0,lsl#16
	and m6809a,r2,#0xFF000000
	mov m6809b,r2,lsl#8
	mrs r2,cpsr					;@ N,Z,V&C
	orr m6809f,m6809f,r2,lsr#28
	eor m6809f,m6809f,#PSR_C	;@ Invert C
	.endm
;@---------------------------------------

	.macro opTSTR reg
	tst \reg,#0xFF000000
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm

	.macro opTST
	movs r0,r0,lsl#24
	bic m6809f,m6809f,#PSR_N+PSR_Z+PSR_V	;@ Clear NZV
	orrmi m6809f,m6809f,#PSR_N				;@ Set PSR_N
	orreq m6809f,m6809f,#PSR_Z				;@ Set PSR_Z
	.endm
;@---------------------------------------
