#include <stdio.h>
#include <stdarg.h>
#include "logging.h"

static enum logging_level_t log_level;

void set_logging_level(enum logging_level_t _log_level){
  log_level = _log_level;
}

void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (log_level <= ERROR){
    printf("\033[0;31m[-] ERROR: ");
    vprintf(fmt, ap);
    printf("\033[0m\n");
  }
  va_end(ap);
}

void debug(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (log_level <= DEBUG){
    printf("[x] ");
    vprintf(fmt, ap);
    printf("\n");
  }
  va_end(ap);
}

void info(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  if (log_level <= INFO){
    printf("[*] ");
    vprintf(fmt, ap);
    printf("\n");
  }
  va_end(ap);
}
