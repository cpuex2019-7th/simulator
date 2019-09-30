#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instr.h"
#include "disasmutil.h"

int main(int argc, char* argv[]){
  if(argc < 2){
    printf("%s <file to disasm>", argv[0]);
  }
  state_t state;
  state.pfp = fopen(argv[1], "rb");
  if(state.pfp == NULL){
    printf("Failed to open specified executable.");
    exit(1);
  }
  fseek(state.pfp, 0L, SEEK_END);
  state.length = ftell(state.pfp);
  
  for(state.pc=0; state.pc<state.length; state.pc+=4){
    char *buf[4];
    fseek(state.pfp, (int) state.pc, SEEK_SET);
    fread(buf, 4, 4, state.pfp);
    int iraw = *(int*)buf;
  
    instr_t *instr = fetch_and_decode_once(&state);
    char detail[100];
    disasm(instr, state.pc, detail, 100);
    printf("0x%08x:\t0x%08x\t%s\n", state.pc, iraw, detail);

    free(instr);
  }
  return 0;
}
