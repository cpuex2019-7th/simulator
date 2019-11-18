#include <stdio.h>
#include <stdlib.h>
#include "instr.h"
#include "state.h"
#include "logging.h"
#include "exec.h"
  
void set_r_instr(int from, instr_r_t *to){
  to->funct7 = srl(from, 25);
  to->rs2 = srl(from, 20) & 0b11111;
  to->rs1 = srl(from, 15) & 0b11111;
  to->funct3 = srl(from, 12) & 0b111;
  to->rd = srl(from, 7) & 0b11111;
}

void set_i_instr(int from, instr_i_t *to){
  to->imm = srl(from, 20);
  to->rs1 = srl(from, 15) & 0b11111;
  to->funct3 = srl(from, 12) & 0b111;
  to->rd = srl(from, 7) & 0b11111;
  
  to->imm = SIGNEXT(to->imm, 11);
}

void set_s_instr(int from, instr_s_t *to){
  to->imm = srl((from & 0xFE000000), 20) | srl((from & 0xF80), 7);
  to->rs2 = srl(from, 20) & 0b11111;
  to->rs1 = srl(from, 15) & 0b11111;
  to->funct3 = srl(from, 12) & 0b111;

  to->imm = SIGNEXT(to->imm, 11);
}

void set_b_instr(int from, instr_b_t *to){
  to->imm = srl((from & 0x80000000), 19) | ((from & 0x80) << 4) | srl((from & 0x7E000000), 20) | srl((from & 0xF00), 7);
  to->rs2 = srl(from, 20) & 0b11111;
  to->rs1 = srl(from, 15) & 0b11111;
  to->funct3 = srl(from, 12) & 0b111;
  to->imm = SIGNEXT(to->imm, 12);
}

void set_u_instr(int from, instr_u_t *to){
  to->imm = from & (~0b111111111111);
  to->rd = srl(from, 7) & 0b11111;
}

void set_j_instr(int from, instr_j_t *to){
  to->imm = srl((from & 0x80000000), 11) | (from & 0xFF000) | srl((from & 0x100000), 9) | srl((from & 0x7FE00000), 20);
  to->rd = srl(from, 7) & 0b11111;
  to->imm = SIGNEXT(to->imm, 20);
}

void unimplemented(int iraw, int loc){
  error("unimplemented instr: %08x (internal state: %d)", iraw, loc);
  exit(1);
}

