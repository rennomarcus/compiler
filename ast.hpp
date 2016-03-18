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
    void print() { std::cout << "(print)Float " << this->value << std::endl; }
    FloatAST(float v) : value(v) {}
    llvm::Value* codegen(Main_block*);
};

class IntegerAST : public BasicAST {
    int value;

    public:
    void print() { std::cout << "(print)Integer " << this->value << std::endl; }
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
    void print() { std::cout << "(print)Variable" << get_name() << std::endl; }
    std::string get_name() { return this->name; }
    VariableAST(std::string name, int var_type) : name(name), var_type(var_type) {}
    llvm::Value* codegen(Main_block*);

};

class AssignAST : public BasicAST {
    std::string var;
    std::vector<BasicAST*> rhs;

    public:
    void print() { std::cout << "(print)Assigning var: " << this->get_name() << std::endl; }
    AssignAST(std::string var_name, std::vector<BasicAST*> rhs) : var(var_name), rhs(rhs) {}
    std::string get_name() { return this->var; }
    llvm::Value* codegen(Main_block*);
};



class CallVarAST : public BasicAST {
    std::string var;
    public:
    std::string get_var() { return this->var; }

    void print() { std::cout << "(print)Calling var" << this->var<< std::endl; }
    CallVarAST(std::string var_name) : var(var_name) {}
    llvm::Value* codegen(Main_block*);
};

class BinopAST : public BasicAST {
    char op;
    BasicAST* op1;
    BasicAST* op2;

    public:
    void print() { std::cout << "(print) bin op" << std::endl; }
    BinopAST(char op, BasicAST* op1, BasicAST* op2) : op(op),op1(op1),op2(op2) {}
    llvm::Value* codegen(Main_block*);
};


#define COND_EQUAL 0
#define COND_NOT_EQUAL 1
#define COND_LESS 2
#define COND_LESS_EQUAL 3
#define COND_GREATER 4
#define COND_GREATER_EQUAL 5

class CondAST : public BasicAST {
    llvm::Function* func; //functio this condition belongs to

    llvm::BasicBlock* cond_true; //block when the cond is true
    llvm::BasicBlock* cond_false; //block when the cond is false

    std::vector<BasicAST*> true_statements; //statements of the true cond block
    std::vector<BasicAST*> false_statements; //statements of the false cond block

    std::vector<BasicAST*> lhs; //condition lhs
    std::vector<int> operand; //operand of condition
    std::vector<BasicAST*> rhs; //condition rhs



    public:
    std::vector<BasicAST*> get_true_statement() { return this->true_statements; }
    std::vector<BasicAST*> get_false_statement() { return this->false_statements; }

    void print() { std::cout << "(print) if block" << std::endl; }
    void add_condition(BasicAST* lhs, int operand, BasicAST* rhs);
    void add_true_statement(BasicAST* stat) { this->true_statements.push_back(stat); }
    void add_false_statement(BasicAST* stat) { this->false_statements.push_back(stat); }

    CondAST() {};
    llvm::Value* codegen(Main_block*);
};

class EndFunctionAST : public BasicAST {
    public:
    EndFunctionAST() { std::cout<< "(print) end function" << std::endl; }
    void print() { }
    llvm::Value* codegen(Main_block *);
};

class FunctionAST : public BasicAST {
    llvm::FunctionType* llvm_func_type;
    llvm::Function* func;

    std::string name; //head
    int func_type; //head
    std::map<std::string, llvm::Type*> args; //head

    std::vector<BasicAST*> list_statement; //body
    std::map<std::string, llvm::Value*> local_var; //body

    llvm::BasicBlock* bb;
    public:
    void print() {  std::cout<< "(print) function " << get_name() << std::endl; }
    FunctionAST(std::string name) : name(name) {}

    void add_arg(std::string, llvm::Type*); //add argument to this function
    void add_var(VariableAST* var); //add var to local variables of this function
    void add_statement(BasicAST*);

    std::string get_name() { return this->name; }
    std::vector<llvm::Type *> get_args_type();
    int change_var_value(std::string, llvm::Value*);
    llvm::Value* get_var(std::string var) { return this->local_var[var]; }
    llvm::Function* get_func() { return this->func; }

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
    void pop_block() { block.pop_back(); }
    FunctionAST* get_func() { return this->functions.back(); }
};
///TODO
/*
Add function semi done to Main_block in the cosntructor
Always pass the Main_block to the codegen of the other classes
Classes can get the current BasicBlock, module, etc...

*/

#endif // __ast_h
