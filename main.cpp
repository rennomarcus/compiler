#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "ast.hpp"
#include "parser.hpp"
#include "scan.hpp"
#include "global.hpp"

using namespace llvm;


llvm::IRBuilder<> Builder(llvm::getGlobalContext());
int state = 0;

int main () {

    //lexer
    std::string file_name = "test.src";
    Scan_file *scan = new Scan_file(file_name);
    Main_block *module = new Main_block();

    int i = 0;


    while (scan->scan_tok() != T_EOF) {
        //parser
        parser(scan,module);



    }
    //module->codegen();
    //module->show_dump();
    return 0;
}