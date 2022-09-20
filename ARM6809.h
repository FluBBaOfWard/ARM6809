//
//  ARM6809.h
//  ARM6809
//
//  Created by Fredrik Ahlström on 2008-07-14.
//  Copyright © 2008-2022 Fredrik Ahlström. All rights reserved.
//

#ifndef ARM6809_HEADER
#define ARM6809_HEADER

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u32 m6809Opz[256];
	u32 m6809ReadTbl[8];
	u32 m6809WriteTbl[8];
	u32 m6809MemTbl[8];
// m6809Regs[8]
	u32 m6809RegF;	// Also DP
	u32 m6809RegA;
	u32 m6809RegB;
	u32 m6809RegX;
	u32 m6809RegY;
	u32 m6809RegCY;
	u32 m6809RegPC;
	u32 m6809RegSP;
	u32 m6809US;
	u8 m6809PendingIrqs;
	u8 m6809NmiPin;
	u8 m6809Padding[2];

	void *m6809LastBank;
	int m6809OldCycles;
	void *m6809NextTimeout_;
	void *m6809NextTimeout;
} ARM6809Core;


extern ARM6809Core m6809OpTable;

void m6809Reset(int type);

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

void m6809SetNMIPin(bool state);
void m6809SetFIRQPin(bool state);
void m6809SetIRQPin(bool state);
void m6809RestoreAndRunXCycles(int cycles);
void m6809RunXCycles(int cycles);
void m6809CheckIrqs(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ARM6809_HEADER
