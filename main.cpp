#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>


#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FormattedStream.h>

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
std::map<char, int> BinopPrecedence;
int g_line = 1;

int main () {
    BinopPrecedence['+'] = 1;
    BinopPrecedence['-'] = 1;
    BinopPrecedence['*'] = 2;
    BinopPrecedence['/'] = 2;
    BinopPrecedence['('] = 3;
    BinopPrecedence[')'] = 3;
    //lexer
    std::string file_name = "test.src";
    Scan_file *scan = new Scan_file(file_name);
    Main_block *module = new Main_block();

    int i = 0;

    while (scan->scan_tok() != T_EOF) {
        //parser
        parser(scan,module);
    }

    //codegen
    std::cout << std::endl << std::endl << std::endl;
    module->codegen();
    //module->show_dump();

    PassManager PM;
    PM.add(createPrintModulePass(outs()));
    Module* mod = module->get_module();
    PM.run(*mod);

    return 0;
}
