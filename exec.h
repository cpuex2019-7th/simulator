#pragma once
#include <stdint.h>
#include "state.h"

typedef enum {
              STEP,
              CONTINUOUS
} execution_mode_t;

int exec_hook_pre(state_t *state);
void exec_hook_post(state_t *state);
void exec_stepi(state_t *state);
void set_execution_mode(execution_mode_t _mode);
void exit_with_dinfo(state_t *state, int code);
int sra(int x, int n);
int srl(int x, int n);
