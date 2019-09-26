#include <stdio.h>
#include "instr.h"
#include "disasm.h"

static int is_instr_meta_initialized = 0;
static instr_meta_t instr_meta[INSTR_UNKNOWN+1] = {};

// to avoid difference between instr_kind_t and instr_meta,
// initialize instr_meta by hand. this is so dirty :-(

void initialize_instr_meta(){
  /////////
  // rv32i
  /////////
  instr_meta[LUI] = (instr_meta_t){INSTR_U, "lui"};
  instr_meta[AUIPC] = (instr_meta_t){INSTR_U, "auipc"};
  instr_meta[JAL] = (instr_meta_t){INSTR_J, "jal"};
  instr_meta[JALR] = (instr_meta_t){INSTR_I, "jalr"};
  instr_meta[BEQ] = (instr_meta_t){INSTR_B, "beq"};
  instr_meta[BNE] = (instr_meta_t){INSTR_B, "bne"};
  instr_meta[BLT] = (instr_meta_t){INSTR_B, "blt"};
  instr_meta[BGE] = (instr_meta_t){INSTR_B, "bge"};
  instr_meta[BLTU] = (instr_meta_t){INSTR_B, "bltu"};
  
  instr_meta[LB] = (instr_meta_t){INSTR_I, "lb"};
  instr_meta[LH] = (instr_meta_t){INSTR_I, "lh"};
  instr_meta[LW] = (instr_meta_t){INSTR_I, "lw"};
  instr_meta[LBU] = (instr_meta_t){INSTR_I, "lbu"};
  instr_meta[LHU] = (instr_meta_t){INSTR_I, "lhu"};

  instr_meta[SB] = (instr_meta_t){INSTR_S, "sb"};
  instr_meta[SH] = (instr_meta_t){INSTR_S, "sh"};
  instr_meta[SW] = (instr_meta_t){INSTR_S, "sw"};

  instr_meta[ADDI] = (instr_meta_t){INSTR_I, "addi"};
  instr_meta[SLTI] = (instr_meta_t){INSTR_I, "slti"};
  instr_meta[SLTIU] = (instr_meta_t){INSTR_I, "sltui"};
  instr_meta[XORI] = (instr_meta_t){INSTR_I, "xori"};
  instr_meta[ORI] = (instr_meta_t){INSTR_I, "ori"};
  instr_meta[ANDI] = (instr_meta_t){INSTR_I, "andi"};
  instr_meta[SLLI] = (instr_meta_t){INSTR_I, "slli"};
  instr_meta[SRAI] = (instr_meta_t){INSTR_I, "srai"};

  instr_meta[ADD] = (instr_meta_t){INSTR_R, "add"};
  instr_meta[SUB] = (instr_meta_t){INSTR_R, "sub"};
  instr_meta[SLL] = (instr_meta_t){INSTR_R, "sll"};
  instr_meta[SLT] = (instr_meta_t){INSTR_R, "slt"};
  instr_meta[SLTU] = (instr_meta_t){INSTR_R, "sltu"};
  instr_meta[XOR] = (instr_meta_t){INSTR_R, "xor"};
  instr_meta[SRL] = (instr_meta_t){INSTR_R, "srl"};
  instr_meta[SRA] = (instr_meta_t){INSTR_R, "sra"};
  instr_meta[OR] = (instr_meta_t){INSTR_R, "or"};
  instr_meta[AND] = (instr_meta_t){INSTR_R, "and"};
  instr_meta[FENCE] = (instr_meta_t){INSTR_R, "fence"};

  //TODO
  
  /////////
  // rv32m
  /////////
  instr_meta[MUL] = (instr_meta_t){INSTR_R, "mul"};
  instr_meta[MULH] = (instr_meta_t){INSTR_R, "mulh"};
  instr_meta[MULHSU] = (instr_meta_t){INSTR_R, "mulhsu"};
  instr_meta[MULHU] = (instr_meta_t){INSTR_R, "mulhu"};
  instr_meta[DIV] = (instr_meta_t){INSTR_R, "divU"};
  instr_meta[DIVU] = (instr_meta_t){INSTR_R, "divu"};
  instr_meta[REM] = (instr_meta_t){INSTR_R, "rem"};
  instr_meta[REMU] = (instr_meta_t){INSTR_R, "remu"};

  // finalize
  is_instr_meta_initialized = 1;  
}

void disasm(instr_t *instr, char *dest, size_t s){
  if (is_instr_meta_initialized == 0)
    initialize_instr_meta();
  
  if (0 <= instr->op && instr->op < INSTR_UNKNOWN){
    switch(instr_meta[instr->op].type){
    case INSTR_R:
      snprintf(dest, s, "%s r%d, r%d, r%d", instr_meta[instr->op].label,
               ((instr_r_t*) instr)->rd,
               ((instr_r_t*) instr)->rs1,
               ((instr_r_t*) instr)->rs2);
      break;
    case INSTR_I:
      snprintf(dest, s, "%s r%d, r%d, %d", instr_meta[instr->op].label,
               ((instr_r_t*) instr)->rd,
               ((instr_i_t*) instr)->rs1,
               ((instr_i_t*) instr)->imm);
      break;
    case INSTR_S:
      snprintf(dest, s, "%s r%d, r%d, %d", instr_meta[instr->op].label,
               ((instr_s_t*) instr)->rs1,
               ((instr_s_t*) instr)->rs2,
               ((instr_s_t*) instr)->imm);
      break;
    case INSTR_B:
      snprintf(dest, s, "%s r%d, r%d, %d", instr_meta[instr->op].label,
               ((instr_b_t*) instr)->rs1,
               ((instr_b_t*) instr)->rs2,
               ((instr_b_t*) instr)->imm);
      break;
    case INSTR_U:
      snprintf(dest, s, "%s r%d, %d", instr_meta[instr->op].label,
               ((instr_u_t*) instr)->rd,
               ((instr_u_t*) instr)->imm);
      break;
    case INSTR_J:
      snprintf(dest, s, "%s r%d, %d", instr_meta[instr->op].label,
               ((instr_j_t*) instr)->rd,
               ((instr_j_t*) instr)->imm);
      break;
    default:
      snprintf(dest, s, "%s (?)", instr_meta[instr->op].label);
      break;
   }
  } else {
    snprintf(dest, s, "[unknown op]");
  }
}
