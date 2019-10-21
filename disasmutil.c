#include <stdio.h>
#include "instr.h"
#include "disasmutil.h"
#include "exec.h"

static int is_instr_meta_initialized = 0;
static instr_meta_t instr_meta[INSTR_UNKNOWN+1] = {};

// to avoid difference between instr_kind_t and instr_meta,
// initialize instr_meta by hand. this is so dirty :-(

static char *reg_aliases[] = {
                              "zero",
                              "ra",
                              "sp",
                              "gp",
                              "tp",
                            
                              "t0",
                              "t1",
                              "t2",
                            
                              "s0",
                              "s1",
                            
                              "a0",
                              "a1",
                            
                              "a2",
                              "a3",
                              "a4",
                              "a5",
                              "a6",
                              "a7",

                              "s2",
                              "s3",
                              "s4",
                              "s5",
                              "s6",
                              "s7",
                              "s8",
                              "s9",
                              "s10",
                              "s11",
                            
                              "t3",
                              "t4",
                              "t5",
                              "t6"
};

static char *freg_aliases[] = {
                               "ft0",
                               "ft1",
                               "ft2",
                               "ft3",
                               "ft4",
                               "ft5",
                               "ft6",
                               "ft7",
                               
                               "fs0",
                               "fs1",
                               
                               "fa0",
                               "fa1",
                               "fa2",
                               "fa3",
                               "fa4",
                               "fa5",
                               "fa6",
                               "fa7",
                               
                               "fs2",
                               "fs3",
                               "fs4",
                               "fs5",
                               "fs6",
                               "fs7",
                               "fs8",
                               "fs9",
                               "fs10",
                               "fs11",
                               
                               "ft8",
                               "ft9",
                               "ft10",
                               "ft11"
};

char *r2t(int reg){
  if(0 <= reg && reg <= 31){
    return reg_aliases[reg];
  } else {
    printf("Error: invalid register references! %d", reg);
    exit(1);
  }
}

char *r2tf(int reg){
  if(0 <= reg && reg <= 31){
    return freg_aliases[reg];
  } else {
    printf("Error: invalid floating point register references! %d", reg);
    exit(1);
  }
}

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
  instr_meta[SRLI] = (instr_meta_t){INSTR_I, "srli"};
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

  // TODO
  
  /////////
  // rv32m
  /////////
  instr_meta[MUL] = (instr_meta_t){INSTR_R, "mul"};
  instr_meta[MULH] = (instr_meta_t){INSTR_R, "mulh"};
  instr_meta[MULHSU] = (instr_meta_t){INSTR_R, "mulhsu"};
  instr_meta[MULHU] = (instr_meta_t){INSTR_R, "mulhu"};
  instr_meta[DIV] = (instr_meta_t){INSTR_R, "div"};
  instr_meta[DIVU] = (instr_meta_t){INSTR_R, "divu"};
  instr_meta[REM] = (instr_meta_t){INSTR_R, "rem"};
  instr_meta[REMU] = (instr_meta_t){INSTR_R, "remu"};

  /////////
  // rv32f
  /////////
  instr_meta[FLW] = (instr_meta_t){INSTR_I, "flw"};
  instr_meta[FSW] = (instr_meta_t){INSTR_S, "fsw"};
  
  instr_meta[FMVWX] = (instr_meta_t){INSTR_R, "fmvwx"};
  instr_meta[FMVXW] = (instr_meta_t){INSTR_R, "fmvxw"};
  
  instr_meta[FADDS] = (instr_meta_t){INSTR_R, "fadd"};
  instr_meta[FSUBS] = (instr_meta_t){INSTR_R, "fsub"};
  instr_meta[FMULS] = (instr_meta_t){INSTR_R, "fmul"};
  instr_meta[FDIVS] = (instr_meta_t){INSTR_R, "fdiv"};
  instr_meta[FSQRTS] = (instr_meta_t){INSTR_R, "fsqrt"};
  
  instr_meta[FEQS] = (instr_meta_t){INSTR_R, "feq"};
  instr_meta[FLES] = (instr_meta_t){INSTR_R, "fle"};
  
  instr_meta[FCVTWS] = (instr_meta_t){INSTR_R, "fcvtws"};
  instr_meta[FCVTSW] = (instr_meta_t){INSTR_R, "fcvtsw"};
  
  instr_meta[FSGNJS] = (instr_meta_t){INSTR_R, "fsgnj"};
  instr_meta[FSGNJNS] = (instr_meta_t){INSTR_R, "fsgnjn"};
  instr_meta[FSGNJXS] = (instr_meta_t){INSTR_R, "fsgnjx"};
    
  // finalize
  is_instr_meta_initialized = 1;  
}

