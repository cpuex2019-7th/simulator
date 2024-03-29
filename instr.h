#pragma once
#include <stdint.h>
#include <stdio.h>
#include "state.h"

typedef enum {
              /////////
              // rv32i
              /////////
              LUI,
              AUIPC,
              JAL,
              JALR,
              BEQ,
              BNE,
              BLT,
              BGE,
              BLTU,
              BGEU,
              LB,
              LH,
              LW,
              LBU,
              LHU,
              SB,
              SH,
              SW,
              ADDI,
              SLTI,
              SLTIU,
              XORI,
              ORI,
              ANDI,
              SLLI,
              SRLI,
              SRAI,
              ADD,
              SUB,
              SLL,
              SLT,
              SLTU,
              XOR,
              SRL,
              SRA,
              OR,
              AND,
              FENCE,
              // TODO,
              
              /////////
              // rv32m
              /////////
              MUL,
              MULH,
              MULHSU,
              MULHU,
              DIV,
              DIVU,
              REM,
              REMU,
              
              /////////
              // rv32f
              /////////
              FLW,
              FSW,
              FMVWX,
              FMVXW,
              FADDS,
              FSUBS,
              FMULS,
              FDIVS,
              FSQRTS,
              FEQS,
              FLES,
              FCVTWS,
              FCVTSW,
              FSGNJS,
              FSGNJNS,
              FSGNJXS, 
              
              /////////
              // for debug
              /////////
              INSTR_UNKNOWN
} instr_kind_t;

typedef struct {  
  int8_t funct7 : 7;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_r_t;

typedef struct {  
  int16_t imm : 12;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_i_t;

typedef struct {  
  int16_t imm : 12;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  instr_kind_t op : 7;
} instr_s_t;

typedef struct {  
  int16_t imm : 13;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  instr_kind_t op : 7;
} instr_b_t;

typedef struct {  
  int32_t imm : 32;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_u_t;

typedef struct {  
  int32_t imm : 21;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_j_t;

typedef struct {
  char pad[40];
  instr_kind_t op;
} instr_t;

void fetch_and_decode_once(state_t*, instr_t *);

// TODO: enhancement ... set ISA configurable (e.g. limitation on the ISA)

#define SIGNEXT(v, sb) ((v) | (((v) & (1 << (sb))) ? ~((1 << (sb))-1) : 0))

#define SIGNEXT_TOL(v, sb) ((v) | (((v) & (1 << (sb))) ? ~((1 << (sb))-1) : 0))
