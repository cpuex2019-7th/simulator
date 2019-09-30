#include <stdio.h>
#include <stdlib.h>
#include "instr.h"
#include "state.h"
#include "logging.h"
  
void set_r_instr(int from, instr_r_t *to){
  to->funct7 = from >> 25;
  to->rs2 = (from >> 20) & 0b11111;
  to->rs1 = (from >> 15) & 0b11111;
  to->funct3 = (from >> 12) & 0b111;
  to->rd = (from >> 7) & 0b11111;
}

void set_i_instr(int from, instr_i_t *to){
  to->imm = from >> 20;
  to->rs1 = (from >> 15) & 0b11111;
  to->funct3 = (from >> 12) & 0b111;
  to->rd = (from >> 7) & 0b11111;
  
  to->imm = SIGNEXT(to->imm, 11);
}

void set_s_instr(int from, instr_s_t *to){
  to->imm = ((from & 0xFE000000) >> 20) | ((from & 0xF80) >> 7);
  to->rs2 = (from >> 20) & 0b11111;
  to->rs1 = (from >> 15) & 0b11111;
  to->funct3 = (from >> 12) & 0b111;

  to->imm = SIGNEXT(to->imm, 11);
}

void set_b_instr(int from, instr_b_t *to){
  to->imm = ((from & 0x80000000) >> 19) | ((from & 0x80) << 4) | ((from & 0x7E000000) >> 20) | ((from & 0xF00) >> 7);
  to->rs2 = (from >> 20) & 0b11111;
  to->rs1 = (from >> 15) & 0b11111;
  to->funct3 = (from >> 12) & 0b111;

  to->imm = SIGNEXT(to->imm, 12);
}

void set_u_instr(int from, instr_u_t *to){
  to->imm = from & (~0b111111111111);
  to->rd = (from >> 7) & 0b11111;
}

void set_j_instr(int from, instr_j_t *to){
  to->imm = ((from & 0x80000000) >> 11) | (from & 0xFF000) | ((from & 0x100000) >> 9) | ((from & 0x7FE00000) >> 20);
  to->rd = (from >> 7) & 0b11111;
  to->imm = SIGNEXT(to->imm, 20);
}

// use fp for future
// (it may allow us to decode the instructions included by rv32c easily?)
instr_t *fetch_and_decode_once(state_t *state){
  instr_t *instr = malloc(sizeof(instr_t));
  
  // here we assume pc may not exceed INT_MAX! lol
  char *buf[4];
  fseek(state->pfp, (int) state->pc, SEEK_SET);
  fread(buf, 4, 4, state->pfp);
  
  // TODO: rv32c
  int iraw = *(int*)buf;
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
    switch((iraw & 0x7000) >> 12){
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
    switch((iraw & 0x7000) >> 12){
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
    switch((iraw & 0x7000) >> 12){
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
      error(":thinking_face:");
    }
    break;
  case 0b0010011: // (Arith Imm)
    set_i_instr(iraw, (instr_i_t*) instr);
    switch((iraw & 0x7000) >> 12){
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
      switch(iraw >> 25){
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
    switch((iraw & 0x7000) >> 12){
    case 0b000: // (ADD or SUB or MUL)
      switch(iraw >> 25){
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
      switch(iraw >> 25){
      case 0b0000000: // SLL
        instr->op = SLL;
        break;
      case 0b0000001: // MULH
        instr->op = MULH;
        break;
      }
      break;
    case 0b010: // (SLT or MULHSU)
      switch(iraw >> 25){
      case 0b0000000: // SLT
        instr->op = SLT;
        break;
      case 0b0000001: // MULHSU
        instr->op = MULHSU;
        break;
      }
      break;      
    case 0b011: // (SLTU or MULHU)
      switch(iraw >> 25){
      case 0b0000000: // SLTU
        instr->op = SLTU;
        break;
      case 0b0000001: // MULHU
        instr->op = MULHU;
        break;
      }
      break;      
    case 0b100: // (XOR or DIV)
      switch(iraw >> 25){
      case 0b0000000: // XOR
        instr->op = XOR;
        break;
      case 0b0000001: // DIV
        instr->op = DIV;
        break;
      }
      break;      
    case 0b101: // (SRL or SRA or DIVU)
      switch(iraw >> 25){
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
      switch(iraw >> 25){
      case 0b0000000: // OR
        instr->op = OR;
        break;
      case 0b0000001: // REM
        instr->op = REM;
        break;
      }
      break;      
    case 0b111: // (AND or REMU)
      switch(iraw >> 25){
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
  default:
    instr->op = INSTR_UNKNOWN;
    break;
  }
  return instr;
}
