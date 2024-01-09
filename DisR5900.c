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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Debug.h"
#include "R5900.h"

long jumpMode;
char ostr[1024];

// Names of registers
char *disRNameGPR[] = {
	"r0", "at", "v0", "v1", "a0", "a1","a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5","t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5","s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp","fp", "ra"};

char *disRNameCP0[] = {
	"Index"     , "Random"    , "EntryLo0" , "EntryLo1", "Context" , "PageMask"  , "Wired"     , "*RES*",
	"BadVAddr"  , "Count"     , "EntryHi"  , "Compare" , "Status"  , "Cause"     , "ExceptPC"  , "PRevID",
	"Config"    , "LLAddr"    , "WatchLo"  , "WatchHi" , "*RES*"   , "*RES*"     , "*RES*"     , "Debug",
	"DEPC"      , "PerfCnt"   , "ErrCtl"   , "CacheErr", "TagLo"   , "TagHi"     , "ErrorEPC"  , "DESAVE"};

char *disRNameCP1[] = {
	"FPR0" , "FPR1" , "FPR2" , "FPR3" , "FPR4" , "FPR5" , "FPR6" , "FPR7",
	"FPR8" , "FPR9" , "FPR10", "FPR11", "FPR12", "FPR13", "FPR14", "FPR15",
	"FPR16", "FPR17", "FPR18", "FPR19", "FPR20", "FPR21", "FPR22", "FPR23",
	"FPR24", "FPR25", "FPR26", "FPR27", "FPR28", "FPR29", "FPR30", "FPR31"};

