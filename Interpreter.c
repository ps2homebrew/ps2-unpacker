/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2003  Pcsx2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdio.h>
#include "InterTables.h"
#include "defines.h"
#include "R5900.h"
#include "memory.h"
#include "Debug.h"

static int branch2 = 0;
static unsigned long branchPC;

cpuRegisters cpuRegs;
u32 bounce_pc = 0;

u32 gp, stack, stack_size, args, root, heap_start, heap_size;

extern u8 trace;

void execI() {
	u32 pc = cpuRegs.pc;
	
	cpuRegs.pc+= 4;
	cpuRegs.cycle++;
	
	if (!(cpuRegs.cycle & 0xffffff)) {
	    printf("Still running, at %08X (%i cycles)\r", cpuRegs.pc - 4, cpuRegs.cycle);
	    fflush(stdout);
	}

	if (memRead32(pc, &cpuRegs.code) == -1) return;

	if (trace)
	    printf("%s\n", disR5900F(cpuRegs.code, pc));
	    
	Int_OpcodePrintTable[cpuRegs.code >> 26]();
}

void dumpasm(u32 pc) {
    u32 code;
    int i;
    
    for (i = -32; i <= 32; i += 4) {
	memRead32(pc + i, &code);
	printf("%08X %s\n", code, disR5900Fasm(code, pc + i));
    }
}

__inline void doBranch(u32 tar) {
	branch2 = cpuRegs.branch = 1;
	branchPC = tar;
	execI();
	cpuRegs.branch = 0;
	cpuRegs.pc = branchPC;
	
	if (has_write(branchPC)) {
		printf("\nExec hit written memory, branched to %08X\n", branchPC);
		bounce_pc = branchPC;
	}
}

void SPECIAL() {Int_SpecialPrintTable[_Funct_]();}
void REGIMM()  {Int_REGIMMPrintTable[_Rt_]();    }

void UnknownOpcode() {
	switch (cpuRegs.code) {
	default:
		printf("\nHit unknown/unsupported opcode %08X at %08X.\nPlease report.\n", cpuRegs.code, cpuRegs.pc - 4);
		dumpasm(cpuRegs.pc - 4);
		exit(-1);
	}
}

void voidOpcode() {
	printf("\nWarning: ignoring opcode %08X at %08X.\n", cpuRegs.code, cpuRegs.pc - 4);
	dumpasm(cpuRegs.pc - 4);
}

void COP0() {
	switch (cpuRegs.code) {
	case 0x40106000: /* mfc0 s0, Status, found in ps2_sbrk, for malloc */
		cpuRegs.GPR.n.s0.UL[0] = 0;
		return;
	case 0x42000038: /* EI */
	case 0x42000039: /* DI */
		return;
	default:
	        printf("\nHit COP0's instruction %08X at %08X.\nNo support for COP0 yet.\n", cpuRegs.code, cpuRegs.pc - 4);
		dumpasm(cpuRegs.pc - 4);
		exit(-1);
	}
}

/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/
void ADDI() 	{ if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0] + _Imm_; }// Rt = Rs + Im signed!!!!
void ADDIU()    { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0] + _Imm_; }// Rt = Rs + Im signed !!!
void DADDI()    { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0] + _Imm_; }// Rt = Rs + Im 
void DADDIU()   { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0] + _Imm_; }// Rt = Rs + Im 
void ANDI() 	{ if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0] & (s64)_ImmU_; }	// Rt = Rs And Im
void ORI()      { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0] | (s64)_ImmU_; }	// Rt = Rs Or  Im
void XORI() 	{ if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0] ^ (s64)_ImmU_; }	// Rt = Rs Xor Im
void SLTI()     { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0] < (s64)(_Imm_); } // Rt = Rs < Im (signed)
void SLTIU()    { if (!_Rt_) return; cpuRegs.GPR.r[_Rt_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0] < (u64)(_Imm_); } // Rt = Rs < Im (unsigned)

