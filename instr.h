#pragma once
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
              UNKNOWN
} instr_kind_t;

typedef struct {  
  uint8_t funct7 : 7;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_r_t;

typedef struct {  
  uint16_t imm : 12;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_i_t;

typedef struct {  
  uint16_t imm : 12;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  instr_kind_t op : 7;
} instr_s_t;

typedef struct {  
  uint16_t imm : 12;
  uint8_t rs2 : 5;
  uint8_t rs1 : 5;
  uint8_t funct3 : 3;
  instr_kind_t op : 7;
} instr_b_t;

typedef struct {  
  uint32_t imm : 32;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_u_t;

typedef struct {  
  uint32_t imm : 20;
  uint8_t rd : 5;
  instr_kind_t op : 7;
} instr_j_t;

typedef struct {
  char pad[sizeof(instr_r_t)-sizeof(instr_kind_t)];
  instr_kind_t op;
} instr_t;

instr_t *fetch_and_decode_once(state_t*);

#define SIGNEXT(v, sb) ((v) | (((v) & (1 << (sb))) ? ~((1 << (sb))-1) : 0))