char *disRNameCP1c[] = {
	"FRevID", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*",
	"*RES*",  "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*",
	"*RES*",  "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*",
	"*RES*",  "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "*RES*", "FStatus"};

char *disRNameCP2f[] = {
	"VF00", "VF01", "VF02", "VF03", "VF04", "VF05", "VF06", "VF07",
	"VF08", "VF09", "VF10", "VF11", "VF12", "VF13", "VF14", "VF15",
	"VF16", "VF17", "VF18", "VF19", "VF20", "VF21", "VF22", "VF23",
	"VF24", "VF25", "VF26", "VF27", "VF28", "VF29", "VF30", "VF31"};

char *disRNameCP2i[] = {
	"VI00",   "VI01",  "VI02", "VI03",   "VI04",  "VI05",     "VI06",  "VI07",
	"VI08",   "VI09",  "VI10", "VI11",   "VI12",  "VI13",     "VI14",  "VI15",
	"Status", "MAC",   "Clip", "*RES*",  "R",     "I",        "Q",     "*RES*",
	"*RES*",  "*RES*", "TPC",  "CMSAR0", "FBRST", "VPU-STAT", "*RES*", "CMSAR1"};

char *CP2VFnames[] = { "x", "y", "z", "w" };

// Type deffinition of our functions
#define DisFInterface  (u32 code, u32 pc)
#define DisFInterfaceT (u32, u32)
#define DisFInterfaceN (code, pc)

typedef char* (*TdisR5900F)DisFInterface;

// These macros are used to assemble the disassembler functions
#define MakeDisF(fn, b) \
	char* fn DisFInterface { \
		sprintf (ostr, "%8.8lx %8.8lx:", pc, code); \
		b; /*ostr[(strlen(ostr) - 1)] = 0;*/ return ostr; \
	}

#undef _Target_
#undef _Branch_
#undef _Funct_
#undef _Rd_
#undef _Rt_
#undef _Rs_
#undef _Sa_
#undef _Im_

#define _Funct_  ((code      ) & 0x3F) // The funct part of the instruction register 
#define _Rd_     ((code >> 11) & 0x1F) // The rd part of the instruction register 
#define _Rt_     ((code >> 16) & 0x1F) // The rt part of the instruction register 
#define _Rs_     ((code >> 21) & 0x1F) // The rs part of the instruction register 
#define _Sa_     ((code >>  6) & 0x1F) // The sa part of the instruction register
#define _Im_     ( code & 0xFFFF)      // The immediate part of the instruction register


#define _rRs_   cpuRegs.GPR.r[_Rs_].UL[1], cpuRegs.GPR.r[_Rs_].UL[0]   // Rs register
#define _rRt_   cpuRegs.GPR.r[_Rt_].UL[1], cpuRegs.GPR.r[_Rt_].UL[0]   // Rt register
#define _rRd_   cpuRegs.GPR.r[_Rd_].UL[1], cpuRegs.GPR.r[_Rd_].UL[0]   // Rd register
#define _rSa_   cpuRegs.GPR.r[_Sa_].UL[1], cpuRegs.GPR.r[_Sa_].UL[0]   // Sa register

#define _rFs_   cpuRegs.CP0.r[_Rd_]   // Fs register

#define _rRs32_   cpuRegs.GPR.r[_Rs_].UL[0]   // Rs register
#define _rRt32_   cpuRegs.GPR.r[_Rt_].UL[0]   // Rt register
#define _rRd32_   cpuRegs.GPR.r[_Rd_].UL[0]   // Rd register
#define _rSa32_   cpuRegs.GPR.r[_Sa_].UL[0]   // Sa register


#define _nRs_ _rRs_, disRNameGPR[_Rs_]
#define _nRt_ _rRt_, disRNameGPR[_Rt_]
#define _nRd_ _rRd_, disRNameGPR[_Rd_]
#define _nSa_ _rSa_, disRNameGPR[_Sa_]
#define _nRd0_ _rFs_, disRNameCP0[_Rd_]

#define _nRs32_ _rRs32_, disRNameGPR[_Rs_]
#define _nRt32_ _rRt32_, disRNameGPR[_Rt_]
#define _nRd32_ _rRd32_, disRNameGPR[_Rd_]
#define _nSa32_ _rSa32_, disRNameGPR[_Sa_]

#define _I_       _Im_, _Im_
#define _Target_  ((pc & 0xf0000000) + ((code & 0x03ffffff) * 4))
#define _Branch_  (pc + 4 + ((short)_Im_ * 4))
#define _OfB_     _Im_, _nRs_

#define _Fsf_ ((code >> 21) & 0x03)
#define _Ftf_ ((code >> 23) & 0x03)

#define dName(i)	sprintf(ostr, "%s %-7s,", ostr, i)
#define dGPR128(i)	sprintf(ostr, "%s %8.8lx_%8.8lx_%8.8lx_%8.8lx (%s),", ostr, cpuRegs.GPR.r[i].UL[3], cpuRegs.GPR.r[i].UL[2], cpuRegs.GPR.r[i].UL[1], cpuRegs.GPR.r[i].UL[0], disRNameGPR[i])
#define dGPR64(i)	sprintf(ostr, "%s %8.8lx_%8.8lx (%s),", ostr, cpuRegs.GPR.r[i].UL[1], cpuRegs.GPR.r[i].UL[0], disRNameGPR[i])
#define dGPR64U(i)	sprintf(ostr, "%s %8.8lx_%8.8lx (%s),", ostr, cpuRegs.GPR.r[i].UL[3], cpuRegs.GPR.r[i].UL[2], disRNameGPR[i])
#define dGPR32(i)	sprintf(ostr, "%s %8.8lx (%s),", ostr, cpuRegs.GPR.r[i].UL[0], disRNameGPR[i])

#define dCP032(i)	sprintf(ostr, "%s %8.8lx (%s),", ostr, cpuRegs.CP0.r[i], disRNameCP0[i])

#define dCP132(i)	sprintf(ostr, "%s %f (%s),", ostr, fpuRegs.fpr[i].f, disRNameCP1[i])
#define dCP1c32(i)	sprintf(ostr, "%s %8.8lx (%s),", ostr, fpuRegs.fprc[i], disRNameCP1c[i])
#define dCP1acc()	sprintf(ostr, "%s %f (ACC),", ostr, fpuRegs.ACC.f)

#define dHI64()		sprintf(ostr, "%s %8.8lx_%8.8lx (%s),", ostr, cpuRegs.HI.UL[1], cpuRegs.HI.UL[0], "hi")
#define dLO64()		sprintf(ostr, "%s %8.8lx_%8.8lx (%s),", ostr, cpuRegs.LO.UL[1], cpuRegs.LO.UL[0], "lo")
#define dImm()		sprintf(ostr, "%s %4.4lx (%ld),", ostr, _Im_, _Im_)
#define dTarget()	sprintf(ostr, "%s %8.8lx,", ostr, _Target_)
#define dSa()		sprintf(ostr, "%s %2.2lx (%ld),", ostr, _Sa_, _Sa_)
#define dSa32()		sprintf(ostr, "%s %2.2lx (%ld),", ostr, _Sa_+32, _Sa_+32)
#define dOfB()		sprintf(ostr, "%s %4.4lx (%8.8lx (%s)),", ostr, _Im_, cpuRegs.GPR.r[_Rs_].UL[0], disRNameGPR[_Rs_])
#define dOffset()	sprintf(ostr, "%s %8.8lx,", ostr, _Branch_)
#define dCode()		sprintf(ostr, "%s %8.8lx,", ostr, (code >> 6) & 0xffffff)
#define dSaR()		sprintf(ostr, "%s %8.8lx,", ostr, cpuRegs.sa)

typedef struct {
	u32 addr;
	char name[32];
} sSymbol;

static sSymbol *dSyms = NULL;
static int nSyms = 0;

void disR5900AddSym(u32 addr, char *name) {
	dSyms = (sSymbol*)realloc(dSyms, sizeof(sSymbol) * (nSyms+1));
	if (dSyms == NULL) return;
	dSyms[nSyms].addr = addr;
	strncpy(dSyms[nSyms].name, name, 32);
	nSyms++;
}

void disR5900FreeSyms() {
	if (dSyms != NULL) { free(dSyms); dSyms = NULL; }
	nSyms = 0;
}

char *disR5900GetSym(u32 addr) {
	int i;

	if (dSyms == NULL) return NULL;
	for (i=0; i<nSyms; i++)
		if (dSyms[i].addr == addr) return dSyms[i].name;

	return NULL;
}

char *disR5900GetUpperSym(u32 addr) {
	u32 laddr;
	int i, j=-1;

	if (dSyms == NULL) return NULL;
	for (i=0, laddr=0; i<nSyms; i++) {
		if (dSyms[i].addr < addr && dSyms[i].addr > laddr) {
			laddr = dSyms[i].addr;
			j = i;
		}
	}
	if (j == -1) return NULL;
	return dSyms[j].name;
}

#define dFindSym(i) { \
	char *str = disR5900GetSym(i); \
	if (str != NULL) sprintf(ostr, "%s %s", ostr, str); \
}