/*********************************************************
* Register arithmetic                                    *
* Format:  OP rd, rs, rt                                 *
*********************************************************/
void ADD()	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0]  + cpuRegs.GPR.r[_Rt_].SL[0];}	// Rd = Rs + Rt		(Exception on Integer Overflow)
void ADDU() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0]  + cpuRegs.GPR.r[_Rt_].SL[0];}	// Rd = Rs + Rt
void DADD()     { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0]  + cpuRegs.GPR.r[_Rt_].SD[0]; }
void DADDU()    { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0]  + cpuRegs.GPR.r[_Rt_].SD[0]; }
void SUB() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0]  - cpuRegs.GPR.r[_Rt_].SL[0];}	// Rd = Rs - Rt		(Exception on Integer Overflow)
void SUBU() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SL[0]  - cpuRegs.GPR.r[_Rt_].SL[0]; }	// Rd = Rs - Rt
void DSUB() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0]  - cpuRegs.GPR.r[_Rt_].SD[0];}	
void DSUBU() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0]  - cpuRegs.GPR.r[_Rt_].SD[0]; }
void AND() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0]  & cpuRegs.GPR.r[_Rt_].UD[0]; }	// Rd = Rs And Rt
void OR() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0]  | cpuRegs.GPR.r[_Rt_].UD[0]; }	// Rd = Rs Or  Rt
void XOR() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0]  ^ cpuRegs.GPR.r[_Rt_].UD[0]; }	// Rd = Rs Xor Rt
void NOR() 	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] =~(cpuRegs.GPR.r[_Rs_].UD[0] | cpuRegs.GPR.r[_Rt_].UD[0]); }// Rd = Rs Nor Rt
void SLT()	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].SD[0] < cpuRegs.GPR.r[_Rt_].SD[0]; }	// Rd = Rs < Rt (signed)
void SLTU()	{ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0] < cpuRegs.GPR.r[_Rt_].UD[0]; }	// Rd = Rs < Rt (unsigned)

/*********************************************************
* Jump to target                                         *
* Format:  OP target                                     *
*********************************************************/

void J()   {
	doBranch(_JumpTarget_);
}

void JAL() {
	_SetLink(31); doBranch(_JumpTarget_);
}

/*********************************************************
* Register jump                                          *
* Format:  OP rs, rd                                     *
*********************************************************/
void JR()   { 
	doBranch(cpuRegs.GPR.r[_Rs_].UL[0]); 
}

void JALR() { 
	u32 temp = cpuRegs.GPR.r[_Rs_].UL[0];

	if (_Rd_) { _SetLink(_Rd_); }
	doBranch(temp);
}


/*********************************************************
* Register mult/div & Register trap logic                *
* Format:  OP rs, rt                                     *
*********************************************************/
void DIV() {
    if (cpuRegs.GPR.r[_Rt_].SL[0] != 0) {
        cpuRegs.LO.SD[0] = cpuRegs.GPR.r[_Rs_].SL[0] / cpuRegs.GPR.r[_Rt_].SL[0];
        cpuRegs.HI.SD[0] = cpuRegs.GPR.r[_Rs_].SL[0] % cpuRegs.GPR.r[_Rt_].SL[0];
    }
}

void DIVU() {
	if (cpuRegs.GPR.r[_Rt_].UL[0] != 0) {
		cpuRegs.LO.SD[0] = cpuRegs.GPR.r[_Rs_].UL[0] / cpuRegs.GPR.r[_Rt_].UL[0];
		cpuRegs.HI.SD[0] = cpuRegs.GPR.r[_Rs_].UL[0] % cpuRegs.GPR.r[_Rt_].UL[0];
	}
}

void MULT() { //different in ps2...
	s64 res = (s64)cpuRegs.GPR.r[_Rs_].SL[0] * (s64)cpuRegs.GPR.r[_Rt_].SL[0];

	cpuRegs.LO.UD[0] = (s32)(res & 0xffffffff);
	cpuRegs.HI.UD[0] = (s32)(res >> 32);

	if (!_Rd_) return;
	cpuRegs.GPR.r[_Rd_].UD[0]= cpuRegs.LO.UD[0]; //that is the difference
}

void MULTU() { //different in ps2..
	u64 res = (u64)cpuRegs.GPR.r[_Rs_].UL[0] * (u64)cpuRegs.GPR.r[_Rt_].UL[0];

	cpuRegs.LO.UD[0] = (s32)(res & 0xffffffff);
	cpuRegs.HI.UD[0] = (s32)(res >> 32);

	if (!_Rd_) return;
	cpuRegs.GPR.r[_Rd_].UD[0]= cpuRegs.LO.UD[0]; //that is the difference
}

/*********************************************************
* Load higher 16 bits of the first word in GPR with imm  *
* Format:  OP rt, immediate                              *
*********************************************************/
void LUI()  { 
	if (!_Rt_) return; 
	cpuRegs.GPR.r[_Rt_].UD[0] = (s32)(cpuRegs.code << 16);
}

