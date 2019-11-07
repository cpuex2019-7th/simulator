#include <limits.h>
#include <string.h>
#include "state.h"
#include "logging.h"
#include "exec.h"
#include "symbols.h"

// uart utils
//////////////
int get_uart_status(state_t *state){
  int c =  state->ifp == NULL? EOF : fgetc(state->ifp);
  if (c != EOF)
    fseek(state->ifp, -1, SEEK_CUR);
  return c == EOF? 0 : 1;
}

// controller of registers
//////////////
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

// controller of memory
//////////////
uint8_t read_mem_uint8(state_t *state, int addr){
  if (0 <= addr && addr < state->memsize) {
    return *(uint8_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

uint16_t read_mem_uint16(state_t *state, int addr){
  if (0 <= addr && addr < state->memsize) {
    return *(uint16_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

uint32_t read_mem_uint32(state_t *state, int addr){
  if (0 <= addr && addr < state->memsize) {
    return *(uint32_t *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

float read_mem_float(state_t *state, int addr){
  if (0 <= addr && addr < state->memsize) {
    return *(float *)(state->mem + addr);
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
    return 0;
  }
}

void write_mem(state_t *state, int addr, char value){
  if (0 <= addr && addr < state->memsize) {
    state->mem[addr] = value;
  } else {
    error("SIGSEGV was thrown.");
    state->is_running = 0;
  }
}

// controller of the state of simulator
//////////////
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

  state->memsize = MEM_SIZE;
  state->mem = malloc(MEM_SIZE * sizeof(uint8_t));
  for(int i=0; i<MEM_SIZE; i++)
    state->mem[i] = 0;

  state->blist = NULL;
  state->slist = NULL;

  state->step_num = 0;

  state->ifp = NULL;
  state->ofp = NULL;
  set_execution_mode(CONTINUOUS);
  
  // process the arguments
  for (int i=1; i < argc; i++){
    if (strcmp(argv[i], "--input") == 0
        || strcmp(argv[i], "-i")==0){
      // set source of uart input
      ///////////////
      if (i == argc-1){
        error("No input file is specified.");
        exit(1);
      }
      state->ifp = fopen(argv[++i], "rb");
      if (state->ifp == NULL){
        error("Cannot open input destination.");
        exit(1);
      }
    } else if (strcmp(argv[i], "--output") == 0
               || strcmp(argv[i], "-o")==0){
      // set target of uart output
      ///////////////
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
      ///////////////
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
      ///////////////
      if (i == argc-1){
        error("No memory is specified.");
        exit(1);
      }
      free(state->mem);
      state->memsize = atoi(argv[++i]);
      state->mem = malloc(sizeof(uint8_t) * state->memsize);
      if(state->mem == NULL){
        error("Cannot allocate memory. (size=%d)", state->memsize);
        exit(1);
      }
    } else if (strcmp(argv[i], "--debug")==0){
      // set logging level to debug
      ///////////////
      set_logging_level(DEBUG);
    }  else if (strcmp(argv[i], "--debugger")==0){
      // launch debugger at startup
      ///////////////
      set_execution_mode(STEP);
    } else if (strcmp(argv[i], "--info")==0){
      // set logging level to info
      ///////////////
      set_logging_level(INFO);
    }  else if (strcmp(argv[i], "--symbols") == 0){
      // loading symbol information
      ///////////////
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
      int addr_buf;
      while (fscanf(slistfp, "%s %d", symbol_buf, &addr_buf) == 2 ) {
        insert_new_symbol(&(state->slist), symbol_buf, addr_buf);
      }      
    } else {
      // argument should be .bin filepath
      ///////////////
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
