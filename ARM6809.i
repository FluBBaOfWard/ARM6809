//
//  ARM6809.i
//  ARM6809
//
//  Created by Fredrik Ahlström on 2008-07-14.
//  Copyright © 2008-2023 Fredrik Ahlström. All rights reserved.
//

				;@ r0,r1,r2=temp regs
	m6809f		.req r3			;@ Bits 0-7=f, 8-23=0, 24-31=DP.
	m6809dp		.req r3			;@ Bits 0-7=f, 8-23=0, 24-31=DP.
	m6809a		.req r4			;@ Bits 0-15=0, bits 16-23=B, bits 24-31=A
	m6809b		.req r5			;@ Bits 0-15=0, bits 16-23=B, bits 24-31=A
	m6809x		.req r6			;@ Bits 0-15=0
	m6809y		.req r7			;@ Bits 0-15=0
	cycles		.req r8
	m6809pc		.req r9
	m6809ptr	.req r10
	m6809sp		.req r11		;@ Bits 0-15=0
	addy		.req r12		;@ Keep this at r12 (scratch for APCS)

;@----------------------------------------------------------------------------

	.struct 0					;@ Changes section so make sure it's set before real code.
m6809Opz:			.space 256*4
m6809ReadTbl:		.space 8*4	;@ $0000-FFFF
m6809WriteTbl:		.space 8*4	;@ $0000-FFFF
m6809MemTbl:		.space 8*4	;@ $0000-FFFF
m6809StateStart:
m6809Regs:
m6809RegF:
m6809RegDP:			.long 0
m6809RegA:			.long 0
m6809RegB:			.long 0
m6809RegX:			.long 0
m6809RegY:			.long 0
m6809RegCY:			.long 0
m6809RegPC:			.long 0
m6809RegSP:			.long 0
m6809US:			.long 0
m6809PendingIrqs:	.byte 0		;@ (interrupt flags)
m6809NmiPin:		.byte 0
m6809Padding:		.space 2
m6809StateEnd:

m6809LastBank:		.long 0		;@ Last memmap added to PC (used to calculate current PC)
m6809End:
m6809Size:

m6809StateSize = m6809StateEnd-m6809StateStart

;@----------------------------------------------------------------------------
