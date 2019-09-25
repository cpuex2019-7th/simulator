#include <stdio.h>
#include <stdlib.h>

#include "instr.h"


void set_r_instr(char *from, instr_t *to){
}
void set_i_instr(char *from, instr_t *to){
}
void set_s_instr(char *from, instr_t *to){
}
void set_b_instr(char *from, instr_t *to){
}
void set_u_instr(char *from, instr_t *to){
}
void set_j_instr(char *from, instr_t *to){
}

// use fp for future
// (it may allow us to decode the instructions included by rv32c easily?)
instr_t *fetch_and_decode_once(FILE* fp){
  instr_t *instr = malloc(sizeof(instr_t));
  
  char buf[4];
  fread(buf, 4, 4, fp);
  
  // opcode: buf[0]
    
  return instr;
}
