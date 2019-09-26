#include "state.h"
#include "instr.h"
#include "debugger.h"
#include "logging.h"

static int is_step_execution_enabled = 0;
static int is_strict = 0;

int srl(int x, int n) {
  return (unsigned)x >> n;
}
int sra(int x, int n) {
  return x < 0 && n > 0? x >> n | ~(~0U >> n): x >> n;
}

void exit_if_strict_mode(int code){
  if (is_strict)
    exit(code);
}

void set_execution_mode(int _mode){
  is_strict = _mode;
}

int is_here_breakpoint(state_t *state){
  blist_t *head = state->blist;
  while(head != NULL){
    if(head->addr == state->pc)
      return 1;
    head = head->next;
  }
  return 0;
}

void exec_hook_pre(state_t *state){
  // TODO: process breakpoint
  info("Executing next instruction ...");

  if(is_step_execution_enabled == 1
     || is_here_breakpoint(state) == 1){
    is_step_execution_enabled = run_debugger(state);
  }
}

int exec_hook_post(state_t *state){
  // TODO: check whether the program should exit
  info("Finished.");
  return 0;
}

void exec_stepi(state_t *state){
  // fetch & decode
  instr_t *instr = fetch_and_decode_once(state);


  // jump destination
  int jump_dest = 0;
  int jump_enabled = 0;
  
  // exec & write
  switch(instr->op){
    /////////
    // rv32i
    /////////
  case LUI:
    state->reg[((instr_u_t *) instr)->rd] = ((instr_u_t *) instr)->imm;
    break;
  case AUIPC:
    // state->pc is the actual address of AUIPC
    // TODO: MMU
      state->reg[((instr_u_t *) instr)->rd] = ((instr_u_t *) instr)->imm + state->pc;
    break;
  case JAL:
    jump_enabled = 1;
    jump_dest = ((instr_j_t *) instr)->imm + state->pc;
    ((instr_j_t *) instr)->rd = jump_dest;
    break;
  case JALR:
    jump_enabled = 1;
    jump_dest = (((instr_i_t *) instr)->imm + state->reg[((instr_i_t *) instr)->rs1]) & (~0b1);
    ((instr_j_t *) instr)->rd = state->pc + 4;
    break;
  case BEQ:
    jump_enabled = state->reg[((instr_b_t *) instr)->rs1] == state->reg[((instr_b_t *) instr)->rs2] ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
  case BNE:
    jump_enabled = state->reg[((instr_b_t *) instr)->rs1] != state->reg[((instr_b_t *) instr)->rs2] ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
  case BLT:
    jump_enabled = state->reg[((instr_b_t *) instr)->rs1] < state->reg[((instr_b_t *) instr)->rs2] ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
  case BGE:
    jump_enabled = state->reg[((instr_b_t *) instr)->rs1] >= state->reg[((instr_b_t *) instr)->rs2] ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
  case BLTU:
    jump_enabled = ((unsigned)state->reg[((instr_b_t *) instr)->rs1]) < ((unsigned)state->reg[((instr_b_t *) instr)->rs2]) ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
  case BGEU:
    jump_enabled = ((unsigned)state->reg[((instr_b_t *) instr)->rs1]) >= ((unsigned)state->reg[((instr_b_t *) instr)->rs2]) ? 1 : 0;
    jump_dest = ((instr_b_t *) instr)->imm + state->pc;
    break;
    // TODO: do we need align check?
  case LB:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              *(uint8_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
    
    break;
  case LH:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              SIGNEXT(*(uint16_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm), 15));
    break;
  case LW:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              SIGNEXT(*(uint32_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm), 15));
    break;
  case LBU:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              *(uint8_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
    // no sign extention
    break;
  case LHU:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              *(uint16_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
    // no sign extention
    break;
  case SB:
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
    break;
  case SH:
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)+1] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 8;
    break;
  case SW:
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)+1] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 8;
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)+2] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 16;
    state->mem[(((instr_s_t *) instr)->rs1 + ((instr_s_t *) instr)->imm)+3] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 24;
    break;    
  case ADDI:
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm;
    break;
  case SLTI:
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] < ((instr_i_t *) instr)->imm? 1 : 0;
    break;
  case SLTIU:
    state->reg[((instr_i_t *) instr)->rd] = ((unsigned)state->reg[((instr_i_t *) instr)->rs1]) < ((unsigned)((instr_i_t *) instr)->imm)? 1 : 0;
    break;
  case XORI:
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] ^ ((instr_i_t *) instr)->imm;
    break;
  case ORI:
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] | ((instr_i_t *) instr)->imm;
    break;
  case ANDI:
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] & ((instr_i_t *) instr)->imm;
    break;
  case SLLI:    
    state->reg[((instr_i_t *) instr)->rd] = state->reg[((instr_i_t *) instr)->rs1] << (((instr_i_t *) instr)->imm & 0b11111);
    break;    
  case SRLI:    
    state->reg[((instr_i_t *) instr)->rd] = srl(state->reg[((instr_i_t *) instr)->rs1], (((instr_i_t *) instr)->imm & 0b11111));
    break;    
  case SRAI:    
    state->reg[((instr_i_t *) instr)->rd] = sra(state->reg[((instr_i_t *) instr)->rs1], (((instr_i_t *) instr)->imm & 0b11111));
    break;
  case ADD:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] + state->reg[((instr_r_t *) instr)->rs2];
    break;
  case SUB:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] - state->reg[((instr_r_t *) instr)->rs2];
    break;
  case SLL:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] << state->reg[((instr_r_t *) instr)->rs2];
    break;
  case SLT:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] < state->reg[((instr_r_t *) instr)->rs2] ? 1 : 0;
    break;
  case SLTU:
    state->reg[((instr_r_t *) instr)->rd] = ((unsigned)state->reg[((instr_r_t *) instr)->rs1]) < ((unsigned)state->reg[((instr_r_t *) instr)->rs2]) ? 1 : 0;
    break;
  case XOR:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] ^ state->reg[((instr_r_t *) instr)->rs2];
    break;
  case SRL:
    state->reg[((instr_r_t *) instr)->rd] = srl(state->reg[((instr_r_t *) instr)->rs1], state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case SRA:
    state->reg[((instr_r_t *) instr)->rd] = sra(state->reg[((instr_r_t *) instr)->rs1], state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case OR:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] | state->reg[((instr_r_t *) instr)->rs2];
    break;
  case AND:
    state->reg[((instr_r_t *) instr)->rd] = state->reg[((instr_r_t *) instr)->rs1] & state->reg[((instr_r_t *) instr)->rs2];
    break;
  case FENCE:
    // TODO
    break;
    
    /////////
    // rv32m
    /////////
    // seems to be buggy due to my poor knowledge on C :cry:
    // tests should be prepared soon :-)
  case MUL:
    state->reg[((instr_r_t *) instr)->rd] = (((int64_t) state->reg[((instr_r_t *) instr)->rs1] * (int64_t) state->reg[((instr_r_t *) instr)->rs2])) & 0xFFFFFFFF;
    break;
  case MULH:
    state->reg[((instr_r_t *) instr)->rd] =
      ((((int64_t) state->reg[((instr_r_t *) instr)->rs1]
         * (int64_t) state->reg[((instr_r_t *) instr)->rs2]))
       & (((int64_t) 0xFFFFFFFF) << 32)) >> 32;
    break;
  case MULHSU:
    state->reg[((instr_r_t *) instr)->rd] =
      ((((int64_t) state->reg[((instr_r_t *) instr)->rs1]
         * (uint64_t) state->reg[((instr_r_t *) instr)->rs2]))
       & (((uint64_t) 0xFFFFFFFF) << 32)) >> 32;
    break;
  case MULHU:
    state->reg[((instr_r_t *) instr)->rd] =
      ((((uint64_t) state->reg[((instr_r_t *) instr)->rs1]
         * (uint64_t) state->reg[((instr_r_t *) instr)->rs2]))
       & (((uint64_t) 0xFFFFFFFF) << 32)) >> 32;
    break;
  case DIV:
    state->reg[((instr_r_t *) instr)->rd] = (((int32_t) state->reg[((instr_r_t *) instr)->rs1] / (int32_t) state->reg[((instr_r_t *) instr)->rs2]));
    break;
  case DIVU:
    state->reg[((instr_r_t *) instr)->rd] = (((uint32_t) state->reg[((instr_r_t *) instr)->rs1] / (uint32_t) state->reg[((instr_r_t *) instr)->rs2]));
    break;
  case REM:
    state->reg[((instr_r_t *) instr)->rd] = (((int32_t) state->reg[((instr_r_t *) instr)->rs1] % (int32_t) state->reg[((instr_r_t *) instr)->rs2]));
    break;
  case REMU:
    state->reg[((instr_r_t *) instr)->rd] = (((uint32_t) state->reg[((instr_r_t *) instr)->rs1] % (uint32_t) state->reg[((instr_r_t *) instr)->rs2]));
    break;

    /////////
    // :thinking_face:
    /////////
  default:    
    error("unimplemented instruction: %d", instr->op);
    exit_if_strict_mode(1);
    break;
  }

  // select the next pc
  state->pc = jump_enabled?  jump_dest : state->pc + 4;
  
  // finalize
  free(instr);
}
