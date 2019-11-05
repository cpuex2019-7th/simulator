#include "breakpoint.h"
#include "state.h"
#include "logging.h"

// controller of breakpoint list
//////////////

int is_here_breakpoint(blist_t *blist, uint32_t addr){
  blist_t *head = blist;
  while(head != NULL){
    if(head->addr == addr)
     return 1;
    head = head->next;
  }
  return 0;
}

void insert_new_breakpoint(blist_t **blist, uint32_t addr){
  // prepare an element
  blist_t *new_elm = malloc(sizeof(blist_t));        
  new_elm->addr = addr;
  new_elm->next = NULL;
  
  if(1 == is_here_breakpoint(*blist,  addr)){    
    error("Breakpoint for 0x%08x is already set.", addr);
    return;
  }

  blist_t **prev =  NULL;
  blist_t **seek = blist;
  while (*seek != NULL){
    if(addr < (*seek)->addr){
      break;
    } else {
      prev = seek;
      seek = &((*seek)->next);
    }
  }
  
  new_elm->next = *seek;
  if(prev == NULL){
    *seek = new_elm;
  } else {   
    (*prev)->next = new_elm;
  }
}

void delete_breakpoint_by_id(blist_t **blist, int id){
  blist_t **prev = NULL;    
  blist_t **seek = blist;

  // pre-validate
  if (*blist == NULL){
    error("There's no breakpoint. Stop.");
    return;
  }

  // search
  for(int i=0; i < id; i++){
    if((*seek)->next == NULL){
      error("No such breakpoint: %d. Stop.", id);
      return;
    } else {
      prev = seek;    
      seek = &((*seek)->next);
    }
  }

  // delete
  if(prev != NULL){
    (*prev)->next = (*seek)->next;
  }
  free(*seek);
}
