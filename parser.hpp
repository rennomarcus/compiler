#ifndef __parser
#define __parser

#include "ast.hpp"
#include "scan.hpp"
#include "global.hpp"
#include <stack>
#include <llvm/ExecutionEngine/GenericValue.h>

void HandleProgram(Scan_file *, Main_block *);
void HandleFunction(Scan_file *, Main_block *);
void HandleTopLevelExpression(Scan_file*, Main_block*);
llvm::Value* HandleExpression(Scan_file*, Main_block*);
VariableAST* HandleVariableDeclaration(Scan_file *, Main_block *);
llvm::Value* parser(Scan_file *, Main_block *);

class NBlock;

using namespace llvm;

class CodeGenBlock {
public:
    BasicBlock *block;
    std::map<std::string, Value*> locals;
};

class MainContext {
    std::stack<CodeGenBlock *> blocks;
    Function *mainFunction;
    std::string name;
public:
    Module *module;
    MainContext(std::string name) : name(name) { module = new Module(name, getGlobalContext()); }

    void generateCode(NBlock& root);
    GenericValue runCode();
    std::map<std::string, Value*>& locals() { return blocks.top()->locals; }
    BasicBlock *currentBlock() { return blocks.top()->block; }
    void pushBlock(BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->block = block; }
    void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
};

#endif // __parser