/*********************************************************
* Move from HI/LO to GPR                                 *
* Format:  OP rd                                         *
*********************************************************/
void MFHI() { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.HI.UD[0]; } // Rd = Hi
void MFLO() { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.LO.UD[0]; } // Rd = Lo

/*********************************************************
* Move to GPR to HI/LO & Register jump                   *
* Format:  OP rs                                         *
*********************************************************/
void MTHI() { cpuRegs.HI.UD[0] = cpuRegs.GPR.r[_Rs_].UD[0]; } // Hi = Rs
void MTLO() { cpuRegs.LO.UD[0] = cpuRegs.GPR.r[_Rs_].UD[0]; } // Lo = Rs


/*********************************************************
* Shift arithmetic with constant shift                   *
* Format:  OP rd, rt, sa                                 *
*********************************************************/
void SLL()   { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].UL[0] << _Sa_); } // Rd = Rt << sa
void DSLL()  { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] << _Sa_); }
void DSLL32(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] << (_Sa_+32));}
void SRA()   { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].SL[0] >> _Sa_); } // Rd = Rt >> sa (arithmetic)
void DSRA()  { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (u64)(cpuRegs.GPR.r[_Rt_].SD[0] >> _Sa_); }
void DSRA32(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (u64)(cpuRegs.GPR.r[_Rt_].SD[0] >> (_Sa_+32));}
void SRL()   { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].UL[0] >> _Sa_); } // Rd = Rt >> sa (logical)
void DSRL()  { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] >> _Sa_); }
void DSRL32(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] >> (_Sa_+32));}

/*********************************************************
* Shift arithmetic with variant register shift           *
* Format:  OP rd, rt, rs                                 *
*********************************************************/
void SLLV() { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].UL[0] << (cpuRegs.GPR.r[_Rs_].UL[0] &0x1f));} // Rd = Rt << rs
void SRAV() { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].SL[0] >> (cpuRegs.GPR.r[_Rs_].UL[0] &0x1f));} // Rd = Rt >> rs (arithmetic)
void SRLV() { if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s32)(cpuRegs.GPR.r[_Rt_].UL[0] >> (cpuRegs.GPR.r[_Rs_].UL[0] &0x1f));} // Rd = Rt >> rs (logical)
void DSLLV(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] << (cpuRegs.GPR.r[_Rs_].UL[0] &0x3f));}  
void DSRAV(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].SD[0] = (s64)(cpuRegs.GPR.r[_Rt_].SD[0] >> (cpuRegs.GPR.r[_Rs_].UL[0] &0x3f));}
void DSRLV(){ if (!_Rd_) return; cpuRegs.GPR.r[_Rd_].UD[0] = (u64)(cpuRegs.GPR.r[_Rt_].UD[0] >> (cpuRegs.GPR.r[_Rs_].UL[0] &0x3f));}

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, rt, offset                             *
*********************************************************/
#define RepBranchi32(op)      if(cpuRegs.GPR.r[_Rs_].SD[0] op cpuRegs.GPR.r[_Rt_].SD[0]) doBranch(_BranchTarget_);

void BEQ() {	RepBranchi32(==) }  // Branch if Rs == Rt
void BNE() {	RepBranchi32(!=) }  // Branch if Rs != Rt

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, offset                                 *
*********************************************************/
#define RepZBranchi32(op)      if(cpuRegs.GPR.r[_Rs_].SD[0] op 0) doBranch(_BranchTarget_);
#define RepZBranchLinki32(op)  { _SetLink(31); if(cpuRegs.GPR.r[_Rs_].SD[0] op 0) doBranch(_BranchTarget_); }

void BGEZ()   { RepZBranchi32(>=) }      // Branch if Rs >= 0
void BGEZAL() { RepZBranchLinki32(>=) }  // Branch if Rs >= 0 and link
void BGTZ()   { RepZBranchi32(>) }       // Branch if Rs >  0
void BLEZ()   { RepZBranchi32(<=) }      // Branch if Rs <= 0
void BLTZ()   { RepZBranchi32(<) }       // Branch if Rs <  0
void BLTZAL() { RepZBranchLinki32(<) }   // Branch if Rs <  0 and link


