#include <stdio.h>
#include <time.h>
#include "state.h"
#include "debugger.h"

void show_stat(FILE* output, state_t *state){
  time_t t = time(NULL);
  fprintf(output, "---------------------\n");
  fprintf(output, "[*] Executed at: %s", ctime(&t));
  fprintf(output, "[*] Executable filename: %s\n", state->filename);
  fprintf(output, "[*] #(instructions): %lu\n", state->length/4);
  fprintf(output, "[*] #(total execution steps): %llu\n", state->step_num);
  fprintf(output, "[*] Range of values of registers (as signed int)\n");
  for(int i=0; i < 16; i++){
    fprintf(output, "\tx%02d: [%12d, %12d]\t/\tx%02d: [%12d, %12d]\n",
            i, state->reg_min[i], state->reg_max[i],
            i+16, state->reg_min[i+16], state->reg_max[i+16]);
  }    
  show_state(state, output);
    
  if (state->slist != NULL){
    fprintf(output, "[*] #(function calls): \n");
    slist_t *seek = state->slist;
    while (seek != NULL){
      fprintf(output, "\t%s\tat 0x%08x: %u \n", seek->label, seek->addr, seek->called_num);
      seek = seek->next;
    }
  }    
  fprintf(output, "---------------------\n");
}
