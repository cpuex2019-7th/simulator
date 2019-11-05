#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "logging.h"

volatile sig_atomic_t is_show_stat_required = 0;

void handler(){
  is_show_stat_required = 1;
}

void set_stat_on_signal(){
  struct sigaction sa_sigusr1;
  memset(&sa_sigusr1, 0, sizeof(sa_sigusr1));
  sa_sigusr1.sa_sigaction = handler;
  sa_sigusr1.sa_flags = SA_SIGINFO;
  if (sigaction(SIGUSR1, &sa_sigusr1, NULL) < 0){
     error("Failed to set USR1 handler.");
     exit(1);
  }
}
