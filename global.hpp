#ifndef __GLOBAL_H
#define __GLOBAL_H

//#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

extern int g_line;
extern int state;
//extern std::unique_ptr<llvm::Module> TheModule;
extern llvm::IRBuilder<> Builder;

/// Error* - These are little helper functions for error handling.
void Error(const char *Str);

#endif __GLOBAL_H

