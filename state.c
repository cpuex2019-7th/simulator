#include <limits.h>
#include <string.h>
#include "state.h"
#include "logging.h"
#include "exec.h"


void write_reg(state_t *state, int dest, int value){
  if (dest == 0){
    debug("Someone tried to write a value to zero register(x0). It is prohibited.");
    exit_if_strict_mode(1);
  } else {
    state->reg[dest] = value;
  }
}


void init_state(state_t *state, int argc, char* argv[]){
  state->pfp = NULL;
  state->pc = 0;
  state->is_running = 1;
  
  for(int i=0; i<32; i++){    
    state->reg[i] = 0;
    state->freg[i] = 0;
  }

  // last return address 
  state->reg[1] = INITIAL_X1;
  
  for(int i=0; i<MEM_SIZE; i++)
    state->mem[i] = 0;

  state->blist = NULL;

  state->ifp = NULL;
  state->ofp = NULL;
  int is_stack_base_specified = 0;
  // process the arguments
  for (int i=1; i < argc; i++){
    if (strcmp(argv[i], "-i")==0){
      // nop
      // for future use
    } else if (strcmp(argv[i], "-o")==0){
      // nop
      // for future use
    } else if (strcmp(argv[i], "--debug")==0){
      set_logging_level(DEBUG);
    } else if (strcmp(argv[i], "--info")==0){
      set_logging_level(INFO);
    } else if (strcmp(argv[i], "--strict")==0){
      set_execution_mode(1); // set the execution strict
    } else if (strcmp(argv[i], "--breakpoint") == 0
               || strcmp(argv[i], "-b") == 0){
      if (i == argc-1){
        error("No address for a new breakpoint is specified.");
        exit(1);
      }
      unsigned baddr;
      sscanf(argv[++i], "%x", &baddr);
      blist_t *new_elm = malloc(sizeof(blist_t));
      new_elm->next = state->blist;
      new_elm->addr = baddr;
      state->blist = new_elm;
    } else if (strcmp(argv[i], "--stack") == 0){
      is_stack_base_specified = 1;
      if (i == argc-1){
        error("No address for sp is specified.");
        exit(1);
      }
      unsigned baddr;
      sscanf(argv[++i], "%x", &baddr);
      state->reg[2] = baddr;
      state->reg[8] = baddr;
    } else {
      // should be .bin filepath
      if(state->pfp != NULL){
        error("Two or more executables were specified.");
        exit(1);
      }
        
      state->pfp = fopen(argv[i], "rb");
      if(state->pfp == NULL){
        error("Failed to open specified executable.");
        exit(1);
      }
    }
  }

  // validate
  if (state->pfp == NULL){
    error("No executable was specified.");
    exit(1);
  }

  // init again
  if (!is_stack_base_specified){
    // 0x100 has no meaning
    debug("Setting x2(sp) and x8(fp) to %08x...", MEM_SIZE-0x100);
    state->reg[2] = MEM_SIZE - 0x100;
    state->reg[8] = MEM_SIZE - 0x100;
  }

  fseek(state->pfp, 0L, SEEK_END);
  state->length = ftell(state->pfp);
}
