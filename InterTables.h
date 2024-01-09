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
#ifndef INTERTABLES_H
#define INTERTABLES_H

extern  void (*Int_OpcodePrintTable[64])();
extern  void (*Int_SpecialPrintTable[64])();
extern  void (*Int_REGIMMPrintTable[32])();
extern  void (*Int_MMIPrintTable[64])();
extern  void (*Int_MMI0PrintTable[32])();
extern  void (*Int_MMI1PrintTable[32])();
extern  void (*Int_MMI2PrintTable[32])();
extern  void (*Int_MMI3PrintTable[32])();

void SPECIAL();
void REGIMM();
void UnknownOpcode();
void voidOpcode();
void COP0();
void MMI_Unknown();
void MMI();
void MMI0();
void MMI1();
void MMI2();
void MMI3();

// **********************Standard Opcodes**************************
void J();
void JAL();
void BEQ();
void BNE();
void BLEZ();
void BGTZ();
void ADDI();
void ADDIU();
void SLTI();
void SLTIU();
void ANDI();
void ORI();
void XORI();
void LUI();
void BEQL();
void BNEL();
void BLEZL();
void BGTZL();
void DADDI();
void DADDIU();
void LDL();
void LDR();
void LB();
void LH();
void LWL();
void LW();
void LBU();
void LHU();
void LWR();
void LWU();
void SB();
void SH();
void SWL();
void SW();
void SDL();
void SDR();
void SWR();
void LD();
void SD();
void LQ();
void SQ();
//***************end of standard opcodes*************************


//***************SPECIAL OPCODES**********************************
void SLL();
void SRL();
void SRA();
void SLLV();
void SRLV();
void SRAV();
void JR();
void JALR();
void SYSCALL();
void BREAK();
void MFHI();
void MTHI();
void MFLO();
void MTLO();
void DSLLV();
void DSRLV();
void DSRAV();
void MULT();
void MULTU();
void DIV();
void DIVU();
void ADD();
void ADDU();
void SUB();
void SUBU();
void AND();
void OR();
void XOR();
void NOR();
void SLT();
void SLTU();
void DADD();
void DADDU();
void DSUB();
void DSUBU();
void DSLL();
void DSRL();
void DSRA();
void DSLL32();
void DSRL32();
void DSRA32();
void MOVZ();
void MOVN();
//******************END OF SPECIAL OPCODES**************************

//******************REGIMM OPCODES**********************************
void BLTZ();
void BGEZ();
void BLTZL();
void BGEZL();
void BLTZAL();
void BGEZAL();
void BLTZALL();
void BGEZALL();
//*****************END OF REGIMM OPCODES*****************************

//*****************MMI OPCODES*********************************
void MADD();
void MADDU();
void PLZCW();
void MADD1();
void MADDU1();
void MFHI1();
void MTHI1();
void MFLO1();
void MTLO1();
void MULT1();
void MULTU1();
void DIV1();
void DIVU1();
void PMFHL();
void PMTHL();
void PSLLH();
void PSRLH();
void PSRAH();
void PSLLW();
void PSRLW();
void PSRAW();
//*****************END OF MMI OPCODES**************************
//*************************MMI0 OPCODES************************

void PADDW();  
void PSUBW();  
void PCGTW();  
void PMAXW(); 
void PADDH();  
void PSUBH();  
void PCGTH();  
void PMAXH(); 
void PADDB();  
void PSUBB();  
void PCGTB();
void PADDSW(); 
void PSUBSW(); 
void PEXTLW();  
void PPACW(); 
void PADDSH();
void PSUBSH();
void PEXTLH(); 
void PPACH();
void PADDSB();
void PSUBSB();
void PEXTLB(); 
void PPACB();
void PEXT5();
void PPAC5();
//***END OF MMI0 OPCODES******************************************
//**********MMI1 OPCODES**************************************
void PABSW();
void PCEQW();
void PMINW();
void PADSBH();
void PABSH();
void PCEQH();
void PMINH();
void PCEQB();
void PADDUW();
void PSUBUW();
void PEXTUW();
void PADDUH();
void PSUBUH();
void PEXTUH();
void PADDUB();
void PSUBUB();
void PEXTUB();
void QFSRV();
//********END OF MMI1 OPCODES***********************************
//*********MMI2 OPCODES***************************************
void PMADDW();
void PSLLVW();
void PSRLVW();
void PMSUBW();
void PMFHI();
void PMFLO();
void PINTH();
void PMULTW();
void PDIVW();
void PCPYLD();
void PMADDH();
void PHMADH();
void PAND();
void PXOR();
void PMSUBH();
void PHMSBH();
void PEXEH();
void PREVH();
void PMULTH();
void PDIVBW();
void PEXEW();
void PROT3W();
//*****END OF MMI2 OPCODES***********************************
//*************************MMI3 OPCODES************************
void PMADDUW();
void PSRAVW();
void PMTHI();
void PMTLO();
void PINTEH();
void PMULTUW();
void PDIVUW();
void PCPYUD();
void POR();
void PNOR();
void PEXCH();
void PCPYH();
void PEXCW();
//**********************END OF MMI3 OPCODES********************

#endif
