#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "symbols.h"
#include "breakpoint.h"

#define INITIAL_X1 0xff000000
#define MEM_SIZE 0x2000000

typedef union {
  float f;
  int i;
} freg_float;

typedef struct { 
  // program
  char *filename;
  int *prog;
  
  uint32_t pc;
  int is_running;
  int is_first_output_done;
  size_t length;
  
  // basic structure
  int reg[32];
  int reg_min[32];
  int reg_max[32];
  freg_float freg[32];

  int memsize;
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

void init_state(state_t*, int, char**);
void write_reg(state_t*, int, int);
void write_freg(state_t *, int, float);
void write_mem(state_t *, int, char);
uint8_t read_mem_uint8(state_t *, int);
uint16_t read_mem_uint16(state_t *, int);
uint32_t read_mem_uint32(state_t *, int);
float read_mem_float(state_t *, int);

int get_uart_status(state_t*);