void disasm(instr_t *instr, uint32_t pc, char *dest, size_t s){
  if (is_instr_meta_initialized == 0)
    initialize_instr_meta();
  
  if (0 <= instr->op && instr->op < INSTR_UNKNOWN){
    switch(instr_meta[instr->op].type){
    case INSTR_R:
      if (instr->op == FADDS
          || instr->op == FSUBS
          || instr->op == FMULS
          || instr->op == FDIVS
          || instr->op == FSGNJS
          || instr->op == FSGNJNS
          || instr->op == FSGNJXS) {
        snprintf(dest, s, "%s %s, %s, %s", instr_meta[instr->op].label,
                 r2tf(((instr_r_t*) instr)->rd),
                 r2tf(((instr_r_t*) instr)->rs1),
                 r2tf(((instr_r_t*) instr)->rs2));
      } else if (instr->op == FSQRTS) {
        snprintf(dest, s, "%s %s, %s", instr_meta[instr->op].label,
                 r2tf(((instr_r_t*) instr)->rd),
                 r2tf(((instr_r_t*) instr)->rs1));
      } else if (instr->op == FCVTWS
                 || instr->op == FMVXW) {
        snprintf(dest, s, "%s %s, %s", instr_meta[instr->op].label,
                 r2t(((instr_r_t*) instr)->rd),
                 r2tf(((instr_r_t*) instr)->rs1));
      } else if (instr->op == FCVTSW
                 || instr->op == FMVWX) {
        snprintf(dest, s, "%s %s, %s", instr_meta[instr->op].label,
                 r2tf(((instr_r_t*) instr)->rd),
                 r2t(((instr_r_t*) instr)->rs1));        
      } else if (instr->op == FEQS
                 || instr->op == FLES) {
        snprintf(dest, s, "%s %s, %s, %s", instr_meta[instr->op].label,
                 r2t(((instr_r_t*) instr)->rd),
                 r2tf(((instr_r_t*) instr)->rs1),
                 r2tf(((instr_r_t*) instr)->rs2));        
      } else {
        snprintf(dest, s, "%s %s, %s, %s", instr_meta[instr->op].label,
                 r2t(((instr_r_t*) instr)->rd),
                 r2t(((instr_r_t*) instr)->rs1),
                 r2t(((instr_r_t*) instr)->rs2));
      }
      break;
    case INSTR_I:
      if (instr->op == FLW) {
        snprintf(dest, s, "%s %s, %s, %d", instr_meta[instr->op].label,
                 r2tf(((instr_r_t*) instr)->rd),
                 r2t(((instr_i_t*) instr)->rs1),
                 ((instr_i_t*) instr)->imm);
      } else {
        snprintf(dest, s, "%s %s, %s, %d", instr_meta[instr->op].label,
                 r2t(((instr_r_t*) instr)->rd),
                 r2t(((instr_i_t*) instr)->rs1),
                 ((instr_i_t*) instr)->imm);
      }
      break;
    case INSTR_S:
      if (instr->op == FSW){
      snprintf(dest, s, "%s %s, %s, %d", instr_meta[instr->op].label,
               r2tf(((instr_s_t*) instr)->rs2),
               r2t(((instr_s_t*) instr)->rs1),
               ((instr_s_t*) instr)->imm);
      } else {
      snprintf(dest, s, "%s %s, %s, %d", instr_meta[instr->op].label,
               r2t(((instr_s_t*) instr)->rs2),
               r2t(((instr_s_t*) instr)->rs1),
               ((instr_s_t*) instr)->imm);
      }
      break;
    case INSTR_B:
      snprintf(dest, s, "%s %s, %s, %d ; jumps to %08x", instr_meta[instr->op].label,
               r2t(((instr_b_t*) instr)->rs1),
               r2t(((instr_b_t*) instr)->rs2),
               ((instr_b_t*) instr)->imm,
               pc + ((instr_b_t*) instr)->imm);
      break;
    case INSTR_U:      
      snprintf(dest, s, "%s %s, %d", instr_meta[instr->op].label,
               r2t(((instr_u_t*) instr)->rd),
               srl((((instr_u_t*) instr)->imm), 12));
      break;
    case INSTR_J:
      snprintf(dest, s, "%s %s, %d ; jumps to %08x", instr_meta[instr->op].label,
               r2t(((instr_j_t*) instr)->rd),
               ((instr_j_t*) instr)->imm,
               pc + ((instr_j_t*) instr)->imm);
      break;
    default:
      snprintf(dest, s, "%s (?)", instr_meta[instr->op].label);
      break;
    }
  } else {
    snprintf(dest, s, "[unknown op]");
  }
}
