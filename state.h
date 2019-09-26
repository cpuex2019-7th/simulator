#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 0x4000

typedef struct _breakpoint_list {
  uint32_t addr;  
  struct _breakpoint_list *next;
} blist_t;

typedef struct { 
  // program
  FILE *pfp;
  uint32_t pc;
  int is_running;
  size_t length;

  // basic structure
  int reg[32];
  float freg[32];
  uint8_t mem[MEM_SIZE];
  
  // for debug
  blist_t *blist;
    
  // for future
  FILE *ifp, *ofp;
} state_t;

void init_state(state_t *state, int, char**);
void write_reg(state_t*, int, int);
