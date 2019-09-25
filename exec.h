#pragma once
#include "state.h"

void exec_hook_pre(state_t *state);
int exec_hook_post(state_t *state);
void exec_stepi(state_t *state);
