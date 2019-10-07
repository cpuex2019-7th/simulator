#pragma once
#include <stdint.h>
#include "state.h"

int exec_hook_pre(state_t *state);
void exec_hook_post(state_t *state);
void exec_stepi(state_t *state);
void set_execution_mode(int _mode);
void exit_if_strict_mode(int code);
int sra(int x, int n);
int srl(int x, int n);
