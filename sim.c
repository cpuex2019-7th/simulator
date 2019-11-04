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
  if (state.sfp != NULL || get_logging_level() <= DEBUG){
    FILE *output = state.sfp != NULL? state.sfp : stdout;
    
    time_t t = time(NULL);
    fprintf(output, "---------------------\n");
    fprintf(output, "[*] Executed at: %s", ctime(&t));
    fprintf(output, "[*] Executable filename: %s\n", state.filename);
    fprintf(output, "[*] #(instructions): %lu\n", state.length/4);
    fprintf(output, "[*] #(total execution steps): %llu\n", state.step_num);
    fprintf(output, "[*] Range of values of registers (as signed int)\n");
    for(int i=0; i < 16; i++){
      fprintf(output, "\tx%02d: [%12d, %12d]\t/\tx%02d: [%12d, %12d]\n",
              i, state.reg_min[i], state.reg_max[i],
              i+16, state.reg_min[i+16], state.reg_max[i+16]);
    }    
    show_state(&state, output);
    
    if (state.slist != NULL){
      fprintf(output, "[*] #(function calls): \n");
      slist_t *seek = state.slist;
      while (seek != NULL){
        fprintf(output, "\t%s\tat 0x%08x: %u \n", seek->label, seek->offset, seek->called_num);
        seek = seek->next;
      }
    }
    
    fprintf(output, "---------------------\n");
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