/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/
MakeDisF(disADDI,		dName("ADDI");   dGPR64(_Rt_); dGPR32(_Rs_); dImm();)
MakeDisF(disADDIU,		dName("ADDIU");  dGPR64(_Rt_); dGPR32(_Rs_); dImm();)
MakeDisF(disANDI,		dName("ANDI");   dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disORI,		dName("ORI");    dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disSLTI,		dName("SLTI");   dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disSLTIU,		dName("SLTIU");  dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disXORI,		dName("XORI");   dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disDADDI,      dName("DADDI");  dGPR64(_Rt_); dGPR64(_Rs_); dImm();)
MakeDisF(disDADDIU,     dName("DADDIU"); dGPR64(_Rt_); dGPR64(_Rs_); dImm();)

/*********************************************************
* Register arithmetic                                    *
* Format:  OP rd, rs, rt                                 *
*********************************************************/
MakeDisF(disADD,		dName("ADD");   dGPR64(_Rd_); dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disADDU,		dName("ADDU");  dGPR64(_Rd_); dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disDADD,		dName("DADD");  dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disDADDU,		dName("DADDU"); dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disSUB,		dName("SUB");   dGPR64(_Rd_); dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disSUBU,		dName("SUBU");  dGPR64(_Rd_); dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disDSUB,		dName("DSUB");  dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disDSUBU,		dName("DSDBU"); dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disAND,		dName("AND");   dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disOR,		    dName("OR");    dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disXOR,		dName("XOR");   dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disNOR,		dName("NOR");   dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disSLT,		dName("SLT");   dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);) 
MakeDisF(disSLTU,		dName("SLTU");  dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)

/*********************************************************
* Jump to target                                         *
* Format:  OP target                                     *
*********************************************************/
MakeDisF(disJ,			dName("J");   dTarget(); dFindSym(_Target_);)
MakeDisF(disJAL,		dName("JAL"); dTarget(); dGPR32(31); dFindSym(_Target_);)

/*********************************************************
* Register jump                                          *
* Format:  OP rs, rd                                     *
*********************************************************/
MakeDisF(disJR,			dName("JR");   dGPR32(_Rs_); dFindSym(cpuRegs.GPR.r[_Rs_].UL[0]);)
MakeDisF(disJALR,		dName("JALR"); dGPR32(_Rs_); dGPR32(_Rd_); dFindSym(cpuRegs.GPR.r[_Rs_].UL[0]);)

/*********************************************************
* Register mult/div & Register trap logic                *
* Format:  OP rs, rt                                     *
*********************************************************/
MakeDisF(disDIV,		dName("DIV");   dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disDIVU,		dName("DIVU");  dGPR32(_Rs_); dGPR32(_Rt_);)
MakeDisF(disMULT,		dName("MULT");  dGPR32(_Rs_); dGPR32(_Rt_); dGPR32(_Rd_);)
MakeDisF(disMULTU,		dName("MULTU"); dGPR32(_Rs_); dGPR32(_Rt_); dGPR32(_Rd_);)

/*********************************************************
* Load higher 16 bits of the first word in GPR with imm  *
* Format:  OP rt, immediate                              *
*********************************************************/
MakeDisF(disLUI,		dName("LUI"); dGPR64(_Rt_); dImm();)

/*********************************************************
* Move from HI/LO to GPR                                 *
* Format:  OP rd                                         *
*********************************************************/
MakeDisF(disMFHI,		dName("MFHI"); dGPR64(_Rd_); dHI64();)
MakeDisF(disMFLO,		dName("MFLO"); dGPR64(_Rd_); dLO64();)

/*********************************************************
* Move to GPR to HI/LO & Register jump                   *
* Format:  OP rs                                         *
*********************************************************/
MakeDisF(disMTHI,		dName("MTHI"); dHI64(); dGPR64(_Rs_);)
MakeDisF(disMTLO,		dName("MTLO"); dLO64(); dGPR64(_Rs_);)

/*********************************************************
* Shift arithmetic with constant shift                   *
* Format:  OP rd, rt, sa                                 *
*********************************************************/
MakeDisF(disSLL,		if (code) { dName("SLL");   dGPR64(_Rd_); dGPR32(_Rt_); dSa(); } else { dName("NOP"); })
MakeDisF(disDSLL,		dName("DSLL");   dGPR64(_Rd_); dGPR64(_Rt_); dSa();)
MakeDisF(disDSLL32,		dName("DSLL32"); dGPR64(_Rd_); dGPR64(_Rt_); dSa32();)
MakeDisF(disSRA,		dName("SRA");    dGPR64(_Rd_); dGPR32(_Rt_); dSa();)
MakeDisF(disDSRA,		dName("DSRA");   dGPR64(_Rd_); dGPR64(_Rt_); dSa();)
MakeDisF(disDSRA32,		dName("DSRA32"); dGPR64(_Rd_); dGPR64(_Rt_); dSa32();)
MakeDisF(disSRL,		dName("SRL");    dGPR64(_Rd_); dGPR32(_Rt_); dSa();)
MakeDisF(disDSRL,		dName("DSRL");   dGPR64(_Rd_); dGPR64(_Rt_); dSa();)
MakeDisF(disDSRL32,		dName("DSRL32"); dGPR64(_Rd_); dGPR64(_Rt_); dSa32();)

