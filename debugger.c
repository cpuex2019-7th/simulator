#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "state.h"

int run_debugger(state_t* state){
  printf("------------------");
  printf("Breakpoint at %u\n", state->pc);
  printf("Registers: \n");
  for (int i=0; i < 16; i++){
    printf("\tr%d\t%d\t/\tr%d\t%d\n", i, state->reg[i], i+16, state->reg[i+16]);
  }
  printf("------------------");
  while(1){
    char cmd[40];
    printf("> ");
    scanf("%40s", cmd);

    if(strncmp(cmd, "stepi ", 6) == 0){
      return 1;
    } else if(strncmp(cmd, "help ", 5) == 0){
      // TODO
    } else if(strncmp(cmd, "examine ", 8) == 0){
      // TODO
    } else if(strncmp(cmd, "next ", 5) == 0){
      break;
    } else if(strncmp(cmd, "quit ", 5) == 0){
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
