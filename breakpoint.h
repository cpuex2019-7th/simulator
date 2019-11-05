#pragma once
#include <stdlib.h>

typedef struct _breakpoint_list {
  uint32_t addr;  
  struct _breakpoint_list *next;
} blist_t;

int is_here_breakpoint(blist_t *blist, uint32_t addr);
void insert_new_breakpoint(blist_t **blist, uint32_t addr);
void delete_breakpoint_by_id(blist_t **blist, int id);
