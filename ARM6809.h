//
//  ARM6809.h
//  ARM6809
//
//  Created by Fredrik Ahlström on 2008-07-14.
//  Copyright © 2008-2023 Fredrik Ahlström. All rights reserved.
//

#ifndef ARM6809_HEADER
#define ARM6809_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u32 Opz[256];
	u32 ReadTbl[8];
	u32 WriteTbl[8];
	u32 MemTbl[8];
//	u32 Regs[8]
	u32 RegF;	// Also DP
	u32 RegA;
	u32 RegB;
	u32 RegX;
	u32 RegY;
	u32 RegCY;
	u32 RegPC;
	u32 RegSP;
	u32 US;
	u8 PendingIrqs;
	u8 NmiPin;
	u8 Padding[2];

	void *LastBank;
} ARM6809Core;


void m6809Init(ARM6809Core *cpu);

void m6809Reset(ARM6809Core *cpu);

/**
 * Saves the state of the cpu to the destination.
 * @param  *destination: Where to save the state.
 * @param  *cpu: The ARM6809 cpu to save.
 * @return The size of the state.
 */
int m6809SaveState(void *destination, const ARM6809Core *cpu);

/**
 * Loads the state of the cpu from the source.
 * @param  *cpu: The ARM6809 cpu to load a state into.
 * @param  *source: Where to load the state from.
 * @return The size of the state.
 */
int m6809LoadState(ARM6809Core *cpu, const void *source);

/**
 * Gets the state size of an ARM6809 cpu.
 * @return The size of the state.
 */
int m6809GetStateSize(void);

/**
 * Patch an opcode to a new function.
 * @param  *cpu: The ARM6809Core cpu to patch.
 * @param  opcode: Which opcode to redirect.
 * @param  *function: Pointer to new function .
 */
void m6809PatchOpcode(ARM6809Core *cpu, int opcode, void *function);

/**
 * Restore a previously patched opcode.
 * @param  *cpu: The ARM6809Core cpu to patch.
 * @param  opcode: Which opcode to restore.
 */
void m6809RestoreOpcode(ARM6809Core *cpu, int opcode);

void m6809RestoreAndRunXCycles(int cycles);
void m6809RunXCycles(int cycles);
void m6809SetNMIPin(bool state);
void m6809SetFIRQPin(bool state);
void m6809SetIRQPin(bool state);
void m6809CheckIrqs(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ARM6809_HEADER
