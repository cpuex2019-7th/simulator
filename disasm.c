#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instr.h"
#include "disasmutil.h"

int main(int argc, char* argv[]){
  state_t state;
  state.pfp = NULL;
  int is_verbose = 0;

  // load state
  for (int i=1; i < argc; i++){
    if (strcmp(argv[i], "-v")==0){
      is_verbose = 1;
    } else if (strcmp(argv[i], "--symbols") == 0){
      // loading symbol information
      ///////////////
      if (i == argc-1){
        fprintf(stderr, "No symbol list is specified.");
        exit(1);
      }
      FILE *slistfp = fopen(argv[++i], "r");
      if (slistfp == NULL){
        fprintf(stderr, "Cannot open the file you given: %s", argv[i]);
        exit(1);
      }
      
      char symbol_buf[80];
      int addr_buf;
      while (fscanf(slistfp, "%s %d", symbol_buf, &addr_buf) == 2 ) {
        insert_new_symbol(&(state.slist), symbol_buf, addr_buf);
      }      
    } else {      
      state.pfp = fopen(argv[i], "rb");
      if(state.pfp == NULL){
        printf("Failed to open specified executable.");
        exit(1);
      }
      fseek(state.pfp, 0L, SEEK_END);
      state.length = ftell(state.pfp);
    }
  }

  // validate
  if (state.pfp == NULL){
    printf("%s <file to disasm>", argv[0]);
    exit(1);
  }

  // main loop
  for(state.pc=0; state.pc<state.length; state.pc+=4){    
    char *buf[4];
    fseek(state.pfp, (int) state.pc, SEEK_SET);
    fread(buf, 4, 4, state.pfp);
    int iraw = *(int*)buf;
    instr_t *instr = fetch_and_decode_once(&state);
    
    char detail[100];
    disasm(instr, state.pc, detail, 100);

    
    // resolve label information with .symbols file
    char *label = NULL;
    uint32_t offset = 0;
    get_label_and_offset(state.slist, state.pc, &label, &offset);
    if (label == NULL){
      label = "<filebase>";
      offset = state.pc;
    }
    
    // show result
    if (is_verbose){
      printf("0x%08x(%20s+0x%08x):\t0x%08x\t%s\n", state.pc, label, offset, iraw, detail);
    } else {
      printf("%s\n", detail);
    }

    free(instr);
  }
  return 0;
}
