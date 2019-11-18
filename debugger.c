#include <stdio.h>
#include <string.h>
#include "logging.h"
#include "state.h"
#include "instr.h"
#include "disasmutil.h"
#include "exec.h"
#include "breakpoint.h"


void get_pretty_location(slist_t *slist, uint32_t addr, char *buf){
  char *label = NULL;
  uint32_t offset = 0;
  get_label_and_offset(slist, addr, &label, &offset);
      
  if (label == NULL){
    label = "<filebase>";
    offset = addr;
  }
  sprintf(buf, "%20s+0x%08x", label, offset);  
}

void print_current_instr(state_t *state){
  int iraw = state->prog[state->pc/4];  
  instr_t instr;
  fetch_and_decode_once(state, &instr);

  // disassemble
  char detail[100];
  disasm(&instr, state->pc, detail, 100);

  // resolve label information with .symbols file
  char location[100];
  get_pretty_location(state->slist, state->pc, location);
  printf("0x%08x(%s):\t0x%08x\t%s\n", state->pc, location, iraw, detail);
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

execution_mode_t run_debugger(state_t* state){
  // show current status
  printf("Stopped at %u\n", state->pc);
  printf("#(total execution steps): %llu\n", state->step_num);
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
        cmd = "continue";
      }
    } else {
      if (previous_cmd != NULL){        
        free(previous_cmd);
      }
      previous_cmd = cmd;
    }

    // exec
    if(strcmp(cmd, "stepi") == 0){
      return STEP;
    } else if(strcmp(cmd, "help") == 0){
      printf("stepi, help, examine, stack, next, quit\n");
    } else if(strncmp(cmd, "examine ", 8) == 0){
      // TODO
    } else if(strncmp(cmd, "b ", 2) == 0){
      char label[80];
      uint32_t addr;
      int addr_base;
      uint32_t offset = 0;
      
      if(1 == sscanf(cmd, "b 0x%x", &addr)){
      } else if (2 == sscanf(cmd, "b %s 0x%x", label, &offset)
                 || 2 == sscanf(cmd, "b %s %d", label, &offset)
                 || 1 == sscanf(cmd, "b %s", label)){
        addr_base = get_addr_from_label(state->slist, label);
        if (addr_base == -1){
          error("No such label: %s", label);
          continue;
        } else {
          addr = addr_base + offset;
        }
      } else {
        error("Undefined command format: %s", cmd);
        continue;
      }

      insert_new_breakpoint(&(state->blist), addr);
      printf("Set breakpoint: 0x%08x\n", addr);
    } else if(strcmp(cmd, "i b") == 0){
      blist_t *seek = state->blist;
      int i=0;
      while(seek != NULL){
        char location[100];
        get_pretty_location(state->slist, seek->addr, location);
        printf("- %d: 0x%08x (%s)\n", i, seek->addr, location);
        seek = seek->next;
        i++;
      }      
    } else if(strncmp(cmd, "d ", 2) == 0){
      int i=0;
      sscanf(cmd, "d %d", &i);      
      delete_breakpoint_by_id(&(state->blist), i);
     
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
    } else if(strcmp(cmd, "history") == 0){
      // TODO: fix this dirty hack
      uint32_t pc = state->pc;
      printf("-----latest-----\n");
      for (int i=0; i < HIST_SIZE; i++){
        state->pc = state->history[i];
        print_current_instr(state);
      }
      printf("-----oldest-----\n");
      state->pc = pc;
    } else if(strcmp(cmd, "next") == 0){
      break;
    } else if(strcmp(cmd, "continue") == 0){
      break;
    } else if(strcmp(cmd, "quit") == 0){
      printf("Really? (y/n) > ");
      scanf("%40s", cmd);
      if(strcmp(cmd, "y") == 0)
        exit_with_dinfo(state, 1);
    } else {
      error("No such command: %s. (`help` will help you!)", cmd);
    }
  }

  return CONTINUOUS;
}