/*********************************************************
* Register branch logic  Likely                          *
* Format:  OP rs, offset                                 *
*********************************************************/
#define RepZBranchi32Likely(op)      if(cpuRegs.GPR.r[_Rs_].SD[0] op 0) { doBranch(_BranchTarget_); } else { cpuRegs.pc +=4; }
#define RepZBranchLinki32Likely(op)  { _SetLink(31); if(cpuRegs.GPR.r[_Rs_].SD[0] op 0) { doBranch(_BranchTarget_); } else { cpuRegs.pc +=4; } }
#define RepBranchi32Likely(op)       if(cpuRegs.GPR.r[_Rs_].SD[0] op cpuRegs.GPR.r[_Rt_].SD[0]) { doBranch(_BranchTarget_); } else { cpuRegs.pc +=4; }


void BEQL()    {  RepBranchi32Likely(==)      }  // Branch if Rs == Rt
void BNEL()    {  RepBranchi32Likely(!=)      }  // Branch if Rs != Rt
void BLEZL()   {  RepZBranchi32Likely(<=)     }  // Branch if Rs <= 0
void BGTZL()   {  RepZBranchi32Likely(>)      }  // Branch if Rs >  0
void BLTZL()   {  RepZBranchi32Likely(<)      }  // Branch if Rs <  0
void BGEZL()   {  RepZBranchi32Likely(>=)     }  // Branch if Rs >= 0
void BLTZALL() {  RepZBranchLinki32Likely(<)  }  // Branch if Rs <  0 and link
void BGEZALL() {  RepZBranchLinki32Likely(>=) }  // Branch if Rs >= 0 and link

/*********************************************************
* Load and store for GPR                                 *
* Format:  OP rt, offset(base)                           *
*********************************************************/

