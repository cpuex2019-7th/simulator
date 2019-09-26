#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "state.h"
#include "instr.h"
#include "disasm.h"

void print_instr(state_t *state){
  char *buf[4];
  fseek(state->pfp, (int) state->pc, SEEK_SET);
  fread(buf, 4, 4, state->pfp);
  int iraw = *(int*)buf;
  
  instr_t *instr = fetch_and_decode_once(state);

  char detail[100];
  disasm(instr, detail, 100);
  printf("%08x:\t%08x\t%s\n", state->pc, iraw, detail);

  free(instr);
}
void show_state(state_t* state){
  if(get_logging_level() > DEBUG)
    return;
  
  printf("------------------\n");
  printf("[*] Registers: \n");
  for (int i=0; i < 16; i++){
    printf("\tr%d\t%d\t/\tr%d\t%d\n", i, state->reg[i], i+16, state->reg[i+16]);
  }
  printf("[*] Next instruction: \n");
  print_instr(state);
  printf("------------------\n");
}

int run_debugger(state_t* state){
  if(get_logging_level() > DEBUG)
    return 0;

  printf("Breakpoint at %u\n", state->pc);
  show_state(state);
  while(1){
    char cmd[40];
    printf("> ");
    scanf("%40s", cmd);

    if(strcmp(cmd, "stepi") == 0){
      return 1;
    } else if(strcmp(cmd, "help") == 0){
      // TODO
    } else if(strncmp(cmd, "examine ", 8) == 0){
      // TODO
    } else if(strcmp(cmd, "next") == 0){
      break;
    } else if(strcmp(cmd, "quit") == 0){
      printf("Really? (y/n) > ");
      scanf("%40s", cmd);
      if(strcmp(cmd, "y") == 0)
        exit(1);
    } else {
      error("No such command. (`help` will help you!)");
    }
  }

  return 0;
}
