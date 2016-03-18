#include "global.hpp"

/// Error* - These are little helper functions for error handling.
void Error(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  exit(-1);
}
