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
    virtual void print()=0;
    virtual llvm::Value* codegen(Main_block*)=0;
};

class FloatAST : public BasicAST {
    float value;

    public:
    void print() {}
    FloatAST(float v) : value(v) {}
    llvm::Value* codegen(Main_block*);
};

class IntegerAST : public BasicAST {
    int value;

    public:
    void print() { std::cout << this->value << std::endl; }
    IntegerAST(int v) : value(v) {}
    llvm::Value* codegen(Main_block* mblock);
};

class CallExprAST : public BasicAST {
    std::string callee;
    std::vector<llvm:: Value*> args;

    public:
    void print() {}
    llvm::Value* codegen(Main_block*);
};

class VariableAST : public BasicAST {
    int var_type;
    std::string name;
    public:
    void print() {}
    std::string get_name() { return this->name; }
    VariableAST(std::string name, int var_type) : name(name), var_type(var_type) {}
    llvm::Value* codegen(Main_block*);

};

class AssignAST : public BasicAST {
    std::string var;
    BasicAST* value;

    public:
    void print() {}
    AssignAST(std::string var_name, BasicAST* value) : var(var_name), value(value) {}
    std::string get_name() { return this->var; }
    llvm::Value* codegen(Main_block*);
};

class CallVarAST : public BasicAST {
    std::string name;
    public:
    void print() { std::cout << this->name << std::endl; }
    CallVarAST(std::string name) : name(name) {}
    llvm::Value* codegen(Main_block*);
};

class BinopAST : public BasicAST {
    char op;
    BasicAST* op1;
    BasicAST* op2;

    public:
    void print() { std::cout << "bin op" << std::endl; }
    BinopAST(char op, BasicAST* op1, BasicAST* op2) : op(op),op1(op1),op2(op2) {}
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
    void print() {}
    FunctionAST(std::string name) : name(name) {}
    void add_arg(std::string, llvm::Type*); //add argument to this function
    void add_var(VariableAST* var); //add var to local variables of this function
    void add_statement(BasicAST*);
    void show_name() { std::cout << this->name << std::endl; }
    std::vector<llvm::Type *> get_args_type();
    int change_var_value(std::string, llvm::Value*);
    llvm::Value* get_var(std::string var) { return this->local_var[var]; }
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
    void add_statement(BasicAST*);
    llvm::BasicBlock* get_block() { return this->block.back(); }
    void drop_block() { block.pop_back(); }
    FunctionAST* get_func() { return this->functions.back(); }
};
///TODO
/*
Add function semi done to Main_block in the cosntructor
Always pass the Main_block to the codegen of the other classes
Classes can get the current BasicBlock, module, etc...

*/

#endif // __ast_h