void LB() {
	u32 addr;
	u8  val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (memRead8(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (s8)val; 
}

void LBU() { 
	u32 addr;
	u8  val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (memRead8(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (u8)val; 
}

void LH() { 
	u32 addr;
	u16 val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (memRead16(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (s16)val; 
}

void LHU() { 
	u32 addr;
	u16 val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (memRead16(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (u16)val; 
}

void LW() {
	s32 addr;
	u32 val;

	/* Reworked UL to SL . asadr */
	addr = (s32)cpuRegs.GPR.r[_Rs_].SL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));

	if (memRead32(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (s64)(s32)val; 
}

void LWU() { 
	s32 addr;
	u32 val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (memRead32(addr, &val) == -1) return;
	if (_Rt_) cpuRegs.GPR.r[_Rt_].SD[0] = (u32)val; 
}

u32 LWL_MASK[4] = { 0xffffff, 0xffff, 0xff, 0 };
u32 LWL_SHIFT[4] = { 24, 16, 8, 0 };

void LWL() {
	s32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	u32 shift = addr & 3;
	u32 mem;

	if (!_Rt_) return;
	if (memRead32(addr & ~3, &mem) == -1) return;
	cpuRegs.GPR.r[_Rt_].UD[0] =	(cpuRegs.GPR.r[_Rt_].UL[0] & LWL_MASK[shift]) | 
								(mem << LWL_SHIFT[shift]);

	/*
	Mem = 1234.  Reg = abcd

	0   4bcd   (mem << 24) | (reg & 0x00ffffff)
	1   34cd   (mem << 16) | (reg & 0x0000ffff)
	2   234d   (mem <<  8) | (reg & 0x000000ff)
	3   1234   (mem      ) | (reg & 0x00000000)
	*/
}

u32 LWR_MASK[4] = { 0, 0xff000000, 0xffff0000, 0xffffff00 };
u32 LWR_SHIFT[4] = { 0, 8, 16, 24 };

void LWR() {
	s32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	u32 shift = addr & 3;
	u32 mem;

	if (!_Rt_) return;
	if (memRead32(addr & ~3, &mem) == -1) return;
	cpuRegs.GPR.r[_Rt_].UD[0] =	(cpuRegs.GPR.r[_Rt_].UL[0] & LWR_MASK[shift]) | 
								(mem >> LWR_SHIFT[shift]);

	/*
	Mem = 1234.  Reg = abcd

	0   1234   (mem      ) | (reg & 0x00000000)
	1   a123   (mem >>  8) | (reg & 0xff000000)
	2   ab12   (mem >> 16) | (reg & 0xffff0000)
	3   abc1   (mem >> 24) | (reg & 0xffffff00)
	*/
}

void LD() {
    s32 addr;
	u64 val;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;

	if (_Rt_)
		 memRead64(addr, &cpuRegs.GPR.r[_Rt_].UD[0]);
	else memRead64(addr, &val);
}

u64 LDL_MASK[8] = { 0x00ffffffffffffff, 0x0000ffffffffffff, 0x000000ffffffffff, 0x00000000ffffffff, 
					0x0000000000ffffff, 0x000000000000ffff, 0x00000000000000ff, 0x0000000000000000 };
u32 LDL_SHIFT[8] = { 56, 48, 40, 32, 24, 16, 8, 0 };

void LDL() {
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	u32 shift = addr & 7;
	u64 mem;

	if (!_Rt_) return;
	if (memRead64(addr & ~7, &mem) == -1) return;
	cpuRegs.GPR.r[_Rt_].UD[0] =	(cpuRegs.GPR.r[_Rt_].UD[0] & LDL_MASK[shift]) | 
								(mem << LDL_SHIFT[shift]);
}

u64 LDR_MASK[8] = { 0x0000000000000000, 0xff00000000000000, 0xffff000000000000, 0xffffff0000000000,
					0xffffffff00000000, 0xffffffffff000000, 0xffffffffffff0000, 0xffffffffffffff00 };
u32 LDR_SHIFT[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

void LDR() {  
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	u32 shift = addr & 7;
	u64 mem;

	if (!_Rt_) return;
	if (memRead64(addr & ~7, &mem) == -1) return;
	cpuRegs.GPR.r[_Rt_].UD[0] =	(cpuRegs.GPR.r[_Rt_].UD[0] & LDR_MASK[shift]) | 
								(mem >> LDR_SHIFT[shift]);
}

void LQ() {
	u32 addr;
	u64 val[2];

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	addr&=~0xf;

	if (_Rt_) {
		memRead128(addr, &cpuRegs.GPR.r[_Rt_].UD[0]);
	} else {
		memRead128(addr, val);
	}
}

void SB() { 
	u32 addr;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
    memWrite8(addr, cpuRegs.GPR.r[_Rt_].UC[0]); 
}

void SH() { 
	u32 addr;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + _Imm_;
	memWrite16(addr, cpuRegs.GPR.r[_Rt_].US[0]); 
}

void SW(){  
	s32 addr;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
    memWrite32(addr, cpuRegs.GPR.r[_Rt_].UL[0]); 
}

u32 SWL_MASK[4] = { 0xffffff00, 0xffff0000, 0xff000000, 0x00000000 };
u32 SWL_SHIFT[4] = { 24, 16, 8, 0 };

void SWL() {
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
	u32 shift = addr & 3;
	u32 mem;

	if (memRead32(addr & ~3, &mem) == -1) return;

	memWrite32(addr & ~3,  (cpuRegs.GPR.r[_Rt_].UL[0] >> SWL_SHIFT[shift]) |
		      (  mem & SWL_MASK[shift]) );
	/*
	Mem = 1234.  Reg = abcd

	0   123a   (reg >> 24) | (mem & 0xffffff00)
	1   12ab   (reg >> 16) | (mem & 0xffff0000)
	2   1abc   (reg >>  8) | (mem & 0xff000000)
	3   abcd   (reg      ) | (mem & 0x00000000)
	*/
}

u32 SWR_MASK[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
u32 SWR_SHIFT[4] = { 0, 8, 16, 24 };

void SWR() {
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
	u32 shift = addr & 3;
	u32 mem;

	if (memRead32(addr & ~3, &mem) == -1) return;

	memWrite32(addr & ~3,  (cpuRegs.GPR.r[_Rt_].UL[0] << SWR_SHIFT[shift]) |
		      ( mem & SWR_MASK[shift]) );

	/*
	Mem = 1234.  Reg = abcd

	0   abcd   (reg      ) | (mem & 0x00000000)
	1   bcd4   (reg <<  8) | (mem & 0x000000ff)
	2   cd34   (reg << 16) | (mem & 0x0000ffff)
	3   d234   (reg << 24) | (mem & 0x00ffffff)
	*/
}

void SD() {
	s32 addr;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
    memWrite64(addr,cpuRegs.GPR.r[_Rt_].UD[0]); 
}

u64 SDL_MASK[8] = { 0xffffffffffffff00, 0xffffffffffff0000, 0xffffffffff000000, 0xffffffff00000000, 
					0xffffff0000000000, 0xffff000000000000, 0xff00000000000000, 0x0000000000000000 };
u32 SDL_SHIFT[8] = { 56, 48, 40, 32, 24, 16, 8, 0 };

void SDL() {
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
	u32 shift = addr & 7;
	u64 mem;

	if (memRead64(addr & ~7, &mem) == -1) return;

	memWrite64(addr & ~7,  (cpuRegs.GPR.r[_Rt_].UD[0] >> SDL_SHIFT[shift]) |
		      ( mem & SDL_MASK[shift]) );
}

u64 SDR_MASK[8] = { 0x0000000000000000, 0x00000000000000ff, 0x000000000000ffff, 0x0000000000ffffff,
					0x00000000ffffffff, 0x000000ffffffffff, 0x0000ffffffffffff, 0x00ffffffffffffff };
u32 SDR_SHIFT[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

void SDR() {
	u32 addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
	u32 shift = addr & 7;
	u64 mem;

	if (memRead64(addr & ~7, &mem) == -1) return;

	memWrite64(addr & ~7,  (cpuRegs.GPR.r[_Rt_].UD[0] << SDR_SHIFT[shift]) |
		      ( mem & SDR_MASK[shift]) );
}

void SQ() {
	s32 addr;

	addr = cpuRegs.GPR.r[_Rs_].UL[0] + ((cpuRegs.code & 0x8000 ? 0xFFFF8000 : 0)| (cpuRegs.code & 0x7fff));
	addr&=~0xf;
	memWrite128(addr, &cpuRegs.GPR.r[_Rt_].UD[0]);
}

/*********************************************************
* Conditional Move                                       *
* Format:  OP rd, rs, rt                                 *
*********************************************************/

void MOVZ() {
	if (!_Rd_) return;
	if (cpuRegs.GPR.r[_Rt_].UD[0] == 0) {
		cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0];
	}
}
void MOVN() {
	if (!_Rd_) return;
	if (cpuRegs.GPR.r[_Rt_].UD[0] != 0) {
		cpuRegs.GPR.r[_Rd_].UD[0] = cpuRegs.GPR.r[_Rs_].UD[0];
	}
}

/*********************************************************
* Special purpose instructions                           *
* Format:  OP                                            *
*********************************************************/

void SYSCALL() {
	u8 call;
	if (cpuRegs.GPR.n.v1.SL[0] < 0)
		call = (u8)(-cpuRegs.GPR.n.v1.SL[0]);
	else
		call = cpuRegs.GPR.n.v1.UC[0];

	switch (call) {
	case 0x32: /* SleepThread */
		printf("\nSleepThread() called at %08X, not good.\nMay be worth reporting.\n", cpuRegs.pc - 4);
		dumpasm(cpuRegs.pc - 4);
		exit(-1);
	case 0x3c: /* RFU060, typical crt0 */
		gp         = cpuRegs.GPR.n.a0.UL[0];
		stack      = cpuRegs.GPR.n.a1.UL[0];
		stack_size = cpuRegs.GPR.n.a2.UL[0];
		args       = cpuRegs.GPR.n.a3.UL[0];
		root       = cpuRegs.GPR.n.t0.UL[0];
		printf("\nGot RFU060(%08X, %08X, %08X, %08X, %08X)\n", gp, stack, stack_size, args, root);
		return;
	case 0x3d: /* RFU061, typical crt0 */
	        heap_start = cpuRegs.GPR.n.a0.UL[0];
		heap_size  = cpuRegs.GPR.n.a1.UL[0];
		printf("\nGot RFU061(%08X, %08X)\n", heap_start, heap_size);
		return;
	case 0x3e: /* End Of Heap */
		cpuRegs.GPR.n.v0.UL[0] = 32 * 1024 * 1024; /* let's put it at the end of the memory... */
		printf("\nGot EndOfHeap. Returning %08X\n", cpuRegs.GPR.n.v0.UL[0]);
		return;
	case 0x64: /* Flush cache, typical crt0 */
		return;
	case 0x07: /* ExecPS2 */
		printf("\nHit ExecPS2 at %08X. A0 = %08X\n", cpuRegs.pc - 4, cpuRegs.GPR.n.a0.UL[0]);
		bounce_pc = cpuRegs.GPR.n.a0.UL[0];
		return;
        default:
		printf("\nHit unknown/unsupported syscall %02X at %08X.\nPlease report.\n", call, cpuRegs.pc - 4);
		exit(-1);
        }
}

void BREAK() {
	printf("\nHit break %08X at %08X.\n", cpuRegs.code, cpuRegs.pc - 4);
	exit(-1);
}
