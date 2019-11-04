#include <string.h>

#include "symbols.h"
#include "state.h"

// controller of symbol list
//////////////
void update_slist(slist_t *slist, uint32_t addr){
  slist_t *seek = slist;
  while(seek != NULL){
    if(seek->addr == addr){
      seek->called_num += 1;
      return;
    } 
    seek = seek->next;
  }
}

void get_label_and_offset(slist_t *slist, uint32_t addr, char **label, uint32_t *offset){
  *label = NULL;
  *offset = 0;
  
  if (slist != NULL) {
    slist_t *seek = slist;
    while (seek != NULL){
      if(seek->addr <= addr){
        *label = seek->label;
        *offset = addr - seek->addr;
        seek = seek->next;
      } else {
        break;
      }
    }
  }
}

void insert_new_symbol(slist_t **slist, char *label, uint32_t addr){
  // preapre an element
  slist_t *new_elm = malloc(sizeof(slist_t));        
  new_elm->label = malloc((strlen(label)+1) * sizeof(char));
  strcpy(new_elm->label, label);        
  new_elm->addr = addr;
  new_elm->called_num = 0;
  new_elm->next = NULL;

  slist_t **seek = slist;
  while (*seek != NULL){
    if(addr < (*seek)->addr){
      break;
    } else {
      seek = &((*seek)->next);
    }
  }
  new_elm->next = *seek;
  (*seek) = new_elm;
}
