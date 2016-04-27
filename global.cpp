#include "global.hpp"

// Error* - These are little helper functions for error handling.
void Error(const char *str) {
  fprintf(stderr, "***Error in line %d: %s\n", g_line, str);
  exit(-1);
}
//Debug to help
void Debug(const char *value) {
  if (debug_on)
    fprintf(stdout, "[Debug] %s\n", value);
}
void Debug(const char *name, const char* value) {
  if (debug_on)
    fprintf(stdout, "[Debug] %s: %s\n", name, value);
}
int g_structure = 0;
