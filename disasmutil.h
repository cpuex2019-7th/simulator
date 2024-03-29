#include <stdint.h>
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
  instr_type_t type;
  char *label;
} instr_meta_t;

void disasm(instr_t *instr, uint32_t, char *dest, size_t s);
char *r2t(int reg);
char *r2tf(int reg);
