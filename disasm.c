#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instr.h"
#include "disasmutil.h"

int main(int argc, char* argv[]){
  state_t state;
  int verbose_level = 0;

  // load state
  for (int i=1; i < argc; i++){
    if (strcmp(argv[i], "-v")==0){
      verbose_level = 1;
    } else if (strcmp(argv[i], "-vv")==0){
      verbose_level = 2;
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
      FILE *pfp = fopen(argv[i], "rb");
      if(pfp == NULL){
        printf("Failed to open specified executable.");
        exit(1);
      }
      // load program & set program length
      fseek(pfp, 0L, SEEK_END);
      state.length = ftell(pfp);
      fseek(pfp, 0, SEEK_SET);
  
      state.prog = malloc(state.length * sizeof(int));
      for(int i=0; i < (int)(state.length/4); i++)
        fread(&(state.prog[i]), 4, 1, pfp);
    }
  }

  // validate
  if (state.prog == NULL){
    printf("%s <file to disasm>", argv[0]);
    exit(1);
  }

  // main loop
  for(state.pc=0; state.pc<state.length; state.pc+=4){    
    int iraw = state.prog[state.pc/4];  
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
    switch (verbose_level){
    case 1:
      printf("0x%08x:\t0x%08x\t%s\n", state.pc, iraw, detail);
      break;
    case 2:
      printf("0x%08x(%20s+0x%08x):\t0x%08x\t%s\n", state.pc, label, offset, iraw, detail);
      break;
    default:
      printf("%s\n", detail);

    }
    free(instr);
  }
  return 0;
}
