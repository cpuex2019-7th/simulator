#pragma once
#include <stdint.h>

typedef struct _symbol_list {
  char *label;
  // where is this symbol?
  uint32_t addr;
  // how many times jump instruction for this symbol occured?
  uint32_t called_num; 
  struct _symbol_list *next;
} slist_t;

void update_slist(slist_t *slist, uint32_t addr);
void insert_new_symbol(slist_t **slist, char *label, uint32_t addr);
void get_label_and_offset(slist_t *slist, uint32_t addr, char **label, uint32_t *offset);
int get_addr_from_label(slist_t *slist, char* label);
