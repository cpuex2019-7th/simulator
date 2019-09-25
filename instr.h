#pragma once
#include <stdio.h>

typedef enum {
             NOP
} instr_kind_t;


typedef struct {  
  instr_kind_t op;
} instr_r_t;

typedef struct {  
  instr_kind_t op;
} instr_i_t;

typedef struct {  
  instr_kind_t op;
} instr_s_t;

typedef struct {  
  instr_kind_t op;
} instr_b_t;

typedef struct {  
  instr_kind_t op;
} instr_u_t;

typedef struct {  
  instr_kind_t op;
} instr_j_t;

typedef struct {
  instr_kind_t op;
  char pad[4-sizeof(instr_kind_t)];
} instr_t;

instr_t *fetch_and_decode_once(FILE*);
