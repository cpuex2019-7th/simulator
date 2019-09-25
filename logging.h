#pragma once
#include <stdio.h>

enum logging_level_t {
      DEBUG,
      INFO,
      ERROR
};

void set_logging_level(enum logging_level_t);

void error(const char *fmt, ...);
void info(const char *fmt, ...);
void debug(const char *fmt, ...);
