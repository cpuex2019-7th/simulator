#include <limits.h>
#include <string.h>
#include "state.h"
#include "logging.h"
#include "exec.h"

int get_uart_status(state_t *state){
  char c =  state->ifp == NULL? EOF : fgetc(state->ifp);
  if (c != EOF)
    fseek(state->ifp, -1, SEEK_CUR);
  return c == EOF? 0 : 1;
}

void write_reg(state_t *state, int dest, int value){
  if (dest == 0){
    debug("Someone tried to write a value to zero register(x0). It will be ignored.");
  } else {
    state->reg[dest] = value;
    state->reg_min[dest] = state->reg_min[dest] <= value? state->reg_min[dest] : value;
    state->reg_max[dest] = value <= state->reg_max[dest]? state->reg_max[dest] : value;
  }
}

void write_freg(state_t *state, int dest, float value){
  state->freg[dest].f = value;
}

uint8_t read_mem_uint8(state_t *state, int addr){
  if (0 <= addr && addr < MEM_SIZE) {
    return *(uint8_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

uint16_t read_mem_uint16(state_t *state, int addr){
  if (0 <= addr && addr < MEM_SIZE) {
    return *(uint16_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

uint32_t read_mem_uint32(state_t *state, int addr){
  if (0 <= addr && addr < MEM_SIZE) {
    return *(uint32_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

float read_mem_float(state_t *state, int addr){
  if (0 <= addr && addr < MEM_SIZE) {
    return *(float *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}


void write_mem(state_t *state, int addr, char value){
  if (0 <= addr && addr < MEM_SIZE) {
    state->mem[addr] = value;
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
  }
}

void init_state(state_t *state, int argc, char* argv[]){
  state->pfp = NULL;
  state->pc = 0;
  state->is_running = 1;
  
  for(int i=0; i<32; i++){    
    state->reg[i] = 0;
    state->freg[i].i = 0;
  }

  // last return address 
  state->reg[1] = INITIAL_X1;

  state->mem = malloc(MEM_SIZE * sizeof(uint8_t));
  for(int i=0; i<MEM_SIZE; i++)
    state->mem[i] = 0;

  state->blist = NULL;
  state->slist = NULL;

  state->step_num = 0;

  state->ifp = NULL;
  state->ofp = NULL;
  
  // process the arguments
  for (int i=1; i < argc; i++){
    if (strcmp(argv[i], "-i")==0){
      // set source of uart input
      if (i == argc-1){
        error("No input file is specified.");
        exit(1);
      }
      state->ifp = fopen(argv[++i], "rb");
      if (state->ifp == NULL){
        error("Cannot open input destination.");
        exit(1);
      }
    } else if (strcmp(argv[i], "-o")==0){
      // set target of uart output
      if (i == argc-1){
        error("No output file is specified.");
        exit(1);
      }
      state->ofp = fopen(argv[++i], "wb");
      if (state->ofp == NULL){
        error("Cannot open output destination.");
        exit(1);
      }
    } else if (strcmp(argv[i], "--statout")==0){
      // set output file of statistics
      if (i == argc-1){
        error("No statout file is specified.");
        exit(1);
      }
      state->sfp = fopen(argv[++i], "w");
      if (state->sfp == NULL){
        error("Cannot open statout destination.");
        exit(1);
      }
    } else if (strcmp(argv[i], "--memsize")==0){
      // set output file of statistics
      if (i == argc-1){
        error("No memory is specified.");
        exit(1);
      }
      free(state->mem);
      state->mem = malloc(sizeof(uint8_t) * atoi(argv[++i]));
      if(state->mem == NULL){
        error("Cannot allocate memory. (size=%d)", atoi(argv[i]));
        exit(1);
      }
    } else if (strcmp(argv[i], "--debug")==0){
      // set logging level to debug
      set_logging_level(DEBUG);
    } else if (strcmp(argv[i], "--info")==0){
      // set logging level to info
      set_logging_level(INFO);
    } else if (strcmp(argv[i], "--strict")==0){
      // set execution mode to strict
      set_execution_mode(1); // set the execution strict
    } else if (strcmp(argv[i], "--breakpoint") == 0
               || strcmp(argv[i], "-b") == 0){
      // set breakpoint
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
    } else if (strcmp(argv[i], "--symbol") == 0){
      // loading symbol information
      if (i == argc-1){
        error("No symbol list is specified.");
        exit(1);
      }
      FILE *slistfp = fopen(argv[++i], "r");
      if (slistfp == NULL){
        error("Cannot open the file you given: %s", argv[i]);
        exit(1);
      }
      
      char symbol_buf[80];
      int offset_buf;
      while (fscanf(slistfp, "%s %d", symbol_buf, &offset_buf) == 2 ) {
        slist_t *new_elm = malloc(sizeof(slist_t));
        new_elm->label = malloc((strlen(symbol_buf)+1) * sizeof(char));
        strcpy(new_elm->label, symbol_buf);        
        new_elm->offset = offset_buf;
        new_elm->called_num = 0;
        new_elm->next = state->slist;
        state->slist = new_elm;        
      }      
    } else {
      // should be .bin filepath
      if(state->pfp != NULL){
        error("Two or more executables were specified.");
        exit(1);
      }
      state->filename = malloc(strlen(argv[i])+1);
      strcpy(state->filename, argv[i]);
      
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

  // set program length
  fseek(state->pfp, 0L, SEEK_END);
  state->length = ftell(state->pfp);
}
