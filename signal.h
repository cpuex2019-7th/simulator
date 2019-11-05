#pragma once
#include <signal.h>

volatile sig_atomic_t is_show_stat_required;
int is_stat_required();
void set_stat_on_signal();
