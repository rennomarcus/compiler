#ifndef __ast_h
#define __ast_h

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <llvm/ExecutionEngine/GenericValue.h>
#include <iostream>
#include <string>
#include <vector>
#include "global.hpp"
#include "scan.hpp"

class Main_block;

class BasicAST {
    public:
    virtual llvm::Value* codegen(Main_block*)=0;
};

class FloatAST : public BasicAST {
    float value;

    public:
    llvm::Value* codegen(Main_block*);
};

class IntegerAST : public BasicAST {
    int value;

    public:
    llvm::Value* codegen();
};

class CallExprAST : public BasicAST {
    std::string callee;
    std::vector<llvm:: Value*> args;

    public:
    llvm::Value* codegen();
};

class VariableAST : public BasicAST {
    int var_type;
    std::string name;
    public:

    void get_name() { std::cout << this->name; }
    VariableAST(std::string name, int var_type) : name(name), var_type(var_type) {}
    llvm::Value* codegen(Main_block*);

};


class FunctionAST : public BasicAST {
    llvm::FunctionType *func;

    std::string name; //head
    int func_type; //head
    std::map<std::string, llvm::Type*> args; //head

    std::vector<BasicAST*> list_statement; //body
    std::map<std::string, llvm::Value*> local_var; //body

    llvm::BasicBlock* bb;
    public:
    FunctionAST(std::string name) : name(name) {}
    void add_arg(std::string, llvm::Type*); //add argument to this function
    void add_var(VariableAST* var) { list_statement.push_back(var); } //add var to local variables of this function
    void add_statement(BasicAST*);
    void show_name() { std::cout << this->name << std::endl; }
    std::vector<llvm::Type *> get_args_type();
    llvm::Value* codegen(Main_block *);
    void show_dump() { func->dump(); }
};


class Main_block {
    std::string name;
    std::unique_ptr<llvm::Module> mod;
    std::vector<llvm::BasicBlock*> block;
    std::map<std::string, llvm::Value*> local_var;

    llvm::Function *main;
    std::vector<FunctionAST*> functions;
    public:


    void set_name(std::string name) { this->name = name; }
    llvm::Module* get_module() { return mod.get(); }
    void codegen();
    void show_dump() { mod->dump(); }
    void add_var(VariableAST* var); //add variable to last function inserted
    void add_func(FunctionAST* func) { functions.push_back(func); }; //add new function to main block
    void add_arg(std::string, llvm::Type*);
    void add_block(llvm::BasicBlock *bb) { block.push_back(bb); }
    llvm::BasicBlock* get_block() { return block.back(); }
};
///TODO
/*
Add function semi done to Main_block in the cosntructor
Always pass the Main_block to the codegen of the other classes
Classes can get the current BasicBlock, module, etc...

*/

#endif // __ast_h
