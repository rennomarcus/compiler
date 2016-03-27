#ifndef __GLOBAL_H
#define __GLOBAL_H

//#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

extern int g_line;
extern int state;
extern int g_structure;
//extern std::unique_ptr<llvm::Module> TheModule;
extern llvm::IRBuilder<> Builder;
extern std::map<char, int> BinopPrecedence;

/// Error* - These are little helper functions for error handling.
void Error(const char *Str);


#endif __GLOBAL_H

