#include "state.h"
#include "instr.h"
#include "debugger.h"
#include "logging.h"

static int is_step_execution_enabled = 0;

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
  instr_t *instr = fetch_and_decode_once(state);  
  free(instr);
}

