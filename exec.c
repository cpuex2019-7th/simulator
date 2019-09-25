#include "state.h"
#include "instr.h"

void exec_hook_pre(state_t *state){
  // TODO: process breakpoint
  info("Executing next instruction ...");
}

int exec_hook_post(state_t *state){
  // TODO: check whether the program should exit
  info("Finished.");
  return 0;
}

void exec_stepi(state_t *state){
  instr_t *instr = fetch_and_decode_once(state->pfp);  
  free(instr);
}

