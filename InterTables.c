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

//all tables for R5900 are define here..

#include "InterTables.h"

void (*Int_OpcodePrintTable[64])() = 
{
    SPECIAL,       REGIMM,       J,             JAL,           BEQ,          BNE,           BLEZ,  BGTZ,
    ADDI,          ADDIU,        SLTI,          SLTIU,         ANDI,         ORI,           XORI,  LUI,
    COP0,          UnknownOpcode,UnknownOpcode, UnknownOpcode, BEQL,         BNEL,          BLEZL, BGTZL,
    DADDI,         DADDIU,       LDL,           LDR,           MMI,          UnknownOpcode, LQ,    SQ,
    LB,            LH,           LWL,           LW,            LBU,          LHU,           LWR,   LWU,
    SB,            SH,           SWL,           SW,            SDL,          SDR,           SWR,   UnknownOpcode,
    UnknownOpcode, UnknownOpcode,UnknownOpcode, UnknownOpcode, UnknownOpcode,UnknownOpcode, UnknownOpcode,  LD,
    UnknownOpcode, UnknownOpcode,UnknownOpcode, UnknownOpcode, UnknownOpcode,UnknownOpcode, UnknownOpcode,  SD
};


void (*Int_SpecialPrintTable[64])() = 
{
    SLL,           UnknownOpcode, SRL,           SRA,           SLLV,    UnknownOpcode, SRLV,          SRAV,
    JR,            JALR,          MOVZ,          MOVN,          SYSCALL, BREAK,         UnknownOpcode, UnknownOpcode,
    MFHI,          MTHI,          MFLO,          MTLO,          DSLLV,   UnknownOpcode, DSRLV,         DSRAV,
    MULT,          MULTU,         DIV,           DIVU,          UnknownOpcode,UnknownOpcode,UnknownOpcode,UnknownOpcode,
    ADD,           ADDU,          SUB,           SUBU,          AND,     OR,            XOR,           NOR,
    UnknownOpcode, UnknownOpcode, SLT,           SLTU,          DADD,    DADDU,         DSUB,          DSUBU,
    UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode,UnknownOpcode, UnknownOpcode, UnknownOpcode,
    DSLL,          UnknownOpcode, DSRL,          DSRA,          DSLL32,  UnknownOpcode, DSRL32,        DSRA32
};

void (*Int_REGIMMPrintTable[32])() = {
    BLTZ,   BGEZ,   BLTZL,            BGEZL,         UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode,
    UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode,
    BLTZAL, BGEZAL, BLTZALL,          BGEZALL,       UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode,
    UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode, UnknownOpcode,
};

void (*Int_MMIPrintTable[64])() = 
{
    MADD,                    MADDU,                  MMI_Unknown,          MMI_Unknown,          PLZCW,            MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    MMI0,                    MMI2,                   MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    MFHI1,                   MTHI1,                  MFLO1,                MTLO1,                MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    MULT1,                   MULTU1,                 DIV1,                 DIVU1,                MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    MADD1,                   MADDU1,                 MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    MMI1 ,                   MMI3,                   MMI_Unknown,          MMI_Unknown,          MMI_Unknown,      MMI_Unknown,       MMI_Unknown,          MMI_Unknown,
    PMFHL,                   PMTHL,                  MMI_Unknown,          MMI_Unknown,          PSLLH,            MMI_Unknown,       PSRLH,                PSRAH,
    MMI_Unknown,             MMI_Unknown,            MMI_Unknown,          MMI_Unknown,          PSLLW,            MMI_Unknown,       PSRLW,                PSRAW,
};

void (*Int_MMI0PrintTable[32])() =
{
 PADDW,         PSUBW,         PCGTW,          PMAXW,
 PADDH,         PSUBH,         PCGTH,          PMAXH,
 PADDB,         PSUBB,         PCGTB,          MMI_Unknown,
 MMI_Unknown,   MMI_Unknown,   MMI_Unknown,    MMI_Unknown,
 PADDSW,        PSUBSW,        PEXTLW,         PPACW,
 PADDSH,        PSUBSH,        PEXTLH,         PPACH,
 PADDSB,        PSUBSB,        PEXTLB,         PPACB,
 MMI_Unknown,   MMI_Unknown,   PEXT5,          PPAC5,
};

void (*Int_MMI1PrintTable[32])() =
{
 MMI_Unknown,   PABSW,         PCEQW,         PMINW,
 PADSBH,        PABSH,         PCEQH,         PMINH,
 MMI_Unknown,   MMI_Unknown,   PCEQB,         MMI_Unknown,
 MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
 PADDUW,        PSUBUW,        PEXTUW,        MMI_Unknown,
 PADDUH,        PSUBUH,        PEXTUH,        MMI_Unknown,
 PADDUB,        PSUBUB,        PEXTUB,        QFSRV, 
 MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown, 
};


void (*Int_MMI2PrintTable[32])() = 
{ 
 PMADDW,        MMI_Unknown,   PSLLVW,        PSRLVW, 
 PMSUBW,        MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
 PMFHI,         PMFLO,         PINTH,         MMI_Unknown,
 PMULTW,        PDIVW,         PCPYLD,        MMI_Unknown,
 PMADDH,        PHMADH,        PAND,          PXOR, 
 PMSUBH,        PHMSBH,        MMI_Unknown,   MMI_Unknown, 
 MMI_Unknown,   MMI_Unknown,   PEXEH,         PREVH, 
 PMULTH,        PDIVBW,        PEXEW,         PROT3W, 
};

void (*Int_MMI3PrintTable[32])() =
{
 PMADDUW,       MMI_Unknown,   MMI_Unknown,   PSRAVW,
 MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
 PMTHI,         PMTLO,         PINTEH,        MMI_Unknown,
 PMULTUW,       PDIVUW,        PCPYUD,        MMI_Unknown,
 MMI_Unknown,   MMI_Unknown,   POR,           PNOR,
 MMI_Unknown,   MMI_Unknown,   MMI_Unknown,   MMI_Unknown,
 MMI_Unknown,   MMI_Unknown,   PEXCH,         PCPYH,
 MMI_Unknown,   MMI_Unknown,   PEXCW,         MMI_Unknown,
};
