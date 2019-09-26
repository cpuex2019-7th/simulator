#include "instr.h"

typedef enum {
              INSTR_R,
              INSTR_I,
              INSTR_S,
              INSTR_B,
              INSTR_U,
              INSTR_J              
} instr_type_t;

typedef struct {
  int instr_meta_t;
  char *label;
} instr_meta_t;

void disasm(instr_t *instr, char *dest, size_t s);
