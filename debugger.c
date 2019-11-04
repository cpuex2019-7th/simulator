#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "state.h"
#include "instr.h"
#include "disasmutil.h"

void print_current_instr(state_t *state){
  char *buf[4];
  fseek(state->pfp, (int) state->pc, SEEK_SET);
  fread(buf, 4, 4, state->pfp);
  int iraw = *(int*)buf;  
  instr_t *instr = fetch_and_decode_once(state);

  // disassemble
  char detail[100];
  disasm(instr, state->pc, detail, 100);

  // resolve label information with .symbols file
  // resolve label information with .symbols file
  char *label = NULL;
  uint32_t offset = 0;
  get_label_and_offset(state->slist, state->pc, &label, &offset);
    
  if (label == NULL){
    label = "<filebase>";
    offset = state->pc;
  }
  printf("0x%08x(%20s+0x%08x):\t0x%08x\t%s\n", state->pc, label, offset, iraw, detail);

  free(instr);
}

void show_state(state_t* state, FILE *fp){
  fprintf(fp, "[*] Registers: \n");
  for (int i=0; i < 16; i++){
    fprintf(fp, "\tx%d\t(%s)\t\t0x%08x\t/\tx%d\t(%s)\t0x%08x\n",
            i,
            r2t(i),
            state->reg[i],
            i+16,
            r2t(i+16),
            state->reg[i+16]);
  }
  for (int i=0; i < 16; i++){
    fprintf(fp, "\tf%d\t(%s)\t\t%f\t/\tf%d\t(%s)\t%f\n",
            i,
            r2tf(i),
            state->freg[i].f,
            i+16,
            r2tf(i+16),
            state->freg[i+16].f);
  }
  if(state->pc < state->length){
    fprintf(fp, "[*] Next instruction: \n");  
    print_current_instr(state);
  }
}


static char *previous_cmd = NULL;

// returns ...
// 0 when step execution finishes
// 1 when step execution continues
int run_debugger(state_t* state){
  if(get_logging_level() > DEBUG)
    return 0;

  // show current status
  printf("Stopped at %u\n", state->pc);
  printf("------------------\n");  
  show_state(state, stdout);
  printf("------------------\n");

  // simple cmd interface
  while(1){
    printf("> ");
    
    size_t len = 0;
    char *cmd = NULL;
    getline(&cmd, &len, stdin);
    cmd[strlen(cmd)-1] = '\0';
    
    // log the command
    if(strcmp(cmd, "") == 0){
      free(cmd);
      if (previous_cmd != NULL){
        cmd = previous_cmd;
      } else {
        cmd = "help";
      }
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
      printf("stepi, help, examine, stack, next, quit\n");
    } else if(strncmp(cmd, "examine ", 8) == 0){
      // TODO
    } else if(strcmp(cmd, "stack") == 0){
      unsigned sp = state->reg[2];
      unsigned fp = state->reg[8];
      for (unsigned i=0; i < fp-sp; i += 8){
        printf("%08x: %02x %02x %02x %02x %02x %02x %02x %02x\n",
               sp+i,
               state->mem[sp+i+0],
               state->mem[sp+i+1],
               state->mem[sp+i+2],
               state->mem[sp+i+3],
               state->mem[sp+i+4],
               state->mem[sp+i+5],
               state->mem[sp+i+6],
               state->mem[sp+i+7]);
      }
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
