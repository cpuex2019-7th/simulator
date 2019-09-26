#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "instr.h"
#include "logging.h"
#include "exec.h"
#include "debugger.h"

int main(int argc, char* argv[]){
  state_t state;
  set_logging_level(ERROR);
  init_state(&state, argc, argv);

  // main execution loop
  while(1){
    exec_hook_pre(&state);    
    exec_stepi(&state);
    if(exec_hook_post(&state) == 1)
      break;
  }

  // for debug
  show_state(&state);

  // finalize
  if(state.pfp != NULL)
    fclose(state.pfp);
  if(state.ifp != NULL)
    fclose(state.ifp);
  if(state.ofp != NULL)
    fclose(state.ofp);

  return 0;
}