/*********************************************************
* Shift arithmetic with variant register shift           *
* Format:  OP rd, rt, rs                                 *
*********************************************************/
MakeDisF(disSLLV,		dName("SLLV");  dGPR64(_Rd_); dGPR32(_Rt_); dGPR32(_Rs_);)
MakeDisF(disDSLLV,		dName("DSLLV"); dGPR64(_Rd_); dGPR64(_Rt_); dGPR32(_Rs_);)
MakeDisF(disSRAV,		dName("SRAV");  dGPR64(_Rd_); dGPR32(_Rt_); dGPR32(_Rs_);)
MakeDisF(disDSRAV,		dName("DSRAV"); dGPR64(_Rd_); dGPR64(_Rt_); dGPR32(_Rs_);)
MakeDisF(disSRLV,		dName("SRLV");  dGPR64(_Rd_); dGPR32(_Rt_); dGPR32(_Rs_);)
MakeDisF(disDSRLV,		dName("DSRLV"); dGPR64(_Rd_); dGPR64(_Rt_); dGPR32(_Rs_);)

/*********************************************************
* Load and store for GPR                                 *
* Format:  OP rt, offset(base)                           *
*********************************************************/
MakeDisF(disLB,			dName("LB");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disLBU,		dName("LBU"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLH,			dName("LH");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disLHU,		dName("LHU"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLW,			dName("LW");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disLWU,		dName("LWU"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLWL,		dName("LWL"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLWR,		dName("LWR"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLD,			dName("LD");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disLDL,		dName("LDL"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLDR,		dName("LDR"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disLQ,			dName("LQ");  dGPR128(_Rt_); dOfB();)
MakeDisF(disSB,			dName("SB");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disSH,			dName("SH");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disSW,			dName("SW");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disSWL,		dName("SWL"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disSWR,		dName("SWR"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disSD,			dName("SD");  dGPR64(_Rt_);  dOfB();)
MakeDisF(disSDL,		dName("SDL"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disSDR,		dName("SDR"); dGPR64(_Rt_);  dOfB();)
MakeDisF(disSQ,			dName("SQ");  dGPR128(_Rt_); dOfB();)

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, rt, offset                             *
*********************************************************/
MakeDisF(disBEQ,		dName("BEQ"); dGPR64(_Rs_); dGPR64(_Rt_); dOffset();)
MakeDisF(disBNE,		dName("BNE"); dGPR64(_Rs_); dGPR64(_Rt_); dOffset();)

/*********************************************************
* Moves between GPR and COPx                             *
* Format:  OP rt, rd                                     *
*********************************************************/
MakeDisF(disMFC0,		dName("MFC0"); dGPR32(_Rt_); dCP032(_Rd_);)
MakeDisF(disMTC0,		dName("MTC0"); dCP032(_Rd_); dGPR32(_Rt_);)

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, offset                                 *
*********************************************************/

MakeDisF(disBGEZ, 		dName("BGEZ");   dGPR64(_Rs_); dOffset();)
MakeDisF(disBGEZAL, 	dName("BGEZAL"); dGPR64(_Rs_); dOffset();)
MakeDisF(disBGTZ, 		dName("BGTZ");   dGPR64(_Rs_); dOffset();)   
MakeDisF(disBLEZ, 		dName("BLEZ");   dGPR64(_Rs_); dOffset();)  
MakeDisF(disBLTZ, 		dName("BLTZ");   dGPR64(_Rs_); dOffset();)    
MakeDisF(disBLTZAL,     dName("BLTZAL"); dGPR64(_Rs_); dOffset();) 


/*********************************************************
* Register branch logic  Likely                          *
* Format:  OP rs, offset                                 *
*********************************************************/


MakeDisF(disBEQL, 		dName("BEQL");      dGPR64(_Rs_); dGPR64(_Rt_); dOffset();)  
MakeDisF(disBNEL, 		dName("BNEL");      dGPR64(_Rs_); dGPR64(_Rt_); dOffset();)   
MakeDisF(disBLEZL, 		dName("BLEZL");     dGPR64(_Rs_); dOffset();)  
MakeDisF(disBGTZL, 		dName("BGTZL");     dGPR64(_Rs_); dOffset();) 
MakeDisF(disBLTZL, 		dName("BLTZL");     dGPR64(_Rs_); dOffset();) 
MakeDisF(disBGEZL, 		dName("BGEZL");     dGPR64(_Rs_); dOffset();)  
MakeDisF(disBLTZALL,    dName("BLTZALL");   dGPR64(_Rs_); dOffset();)
MakeDisF(disBGEZALL, 	dName("BGEZALL");   dGPR64(_Rs_); dOffset();)


/*********************************************************
*   COP0 opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disBC0F,       dName("BC0F");   dOffset();)
MakeDisF(disBC0T,       dName("BC0T");   dOffset();)
MakeDisF(disBC0FL,      dName("BC0FL");  dOffset();)
MakeDisF(disBC0TL,      dName("BC0TL");  dOffset();)

MakeDisF(disTLBR,		dName("TLBR");)
MakeDisF(disTLBWI,		dName("TLBWI");)
MakeDisF(disTLBWR,		dName("TLBWR");)
MakeDisF(disTLBP,		dName("TLBP");)
MakeDisF(disERET,		dName("ERET");)
MakeDisF(disEI,			dName("EI");)
MakeDisF(disDI,			dName("DI");)

/*********************************************************
*   COP1 opcodes                                         *
*                                                        *
*********************************************************/

#define _Ft_ _Rt_
#define _Fs_ _Rd_
#define _Fd_ _Sa_

MakeDisF(disMFC1,		dName("MFC1"); dGPR64(_Rt_); dCP132(_Fs_);)
MakeDisF(disCFC1,		dName("CFC1"); dGPR64(_Rt_); dCP1c32(_Fs_);)
MakeDisF(disMTC1,		dName("MTC1"); dCP132(_Fs_); dGPR64(_Rt_);)
MakeDisF(disCTC1,		dName("CTC1"); dCP1c32(_Fs_); dGPR64(_Rt_);)

MakeDisF(disBC1F,		dName("BC1F");)
MakeDisF(disBC1T,		dName("BC1T");)
MakeDisF(disBC1FL,		dName("BC1FL");)
MakeDisF(disBC1TL,		dName("BC1TL");)

MakeDisF(disADDs,		dName("ADDs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disSUBs,		dName("SUBs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMULs,		dName("MULs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disDIVs,		dName("DIVs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disSQRTs,		dName("SQRTs");  dCP132(_Fd_); dCP132(_Ft_);)
MakeDisF(disABSs,		dName("ABSs");   dCP132(_Fd_); dCP132(_Fs_);)
MakeDisF(disMOVs,		dName("MOVs");   dCP132(_Fd_); dCP132(_Fs_);)
MakeDisF(disNEGs,		dName("NEGs");   dCP132(_Fd_); dCP132(_Fs_);)
MakeDisF(disRSQRTs,		dName("RSQRTs"); dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disADDAs,		dName("ADDAs");  dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disSUBAs,		dName("SUBAs");  dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMULAs,		dName("MULAs");  dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMADDs,		dName("MADDs");  dCP132(_Fd_); dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMSUBs,		dName("MSUBs");  dCP132(_Fd_); dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMADDAs,		dName("MADDAs"); dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMSUBAs,		dName("MSUBAs"); dCP1acc();    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disCVTWs,		dName("CVTWs");  dCP132(_Fd_); dCP132(_Fs_);)
MakeDisF(disMAXs,		dName("MAXs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disMINs,		dName("MINs");   dCP132(_Fd_); dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disCFs,		dName("CFs");    dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disCEQs,		dName("CEQs");   dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disCLTs,		dName("CLTs");   dCP132(_Fs_); dCP132(_Ft_);)
MakeDisF(disCLEs,		dName("CLEs");   dCP132(_Fs_); dCP132(_Ft_);)

MakeDisF(disCVTSw,		dName("CVTSw"); dCP132(_Fd_); dCP132(_Fs_);)

/*********************************************************
* Load and store for COP1                                *
* Format:  OP rt, offset(base)                           *
*********************************************************/

MakeDisF(disLWC1,		dName("LWC1"); dCP132(_Rt_); dOffset();)
MakeDisF(disSWC1,		dName("SWC1"); dCP132(_Rt_); dOffset();)

/*********************************************************
* Conditional Move                                       *
* Format:  OP rd, rs, rt                                 *
*********************************************************/

MakeDisF(disMOVZ,		dName("MOVZ"); dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disMOVN,		dName("MOVN"); dGPR64(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)

/*********************************************************
*   MMI opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disMULT1,		dName("MULT1");)
MakeDisF(disMULTU1,		dName("MULTU1");)

/*********************************************************
*   MMI0 opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disPADDW,		dName("PADDW");)
MakeDisF(disPADDH,		dName("PADDH");)
MakeDisF(disPADDB,		dName("PADDB");)

MakeDisF(disPADDSW,		dName("PADDSW");)
MakeDisF(disPADDSH,		dName("PADDSH");)
MakeDisF(disPADDSB,		dName("PADDSB");)

MakeDisF(disPSUBW,		dName("PSUBW");)
MakeDisF(disPSUBH,		dName("PSUBH");)
MakeDisF(disPSUBB,		dName("PSUBB");)

MakeDisF(disPSUBSW,		dName("PSUBSW");)
MakeDisF(disPSUBSH,		dName("PSUBSH");)
MakeDisF(disPSUBSB,		dName("PSUBSB");)

MakeDisF(disPCGTW,		dName("PCGTW");)
MakeDisF(disPCGTH,		dName("PCGTH");)
MakeDisF(disPCGTB,		dName("PCGTB");)

MakeDisF(disPMAXW,		dName("PMAXW");)
MakeDisF(disPMAXH,		dName("PMAXH");)

MakeDisF(disPEXTLW,		dName("PEXTLW"); dGPR128(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disPEXTLH,		dName("PEXTLH"); dGPR128(_Rd_); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disPEXTLB,		dName("PEXTLB");)
MakeDisF(disPEXTS,		dName("PEXTS");)

MakeDisF(disPPACW,		dName("PPACW");)
MakeDisF(disPPACH,		dName("PPACH");)
MakeDisF(disPPACB,		dName("PPACB");)
MakeDisF(disPPACS,		dName("PPACS");)

/*********************************************************
*   MMI1 opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disPADSBH,		dName("PADSBH");)

MakeDisF(disPABSW,		dName("PABSW");)
MakeDisF(disPABSH,		dName("PABSH");)

MakeDisF(disPCEQW,		dName("PCEQW");)
MakeDisF(disPCEQH,		dName("PCEQH");)
MakeDisF(disPCEQB,		dName("PCEQB");)

MakeDisF(disPMINW,		dName("PMINW");)
MakeDisF(disPMINH,		dName("PMINH");)

MakeDisF(disPADDUW,		dName("PADDUW");)
MakeDisF(disPADDUH,		dName("PADDUH");)
MakeDisF(disPADDUB,		dName("PADDUB");)

MakeDisF(disPSUBUW,		dName("PSUBUW");)
MakeDisF(disPSUBUH,		dName("PSUBUH");)
MakeDisF(disPSUBUB,		dName("PSUBUB");)

MakeDisF(disPEXTUW,		dName("PEXTUW"); dGPR128(_Rd_); dGPR64U(_Rs_); dGPR64U(_Rt_);)
MakeDisF(disPEXTUH,		dName("PEXTUH"); dGPR128(_Rd_); dGPR64U(_Rs_); dGPR64U(_Rt_);)
MakeDisF(disPEXTUB,		dName("PEXTUB");)

MakeDisF(disQFSRV,		dName("QFSRV");)

/*********************************************************
*   MMI2 opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disPMADDW,		dName("PMADDW");)
MakeDisF(disPMADDH,		dName("PMADDH");)

MakeDisF(disPSLLVW,		dName("PSLLVW");)
MakeDisF(disPSRLVW,		dName("PSRLVW");)

MakeDisF(disPMFHI,		dName("PMFHI");)
MakeDisF(disPMFLO,		dName("PMFLO");)

MakeDisF(disPINTH,		dName("PINTH");)

MakeDisF(disPMULTW,		dName("PMULTW");)
MakeDisF(disPMULTH,		dName("PMULTH");)

MakeDisF(disPDIVW,		dName("PDIVW");)
MakeDisF(disPDIVH,		dName("PDIVH");)

MakeDisF(disPCPYLD,		dName("PCPYLD"); dGPR128(_Rd_); dGPR128(_Rs_); dGPR128(_Rt_);)

MakeDisF(disPAND,		dName("PAND"); dGPR128(_Rd_); dGPR128(_Rs_); dGPR128(_Rt_);)
MakeDisF(disPXOR,		dName("PXOR"); dGPR128(_Rd_); dGPR128(_Rs_); dGPR128(_Rt_);)

MakeDisF(disPMSUBW,		dName("PMSUBW");)
MakeDisF(disPMSUBH,		dName("PMSUBH");)

MakeDisF(disPHMADH,		dName("PHMADH");)
MakeDisF(disPHMSBH,		dName("PHMSBH");)

MakeDisF(disPEXEW,		dName("PEXEW");)
MakeDisF(disPEXEH,		dName("PEXEH");)

MakeDisF(disPREVH,		dName("PREVH");)

MakeDisF(disPDIVBW,		dName("PDIVBW");)

MakeDisF(disPROT3W,		dName("PROT3W");)

/*********************************************************
*   MMI3 opcodes                                         *
*                                                        *
*********************************************************/

MakeDisF(disPMADDUW,	dName("PMADDUW");)

MakeDisF(disPSRAVW,		dName("PSRAVW");)

MakeDisF(disPMTHI,		dName("PMTHI");)
MakeDisF(disPMTLO,		dName("PMTLO");)

MakeDisF(disPINTEH,		dName("PINTEH");)

MakeDisF(disPMULTUW,	dName("PMULTUW");)
MakeDisF(disPDIVUW,		dName("PDIVUW");)

MakeDisF(disPCPYUD,		dName("PCPYUD"); dGPR128(_Rd_); dGPR128(_Rt_); dGPR128(_Rs_);)

MakeDisF(disPOR,		dName("POR"); dGPR128(_Rd_); dGPR128(_Rs_); dGPR128(_Rt_);)
MakeDisF(disPNOR,		dName("PNOR"); dGPR128(_Rd_); dGPR128(_Rs_); dGPR128(_Rt_);)

MakeDisF(disPEXCH,		dName("PEXCH");)
MakeDisF(disPEXCW,		dName("PEXCW");)

MakeDisF(disPCPYH,		dName("PCPYH"); dGPR128(_Rd_); dGPR128(_Rt_);)

/*********************************************************
* Special purpose instructions                           *
* Format:  OP                                            *
*********************************************************/

MakeDisF(disSYNC,		dName("SYNC");)  
MakeDisF(disBREAK,		dName("BREAK");) 
MakeDisF(disSYSCALL,	dName("SYSCALL"); dCode();)
MakeDisF(disCACHE,		sprintf(ostr, "%s %-7s, %lx,", ostr, "CACHE", _Rt_); dOfB();)
MakeDisF(disPREF,		dName("PREF");) 

MakeDisF(disMFSA,		dName("MFSA"); dGPR64(_Rd_); dSaR();)   
MakeDisF(disMTSA,		dName("MTSA"); dGPR64(_Rs_); dSaR();)   

MakeDisF(disMTSAB,      dName("MTSAB");dGPR64(_Rs_); dImm();)
MakeDisF(disMTSAH,      dName("MTSAH");dGPR64(_Rs_); dImm();)

MakeDisF(disTGE,	    dName("TGE");  dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disTGEU,	    dName("TGEU"); dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disTLT,	    dName("TLT");  dGPR64(_Rs_); dGPR64(_Rt_);)
MakeDisF(disTLTU,	    dName("TLTU"); dGPR64(_Rs_); dGPR64(_Rt_);) 
MakeDisF(disTEQ,		dName("TEQ");  dGPR64(_Rs_); dGPR64(_Rt_);) 
MakeDisF(disTNE,	    dName("TNE");  dGPR64(_Rs_); dGPR64(_Rt_);)

MakeDisF(disTGEI,	    dName("TGEI");  dGPR64(_Rs_); dImm();)
MakeDisF(disTGEIU,	    dName("TGEIU"); dGPR64(_Rs_); dImm();) 
MakeDisF(disTLTI,	    dName("TLTI");  dGPR64(_Rs_); dImm();) 
MakeDisF(disTLTIU,	    dName("TLTIU"); dGPR64(_Rs_); dImm();)  
MakeDisF(disTEQI,	    dName("TEQI");  dGPR64(_Rs_); dImm();)  
MakeDisF(disTNEI,	    dName("TNEI");  dGPR64(_Rs_); dImm();)

/*********************************************************
* Unknow instruction (would generate an exception)       *
* Format:  ?                                             *
*********************************************************/
MakeDisF(disNULL,		dName("*** Bad OP ***");)

TdisR5900F disR5900_MMI0[] = { // Subset of disMMI0
    disPADDW,  disPSUBW,  disPCGTW,  disPMAXW,
	disPADDH,  disPSUBH,  disPCGTH,  disPMAXH,
    disPADDB,  disPSUBB,  disPCGTB,  disNULL, 
	disNULL,   disNULL,   disNULL,   disNULL,
    disPADDSW, disPSUBSW, disPEXTLW, disPPACW, 
	disPADDSH, disPSUBSH, disPEXTLH, disPPACH,
    disPADDSB, disPSUBSB, disPEXTLB, disPPACB, 
	disNULL,   disNULL,   disPEXTS,  disPPACS};

MakeDisF(disMMI0,		disR5900_MMI0[_Sa_] DisFInterfaceN)

TdisR5900F disR5900_MMI1[] = { // Subset of disMMI1
    disNULL,   disPABSW,  disPCEQW,  disPMINW, 
	disPADSBH, disPABSH,  disPCEQH,  disPMINH,
    disNULL,   disNULL,   disPCEQB,  disNULL, 
	disNULL,   disNULL,   disNULL,   disNULL,
    disPADDUW, disPSUBUW, disPEXTUW, disNULL, 
	disPADDUH, disPSUBUH, disPEXTUH, disNULL,
    disPADDUB, disPSUBUB, disPEXTUB, disQFSRV, 
	disNULL,   disNULL,   disNULL,   disNULL};

MakeDisF(disMMI1,		disR5900_MMI1[_Sa_] DisFInterfaceN)

TdisR5900F disR5900_MMI2[] = { // Subset of disMMI2
    disPMADDW, disNULL,   disPSLLVW, disPSRLVW, 
	disPMSUBW, disNULL,   disNULL,   disNULL,
    disPMFHI,  disPMFLO,  disPINTH,  disNULL, 
	disPMULTW, disPDIVW,  disPCPYLD, disNULL,
    disPMADDH, disPHMADH, disPAND,   disPXOR, 
	disPMSUBH, disPHMSBH, disNULL,   disNULL,
    disNULL,   disNULL,   disPEXEH,  disPREVH, 
	disPMULTH, disPDIVBW, disPEXEW,  disPROT3W};

MakeDisF(disMMI2,		disR5900_MMI2[_Sa_] DisFInterfaceN)

TdisR5900F disR5900_MMI3[] = { // Subset of disMMI3
    disPMADDUW, disNULL,   disNULL,   disPSRAVW, 
	disNULL,    disNULL,   disNULL,   disNULL,
    disPMTHI,   disPMTLO,  disPINTEH, disNULL, 
	disPMULTUW, disPDIVUW, disPCPYUD, disNULL,
    disNULL,    disNULL,   disPOR,    disPNOR, 
	disNULL,    disNULL,   disNULL,   disNULL,
    disNULL,    disNULL,   disPEXCH,  disPCPYH, 
	disNULL,    disNULL,   disPEXCW,  disNULL};

MakeDisF(disMMI3,		disR5900_MMI3[_Sa_] DisFInterfaceN)

TdisR5900F disR5900_MMI[] = { // Subset of disMMI
    disNULL,  disNULL,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disMMI0,  disMMI2,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disMULT1, disMULTU1, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disMMI1,  disMMI3,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL,   disNULL, disNULL, disNULL, disNULL, disNULL, disNULL};

MakeDisF(disMMI,		disR5900_MMI[_Funct_] DisFInterfaceN)


TdisR5900F disR5900_COP0_BC0[] = { //subset of disCOP0 BC
    disBC0F, disBC0T, disBC0FL, disBC0TL, disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
};

MakeDisF(disCOP0_BC0,		disR5900_COP0_BC0[_Rt_] DisFInterfaceN)

TdisR5900F disR5900_COP0_Func[] = { //subset of disCOP0 Function
    disNULL, disTLBR, disTLBWI, disNULL, disNULL, disNULL, disTLBWR, disNULL,
    disTLBP, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disNULL, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disERET, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disNULL, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disNULL, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disNULL, disNULL, disNULL , disNULL, disNULL, disNULL, disNULL , disNULL,
    disEI  , disDI  , disNULL , disNULL, disNULL, disNULL, disNULL , disNULL
};
MakeDisF(disCOP0_Func,		disR5900_COP0_Func[_Funct_] DisFInterfaceN)

TdisR5900F disR5900_COP0[] = { // Subset of disCOP0
    disMFC0,      disNULL, disNULL, disNULL, disMTC0, disNULL, disNULL, disNULL,
    disCOP0_BC0,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disCOP0_Func, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,      disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL};

MakeDisF(disCOP0,		disR5900_COP0[_Rs_] DisFInterfaceN)

TdisR5900F disR5900_COP1_S[] = { //subset of disCOP1 S
    disADDs,  disSUBs,  disMULs,  disDIVs, disSQRTs, disABSs,  disMOVs,   disNEGs,
    disNULL,  disNULL,  disNULL,  disNULL, disNULL,  disNULL,  disNULL,   disNULL,
    disNULL,  disNULL,  disNULL,  disNULL, disNULL,  disNULL,  disRSQRTs, disNULL,
    disADDAs, disSUBAs, disMULAs, disNULL, disMADDs, disMSUBs, disMADDAs, disMSUBAs,
    disNULL,  disNULL,  disNULL,  disNULL, disCVTWs, disNULL,  disNULL,   disNULL,
    disMINs,  disMAXs,  disNULL,  disNULL, disNULL,  disNULL,  disNULL,   disNULL,
    disCFs,   disNULL,  disCEQs,  disNULL, disCLTs,  disNULL,  disCLEs,   disNULL,
    disNULL,  disNULL,  disNULL,  disNULL, disNULL,  disNULL,  disNULL,   disNULL,
};

MakeDisF(disCOP1_S,		disR5900_COP1_S[_Funct_] DisFInterfaceN)

TdisR5900F disR5900_COP1_W[] = { //subset of disCOP1 W
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disCVTSw, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
    disNULL,  disNULL, disNULL, disNULL, disNULL, disNULL, disNULL, disNULL,
};

MakeDisF(disCOP1_W,		disR5900_COP1_W[_Funct_] DisFInterfaceN)

TdisR5900F disR5900_COP1_BC1[] = { //subset of disCOP1 BC
    disBC1F, disBC1T, disBC1FL, disBC1TL, disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
    disNULL, disNULL, disNULL , disNULL , disNULL, disNULL, disNULL, disNULL,
};

MakeDisF(disCOP1_BC1,	disR5900_COP1_BC1[_Rt_] DisFInterfaceN)

TdisR5900F disR5900_COP1[] = { // Subset of disCOP1
    disMFC1,     disNULL, disCFC1, disNULL, disMTC1,   disNULL, disCTC1, disNULL,
    disCOP1_BC1, disNULL, disNULL, disNULL, disNULL,   disNULL, disNULL, disNULL,
    disCOP1_S,   disNULL, disNULL, disNULL, disCOP1_W, disNULL, disNULL, disNULL,
    disNULL,     disNULL, disNULL, disNULL, disNULL,   disNULL, disNULL, disNULL};

MakeDisF(disCOP1,		disR5900_COP1[_Rs_] DisFInterfaceN)

TdisR5900F disR5900_REGIMM[] = { // Subset of disREGIMM
    disBLTZ,   disBGEZ,   disBLTZL,   disBGEZL,   disNULL, disNULL, disNULL, disNULL,
    disTGEI,   disTGEIU,  disTLTI,    disTLTIU,   disTEQI, disNULL, disTNEI, disNULL,
    disBLTZAL, disBGEZAL, disBLTZALL, disBGEZALL, disNULL, disNULL, disNULL, disNULL,
    disMTSAB,  disMTSAH , disNULL,    disNULL,    disNULL, disNULL, disNULL, disNULL};

MakeDisF(disREGIMM,		disR5900_REGIMM[_Rt_] DisFInterfaceN)

TdisR5900F disR5900_SPECIAL[] = {
    disSLL,    disNULL, disSRL,    disSRA,   disSLLV,    disNULL, disSRLV,  disSRAV,
    disJR,     disJALR, disMOVZ,   disMOVN,  disSYSCALL, disBREAK,disNULL,  disSYNC,
    disMFHI,   disMTHI, disMFLO,   disMTLO,  disDSLLV,   disNULL, disDSRLV, disDSRAV,
    disMULT,   disMULTU,disDIV,    disDIVU,  disNULL,    disNULL, disNULL,  disNULL,
    disADD,    disADDU, disSUB,    disSUBU,  disAND,     disOR,   disXOR,   disNOR,
    disMFSA ,  disMTSA, disSLT,    disSLTU,  disDADD,    disDADDU,disDSUB,  disDSUBU,
    disTGE,    disTGEU, disTLT,    disTLTU,  disTEQ,     disNULL, disTNE,   disNULL,
    disDSLL,   disNULL, disDSRL,   disDSRA,  disDSLL32,  disNULL, disDSRL32,disDSRA32 };

MakeDisF(disSPECIAL,	disR5900_SPECIAL[_Funct_] DisFInterfaceN)

TdisR5900F disR5900[] = {
    disSPECIAL, disREGIMM, disJ   , disJAL  , disBEQ , disBNE , disBLEZ , disBGTZ ,
    disADDI   , disADDIU , disSLTI, disSLTIU, disANDI, disORI , disXORI , disLUI  ,
    disCOP0   , disCOP1  , disNULL, disNULL , disBEQL, disBNEL, disBLEZL, disBGTZL,
    disDADDI  , disDADDIU, disLDL , disLDR  , disMMI , disNULL, disLQ   , disSQ   ,
    disLB     , disLH    , disLWL , disLW   , disLBU , disLHU , disLWR  , disLWU  ,
    disSB     , disSH    , disSWL , disSW   , disSDL , disSDR , disSWR  , disCACHE,
    disNULL   , disLWC1  , disNULL, disPREF , disNULL, disNULL, disNULL , disLD   ,
    disNULL   , disSWC1  , disNULL, disNULL , disNULL, disNULL, disNULL , disSD  };

MakeDisF(disR5900F,		disR5900[code >> 26] DisFInterfaceN)