// use fp for future
// (it may allow us to decode the instructions included by rv32c easily?)
void fetch_and_decode_once(state_t *state, instr_t *instr){  
  int iraw = state->prog[state->pc/4];
  
  switch (iraw & 0b1111111){
    // rv32i
  case 0b0110111: // LUI    
    set_u_instr(iraw, (instr_u_t*) instr);
    instr->op = LUI;
    break;
  case 0b0010111: // AUIPC
    set_u_instr(iraw, (instr_u_t*) instr);
    instr->op = AUIPC;
    break;
  case 0b1101111: // JAL
    set_j_instr(iraw, (instr_j_t*) instr);
    instr->op = JAL;
    break;
  case 0b1100111: // JALR
    set_i_instr(iraw, (instr_i_t*) instr);
    instr->op = JALR;
    break;
  case 0b1100011: // (B*)
    set_b_instr(iraw, (instr_b_t*) instr);
    switch(srl(iraw & 0x7000, 12)){
    case 0b000: // BEQ
      instr->op = BEQ;
      break;
    case 0b001: // BNE
      instr->op = BNE;
      break;
    case 0b100: // BLT
      instr->op = BLT;
      break;
    case 0b101: // BGE
      instr->op = BGE;
      break;
    case 0b110: // BLTU
      instr->op = BLTU;
      break;
    case 0b111: // BGEU
      instr->op = BGEU;
      break;
    }
    break;
  case 0b0000011: // (L*)
    set_i_instr(iraw, (instr_i_t*) instr);
    switch(srl(iraw & 0x7000, 12)){
    case 0b000: // LB
      instr->op = LB;
      break;
    case 0b001: // LH
      instr->op = LH;
      break;
    case 0b010: // LW
      instr->op = LW;
      break;
    case 0b100: // LBU
      instr->op = LBU;
      break;
    case 0b101: // LHU
      instr->op = LHU;
      break;      
    }
    break;
  case 0b0100011: // (S*)
    set_s_instr(iraw, (instr_s_t*) instr);
    switch(srl(iraw & 0x7000, 12)){
    case 0b000: // SB
      instr->op = SB;
      break;
    case 0b001: // SH
      instr->op = SH;
      break;
    case 0b010: // SW
      instr->op = SW;
      break;
    default:
      unimplemented(iraw, 0);
      break;
    }
    break;
  case 0b0010011: // (Arith Imm)
    set_i_instr(iraw, (instr_i_t*) instr);
    switch(srl(iraw & 0x7000, 12)){
    case 0b000: // ADDI
      instr->op = ADDI;
      break;
    case 0b010: // SLTI
      instr->op = SLTI;
      break;
    case 0b011: // SLTIU
      instr->op = SLTIU;
      break;      
    case 0b100: // XORI
      instr->op = XORI;
      break;      
    case 0b110: // ORI
      instr->op = ORI;
      break;      
    case 0b111: // ANDI
      instr->op = ANDI;
      break;      
    case 0b001: // SLLI
      instr->op = SLLI;
      break;      
    case 0b101: // (SRLI or SRAI)
      switch(srl(iraw, 25)){
      case 0b0000000: // SRLI
        instr->op = SRLI;
        break;
      case 0b0100000: // SRAI
        instr->op = SRAI;
        break;
      }
      break;      
    }
    break;
  case 0b0110011: // (Arith Other)
    set_r_instr(iraw, (instr_r_t*) instr);
    switch(srl(iraw & 0x7000, 12)){
    case 0b000: // (ADD or SUB or MUL)
      switch(srl(iraw, 25)){
      case 0b0000000: // ADD
        instr->op = ADD;
        break;
      case 0b0100000: // SUB
        instr->op = SUB;
        break;
      case 0000001: // MUL
        instr->op = MUL;
        break;
      }
      break;
    case 0b001: // (SLL or MULH)
      switch(srl(iraw, 25)){
      case 0b0000000: // SLL
        instr->op = SLL;
        break;
      case 0b0000001: // MULH
        instr->op = MULH;
        break;
      }
      break;
    case 0b010: // (SLT or MULHSU)
      switch(srl(iraw, 25)){
      case 0b0000000: // SLT
        instr->op = SLT;
        break;
      case 0b0000001: // MULHSU
        instr->op = MULHSU;
        break;
      }
      break;      
    case 0b011: // (SLTU or MULHU)
      switch(srl(iraw, 25)){
      case 0b0000000: // SLTU
        instr->op = SLTU;
        break;
      case 0b0000001: // MULHU
        instr->op = MULHU;
        break;
      }
      break;      
    case 0b100: // (XOR or DIV)
      switch(srl(iraw, 25)){
      case 0b0000000: // XOR
        instr->op = XOR;
        break;
      case 0b0000001: // DIV
        instr->op = DIV;
        break;
      }
      break;      
    case 0b101: // (SRL or SRA or DIVU)
      switch(srl(iraw, 25)){
      case 0b0000000: // SRL
        instr->op = SRL;
        break;
      case 0b0100000: // SRA
        instr->op = SRA;
        break;
      case 0b0000001: // DIVU
        instr->op = DIVU;
        break;
      }
      break;      
    case 0b110: // (OR or REM)
      switch(srl(iraw, 25)){
      case 0b0000000: // OR
        instr->op = OR;
        break;
      case 0b0000001: // REM
        instr->op = REM;
        break;
      }
      break;      
    case 0b111: // (AND or REMU)
      switch(srl(iraw, 25)){
      case 0b0000000: // AND
        instr->op = AND;
        break;
      case 0b0000001: // REMU
        instr->op = REMU;
        break;
      }
      break;            
    }
    break;
  case 0b0001111: // FENCE
    set_i_instr(iraw, (instr_i_t*) instr);
    instr->op = FENCE;
    break;
  case 0b1110011:
    // TODO: rv32i Others
    unimplemented(iraw, 1);
    break;

    // rv32f
    //////////
  case 0b0000111: // FLW
    if(srl(iraw & 0x7000, 12) == 0b010){
      set_i_instr(iraw, (instr_i_t*) instr);
      instr->op = FLW;
      break;
    } else {
      unimplemented(iraw, 2);
      break;
    }
  case 0b0100111: // FSW
    if(srl(iraw & 0x7000, 12) == 0b010){
      set_s_instr(iraw, (instr_s_t*) instr);
      instr->op = FSW;
      break;
    } else {
      unimplemented(iraw, 3);
      break;
    }
  case 0b1010011: // Floating Arith
    set_r_instr(iraw, (instr_r_t*) instr);
    switch(srl(iraw, 25)){
    case 0b0000000: // FADDS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FADDS;
        break;
      } else {
        unimplemented(iraw, 4);
        break;
      }
    case 0b0000100: // FSUBS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FSUBS;
        break;
      } else {
        unimplemented(iraw, 5);
        break;
      }
    case 0b0001000: // FMULS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FMULS;
        break;
      } else {
        unimplemented(iraw, 6);
        break;
      }
    case 0b0001100: // FDIVS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FDIVS;
        break;
      } else {
        unimplemented(iraw, 7);
        break;
      }
    case 0b0101100: // FSQRTS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FSQRTS;
        break;
      } else {
        unimplemented(iraw, 8);
        break;
      }
    case 0b0010000: // FSGNJS, FSGNJNS, FSGNJXS
      switch (srl(iraw & 0x7000, 12)){
      case 0b000:
        instr->op = FSGNJS;
        break;
      case 0b001:
        instr->op = FSGNJNS;
        break;
      case 0b010:
        instr->op = FSGNJXS;
        break;
      default:
        unimplemented(iraw, 8);
        break;
      }
      break;
    case 0b1100000: // FCVTWS
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FCVTWS;
        break;
      } else {
        unimplemented(iraw, 9);
        break;
      }
    case 0b1110000: // FMVXW
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FMVXW;
        break;
      } else {
        unimplemented(iraw, 10);
        break;
      }
    case 0b1010000: // FEQS, FLES
      switch (srl(iraw & 0x7000, 12)){
      case 0b010:
        instr->op = FEQS;
        break;
      case 0b000:
        instr->op = FLES;
        break;
      default:
        unimplemented(iraw, 11);
        break;
      }
      break;
    case 0b1101000: // FCVTSW
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FCVTSW;
        break;
      } else {
        unimplemented(iraw, 12);
        break;
      }
    case 0b1111000: // FMVWX
      if(srl(iraw & 0x7000, 12) == 0b000){
        instr->op = FMVWX;
        break;
      } else {
        unimplemented(iraw, 13);
        break;
      }
    default:
      unimplemented(iraw, 14);
      break;
    }
    break;
  default:
    unimplemented(iraw, 15);
    break;
  }
}
