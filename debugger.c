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
  printf("0x%08x:\t0x%08x\t%s\n", state->pc, iraw, detail);

  free(instr);
}
void show_state(state_t* state){
  if(get_logging_level() > DEBUG)
    return;
  
  printf("------------------\n");
  printf("[*] Registers: \n");
  for (int i=0; i < 16; i++){
    printf("\tr%d\t0x%08x\t/\tr%d\t0x%08x\n",
           i,
           state->reg[i],
           i+16,
           state->reg[i+16]);
  }
  printf("[*] Next instruction: \n");
  print_instr(state);
  printf("------------------\n");
}


static char *previous_cmd = NULL;

int run_debugger(state_t* state){
  if(get_logging_level() > DEBUG)
    return 0;

  printf("Breakpoint at %u\n", state->pc);
  show_state(state);
  while(1){
    printf("> ");
    
    size_t len = 0;
    char *cmd = NULL;
    getline(&cmd, &len, stdin);
    cmd[strlen(cmd)-1] = '\0';
    
    // log the command
    if(strcmp(cmd, "") == 0){
      free(cmd);
      cmd = previous_cmd;
    } else {
      if (previous_cmd != NULL){        
        free(previous_cmd);
      }
      previous_cmd = cmd;
    }

    // exec
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
      error("No such command: %s. (`help` will help you!)", cmd);
    }
  }

  return 0;
}
