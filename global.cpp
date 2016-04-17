#include "global.hpp"

/// Error* - These are little helper functions for error handling.
void Error(const char *Str) {
  fprintf(stderr, "Error in line %d: %s\n", g_line, Str);
  exit(-1);
}

int g_structure = 0;
