#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fenv.h>
#include <time.h>

#include "state.h"
#include "instr.h"
#include "logging.h"
#include "exec.h"
#include "debugger.h"
#include "stat.h"
#include "signal.h"

int main(int argc, char* argv[]){
  state_t state;
  set_stat_on_signal();
    
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
  if (state.sfp != NULL || get_logging_level() <= DEBUG){
    FILE *output = state.sfp != NULL? state.sfp : stderr;
    show_stat(output, &state);
  }

  // finalize
  if(state.ifp != NULL)
    fclose(state.ifp);
  if(state.ofp != NULL)
    fclose(state.ofp);

  printf("%d", state.reg[10]);
  return 0;
}
