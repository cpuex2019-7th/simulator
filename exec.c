#include <math.h>

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

void exit_if_strict_mode(state_t *state, int code){
  if (is_strict){
    // for debug
    show_state(state);    
    exit(code);
  }
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

int exec_hook_pre(state_t *state){
  if(state->length < state->pc+4 || state->pc == INITIAL_X1){
    info("Execution Finished.");
    state->is_running = 0;
    // for debug
    show_state(state);
    return 1;
  }  
  
  info("Executing next instruction ... (PC=%08x)", state->pc);
  if(is_step_execution_enabled == 1
     || is_here_breakpoint(state) == 1){
    is_step_execution_enabled = run_debugger(state);
  }

  return 0;
}

void exec_hook_post(state_t *state){
  // TODO
}

void exec_stepi(state_t *state){
  // fetch & decode
  instr_t *instr = fetch_and_decode_once(state);


  // jump destination
  int jump_dest = 0;
  int jump_enabled = 0;

  int s_addr = state->reg[((instr_s_t *) instr)->rs1] + ((instr_s_t *) instr)->imm;
  int i_addr = state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm;
  int tmp = 0;
  
  // exec & write
  switch(instr->op){
    /////////
    // rv32i
    /////////
  case LUI:
    write_reg(state,
              ((instr_u_t *) instr)->rd,
              ((instr_u_t *) instr)->imm);
    break;
  case AUIPC:
    // state->pc is the actual address of AUIPC
    // TODO: MMU
    write_reg(state,
              ((instr_u_t *) instr)->rd,
              ((instr_u_t *) instr)->imm + state->pc);
    break;
  case JAL:
    jump_enabled = 1;
    jump_dest = ((instr_j_t *) instr)->imm + state->pc;
    write_reg(state,
              ((instr_j_t *) instr)->rd,
              state->pc + 4);
    break;
  case JALR:
    jump_enabled = 1;
    jump_dest = (((instr_i_t *) instr)->imm + state->reg[((instr_i_t *) instr)->rs1]) & (~0b1);
    write_reg(state,
              ((instr_j_t *) instr)->rd,
              state->pc + 4);
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
    // TODO: refactor here with nested switch
  case LB:    
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Byte): from %08x\n", i_addr);
      write_reg(state,
                ((instr_i_t *) instr)->rd,
                SIGNEXT(*(uint8_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm), 7));
    } else {
      debug("[*] UART read (Byte): %08x\n", i_addr);
      switch(i_addr & 0b11111111){
      case 0:
        if (state->ifp == NULL){
          error("No input file was specified.");
          exit(1);
        }
        tmp = fgetc(state->ifp);
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  SIGNEXT(tmp, 7));
        break;
      case 8:
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  get_uart_status(state));
        break;
      default:
        error("Invalid UART address: %08x");
        exit(1);
      }
    }
    break;
  case LH:
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Half Word): from %08x\n", i_addr);
      write_reg(state,
                ((instr_i_t *) instr)->rd,
                SIGNEXT(*(uint16_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm), 15));
    } else {
      debug("[*] UART read (Half Word, but lower 1 byte will be read): %08x\n", i_addr & 0b11111111);
      switch(i_addr & 0b11111111){
      case 0:
        if (state->ifp == NULL){
          error("No input file was specified.");
          exit(1);
        }
        tmp = fgetc(state->ifp);
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  SIGNEXT(tmp, 7));
        break;
      case 8:
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  get_uart_status(state)); 
        break;
      default:
        error("Invalid UART address");
        exit(1);
      }
    }
    break;
  case LW:
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Word): from %08x\n", i_addr);
      write_reg(state,
                ((instr_i_t *) instr)->rd,
                *(uint32_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
    } else {
      debug("[*] UART read (Word, but lower 1 byte will be read): %08x\n", i_addr & 0b11111111);
      switch(i_addr & 0b11111111){
      case 0:
        if (state->ifp == NULL){
          error("No input file was specified.");
          exit(1);
        }
        tmp = fgetc(state->ifp);
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  SIGNEXT(tmp, 7));
        break;
      case 8:
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  get_uart_status(state));
        break;
      default:
        error("Invalid UART address");
        exit(1);
      }
    }
    break;
  case LBU:
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Byte Unsigned): from %08x\n", i_addr);
      write_reg(state,
                ((instr_i_t *) instr)->rd,
                (uint32_t) *(uint8_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
      // no sign extention
    } else {
      debug("[*] UART read (Byte Unsigned, but lower 1 byte will be read): %08x\n", i_addr & 0b11111111);
      switch(i_addr & 0b11111111){
      case 0:
        if (state->ifp == NULL){
          error("No input file was specified.");
          exit(1);
        }
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  fgetc(state->ifp));
        break;
      case 8:
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  get_uart_status(state));
        break;
      default:
        error("Invalid UART address");
        exit(1);
      }
    }
    break;
  case LHU:
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Half Word Unsigned): from %08x\n", i_addr);
      write_reg(state,
                ((instr_i_t *) instr)->rd,
                (uint32_t) *(uint16_t *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
      // no sign extention
    } else {
      debug("[*] UART read (Half Word Unsigned, but lower 1 byte will be read): %08x\n", i_addr & 0b11111111);
      switch(i_addr & 0b11111111){
      case 0:
        if (state->ifp == NULL){
          error("No input file was specified.");
          exit(1);
        }
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  fgetc(state->ifp));
        break;
      case 8:
        write_reg(state,
                  ((instr_i_t *) instr)->rd,
                  get_uart_status(state));
        break;
      default:
        error("Invalid UART address");
        exit(1);
      }
    }
    break;
  case SB:
    if((s_addr >> 24) != 0x7F){
      debug("[*] Mem write (Byte): %02x to %08x\n",
            state->reg[((instr_s_t *) instr)->rs2] & 0xFF,
            s_addr);    
      state->mem[s_addr] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
    } else {
      debug("[*] UART write (Byte): %08x\n", s_addr);
      switch(s_addr & 0b11111111){
      case 4:
        if (state->ofp == NULL){
          error("No output file was specified.");
          exit(1);
        }
        fprintf(state->ofp, "%1c", state->reg[((instr_s_t *) instr)->rs2] & 0b11111111);
        fflush(state->ofp);
        break;
      case 0xc:
        // TODO
        break;
      default:
        error("Invalid UART address");
        exit(1);       
      }
    }
    break;
  case SH:
    if((s_addr >> 24) != 0x7F){
      debug("[*] Mem write (Half word): %04x to %08x\n",
            state->reg[((instr_s_t *) instr)->rs2] & 0x00FF,
            s_addr);
      state->mem[s_addr] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
      state->mem[s_addr+1] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 8;
    } else {
      debug("[*] UART write (Half word, but lower 1 byte will be written): %08x\n", s_addr);
      switch(s_addr & 0b11111111){
      case 4:
        if (state->ofp == NULL){
          error("No output file was specified.");
          exit(1);
        }
        fprintf(state->ofp, "%1c", state->reg[((instr_s_t *) instr)->rs2] & 0b11111111);
        fflush(state->ofp);
        break;
      case 0xc:
        // TODO
        break;
      default:
        error("Invalid UART address");
        exit(1);       
      }
    }
    break;
  case SW:
    if((s_addr >> 24) != 0x7F){
      debug("[*] Mem write (Word): %08x to %08x (little endian: %02x %02x %02x %02x)\n",
            state->reg[((instr_s_t *) instr)->rs2],
            s_addr,
            state->reg[((instr_s_t *) instr)->rs2] & 0b11111111,
            (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 8,
            (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 16)) >> 16,
            (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 24)) >> 24);
      state->mem[s_addr] = state->reg[((instr_s_t *) instr)->rs2] & 0b11111111;
      state->mem[s_addr+1] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 8)) >> 8;
      state->mem[s_addr+2] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 16)) >> 16;
      state->mem[s_addr+3] = (state->reg[((instr_s_t *) instr)->rs2] & (0b11111111 << 24)) >> 24;
    } else {
      debug("[*] UART write (Word, but lower 1 byte will be written): %08x\n", s_addr);
      switch(s_addr & 0b11111111){
      case 4:
        if (state->ofp == NULL){
          error("No output file was specified.");
          exit(1);
        }
        fprintf(state->ofp, "%1c", state->reg[((instr_s_t *) instr)->rs2] & 0b11111111);
        fflush(state->ofp);
        break;
      case 0xc:
        // TODO
        break;
      default:
        error("Invalid UART address");
        exit(1);       
      }
    }
    break;    
  case ADDI:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm);
    break;
  case SLTI:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] < ((instr_i_t *) instr)->imm? 1 : 0);
    break;
  case SLTIU:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              ((unsigned)state->reg[((instr_i_t *) instr)->rs1]) < ((unsigned)((instr_i_t *) instr)->imm)? 1 : 0);
    break;
  case XORI:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] ^ ((instr_i_t *) instr)->imm);
    break;
  case ORI:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] | ((instr_i_t *) instr)->imm);
    break;
  case ANDI:
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] & ((instr_i_t *) instr)->imm);
    break;
  case SLLI:    
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              state->reg[((instr_i_t *) instr)->rs1] << (((instr_i_t *) instr)->imm & 0b11111));
    break;    
  case SRLI:    
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              srl(state->reg[((instr_i_t *) instr)->rs1], (((instr_i_t *) instr)->imm & 0b11111)));
    break;    
  case SRAI:    
    write_reg(state,
              ((instr_i_t *) instr)->rd,
              sra(state->reg[((instr_i_t *) instr)->rs1], (((instr_i_t *) instr)->imm & 0b11111)));
    break;
  case ADD:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] + state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case SUB:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] - state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case SLL:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] << state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case SLT:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] < state->reg[((instr_r_t *) instr)->rs2] ? 1 : 0);
    break;
  case SLTU:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              ((unsigned)state->reg[((instr_r_t *) instr)->rs1]) < ((unsigned)state->reg[((instr_r_t *) instr)->rs2]) ? 1 : 0);
    break;
  case XOR:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] ^ state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case SRL:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              srl(state->reg[((instr_r_t *) instr)->rs1], state->reg[((instr_r_t *) instr)->rs2]));
    break;
  case SRA:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              sra(state->reg[((instr_r_t *) instr)->rs1], state->reg[((instr_r_t *) instr)->rs2]));
    break;
  case OR:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] | state->reg[((instr_r_t *) instr)->rs2]);
    break;
  case AND:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->reg[((instr_r_t *) instr)->rs1] & state->reg[((instr_r_t *) instr)->rs2]);
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
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (((int64_t) state->reg[((instr_r_t *) instr)->rs1] * (int64_t) state->reg[((instr_r_t *) instr)->rs2])) & 0xFFFFFFFF);
    break;
  case MULH:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              ((((int64_t) state->reg[((instr_r_t *) instr)->rs1]
                 * (int64_t) state->reg[((instr_r_t *) instr)->rs2]))
               & (((int64_t) 0xFFFFFFFF) << 32)) >> 32);
    break;
  case MULHSU:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              ((((int64_t) state->reg[((instr_r_t *) instr)->rs1]
                 * (uint64_t) ((uint32_t) state->reg[((instr_r_t *) instr)->rs2])))
               & (((uint64_t) 0xFFFFFFFF) << 32)) >> 32);
    break;
  case MULHU:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              ((((uint64_t) ((uint32_t) state->reg[((instr_r_t *) instr)->rs1])
                 * (uint64_t) ((uint32_t) state->reg[((instr_r_t *) instr)->rs2])))
               & (((uint64_t) 0xFFFFFFFF) << 32)) >> 32);
    break;
  case DIV:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (((int32_t) state->reg[((instr_r_t *) instr)->rs1] / (int32_t) state->reg[((instr_r_t *) instr)->rs2])));
    break;
  case DIVU:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (((uint32_t) state->reg[((instr_r_t *) instr)->rs1] / (uint32_t) state->reg[((instr_r_t *) instr)->rs2])));
    break;
  case REM:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (((int32_t) state->reg[((instr_r_t *) instr)->rs1] % (int32_t) state->reg[((instr_r_t *) instr)->rs2])));
    break;
  case REMU:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (((uint32_t) state->reg[((instr_r_t *) instr)->rs1] % (uint32_t) state->reg[((instr_r_t *) instr)->rs2])));
    break;
    
    /////////
    // rv32f
    /////////

  case FLW:
    if((i_addr >> 24) != 0x7F){
      debug("Mem read (Float Word): from %08x\n", i_addr);      
      write_freg(state,
                 ((instr_i_t *) instr)->rd,
                 *(float *)(state->mem + state->reg[((instr_i_t *) instr)->rs1] + ((instr_i_t *) instr)->imm));
    } else {
      error("[*] UART read with FLW is prohibited.");
      exit(1);
    }
    break;
  case FSW:
    if((s_addr >> 24) != 0x7F){
      debug("[*] Mem write (Float Word): %08x to %08x (little endian: %02x %02x %02x %02x)\n",
            state->freg[((instr_s_t *) instr)->rs2],
            s_addr,
            state->freg[((instr_s_t *) instr)->rs2].i & 0b11111111,
            (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 8)) >> 8,
            (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 16)) >> 16,
            (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 24)) >> 24);
      state->mem[s_addr] = state->freg[((instr_s_t *) instr)->rs2].i & 0b11111111;
      state->mem[s_addr+1] = (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 8)) >> 8;
      state->mem[s_addr+2] = (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 16)) >> 16;
      state->mem[s_addr+3] = (state->freg[((instr_s_t *) instr)->rs2].i & (0b11111111 << 24)) >> 24;
    } else {
      error("[*] UART write with SLW is prohibited.");
      exit(1);
    }
    break;    
  case FMVWX:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               ((freg_float) state->reg[((instr_r_t *) instr)->rs1]).f);
    break;
  case FMVXW:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->freg[((instr_r_t *) instr)->rs1].i);
    break;
  case FADDS:
    write_freg(state,
              ((instr_r_t *) instr)->rd,
              state->freg[((instr_r_t *) instr)->rs1].f + state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FSUBS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               state->freg[((instr_r_t *) instr)->rs1].f - state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FMULS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               state->freg[((instr_r_t *) instr)->rs1].f * state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FDIVS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               state->freg[((instr_r_t *) instr)->rs1].f / state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FSQRTS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               sqrtf(state->freg[((instr_r_t *) instr)->rs1].f));
    break;
  case FEQS:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->freg[((instr_r_t *) instr)->rs1].f == state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FLES:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              state->freg[((instr_r_t *) instr)->rs1].f < state->freg[((instr_r_t *) instr)->rs2].f);
    break;
  case FCVTWS:
    write_reg(state,
              ((instr_r_t *) instr)->rd,
              (int) (state->freg[((instr_r_t *) instr)->rs1].f));
    break;
  case FCVTSW:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               (float) (state->reg[((instr_r_t *) instr)->rs1]));
    break;
  case FSGNJS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               (state->freg[((instr_r_t *) instr)->rs2].f >= 0? +1.0 : -1.0) * fabsf(state->freg[((instr_r_t *) instr)->rs1].f));
    break;
  case FSGNJNS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               (state->freg[((instr_r_t *) instr)->rs2].f >= 0? -1.0 : +1.0) * fabsf(state->freg[((instr_r_t *) instr)->rs1].f));
    break;
  case FSGNJXS:
    write_freg(state,
               ((instr_r_t *) instr)->rd,
               ((freg_float)(((srl(state->freg[((instr_r_t *) instr)->rs1].i, 31) ^ srl(state->freg[((instr_r_t *) instr)->rs2].i, 31)) << 31) & (state->freg[((instr_r_t *) instr)->rs1].i & ~(0b1 << 31)))).f);
    break;
    
    /////////
    // :thinking_face:
    /////////
  default:    
    error("unimplemented instruction: %d", instr->op);
    exit_if_strict_mode(state, 1);
    break;
  }

  // select the next pc
  state->pc = jump_enabled?  jump_dest : state->pc + 4;
  
  // finalize
  free(instr);
}
