#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_X1 0xff000000
#define MEM_SIZE 0x2000000

typedef union {
  float f;
  int i;
} freg_float;

typedef struct _breakpoint_list {
  uint32_t addr;  
  struct _breakpoint_list *next;
} blist_t;

typedef struct _symbol_list {
  char *label;
  // where is this symbol?
  uint32_t offset;
  // how many times jump instruction for this symbol occured?
  uint32_t called_num; 
  struct _symbol_list *next;
} slist_t;

typedef struct { 
  // program
  char *filename;
  FILE *pfp;
  uint32_t pc;
  int is_running;
  size_t length;
  
  // basic structure
  int reg[32];
  int reg_min[32];
  int reg_max[32];
  freg_float freg[32];
  
  uint8_t *mem;
  
  // breakpoints
  blist_t *blist;

  // symbols
  // TODO: use hashmap
  slist_t *slist;

  // stats
  unsigned long long int step_num;

  // file pointer to output stats
  FILE *sfp;
  
  // UART in/out
  FILE *ifp, *ofp;
} state_t;

void init_state(state_t *state, int, char**);
void write_reg(state_t*, int, int);
void write_freg(state_t *, int, float);
void write_mem(state_t *, int, char);
uint8_t read_mem_uint8(state_t *, int);
uint16_t read_mem_uint16(state_t *, int);
uint32_t read_mem_uint32(state_t *, int);
float read_mem_float(state_t *, int);

int get_uart_status(state_t*);
