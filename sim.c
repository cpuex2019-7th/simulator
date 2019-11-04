#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fenv.h>

#include "state.h"
#include "instr.h"
#include "logging.h"
#include "exec.h"
#include "debugger.h"

int main(int argc, char* argv[]){
  state_t state;
  set_logging_level(ERROR);
  fesetround(FE_TONEAREST);
  init_state(&state, argc, argv);

  // main execution loop
  while(state.is_running){
    exec_hook_pre(&state);
    exec_stepi(&state);
    exec_hook_post(&state);
  }
  
  // output statistics
  if (state.sfp != NULL){
    fprintf(state.sfp, "Input: %s\n", state.filename);
    fprintf(state.sfp, "Total Execution Steps: %d\n", state.step_num);
  }

  // finalize
  if(state.pfp != NULL)
    fclose(state.pfp);
  if(state.ifp != NULL)
    fclose(state.ifp);
  if(state.ofp != NULL)
    fclose(state.ofp);

  printf("%d", state.reg[10]);
  return 0;
}
