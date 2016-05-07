#ifndef __ast_h
#define __ast_h

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <iostream>
#include <string>
#include <vector>
#include "global.hpp"
#include "scan.hpp"

class Main_block;
class FunctionAST;

class BasicAST {
    public:
    virtual void print()=0;
    virtual llvm::Value* codegen(Main_block*)=0;
};

class FloatAST : public BasicAST {
    float value;

    public:
    void print() { Debug("(print)Float ", std::to_string(this->value).c_str()); }
    FloatAST(float v) : value(v) {}
    llvm::Value* codegen(Main_block*);
};

class IntegerAST : public BasicAST {
    int value;

    public:
    void print() { Debug("(print) Integer ", std::to_string(this->value).c_str()); }
    IntegerAST(int v) : value(v) {}
    llvm::Value* codegen(Main_block* mblock);
};

class CallFuncAST : public BasicAST {
    std::string callee;
    std::string message;
    std::string g_var;
    std::string var;
    std::vector<std::string> args;
    std::vector<FunctionAST*> functions;

    int external;
    public:
    void print() { Debug("(print) CallFunc", this->callee.c_str()); }
    void add_arg(std::string arg) { args.push_back(arg); }
    int get_external() { return external; }
    void set_external() { external = 1; }
    void set_message(int);
    void set_name(std::string name) { this->callee = name; }
    void set_var(std::string var) { this->var = var; }

    std::string get_name() { return callee; }
    std::string get_message() { return message; }
    std::string get_gvar() { return g_var; }
    std::string get_var() { return var; }
    std::vector<std::string> get_args() { return this->args; }
    FunctionAST* get_func(Main_block*);
    CallFuncAST() { external = 0; }
    CallFuncAST(std::string name) : callee(name) { external = 0; }
    llvm::Value* codegen(Main_block*);
};

class VariableAST : public BasicAST {
    int var_type;
    bool is_global;
    std::string name;
    int array_size;
    public:

    bool get_global() { return is_global; }
    int get_array() { return array_size; }
    void set_global() { is_global = true; }
    void set_array(int n) { array_size = n; }

    void print() { Debug("(print) Variable", get_name().c_str()); }
    std::string get_name() { return this->name; }
    VariableAST(std::string name, int var_type) : name(name), var_type(var_type), array_size(0) {}
    llvm::Value* codegen(Main_block*);

};

class AssignAST : public BasicAST {
    std::string var;
    std::vector<BasicAST*> rhs;
    bool isarray;
    int array_pos;
    public:
    bool get_array() {  return isarray; }
    int get_array_pos() { return array_pos; }
    void set_array(bool val) { isarray = val; }
    void print() { Debug("(print)Assigning var", this->get_name().c_str()); }
    AssignAST(std::string var_name, std::vector<BasicAST*> rhs) : var(var_name), rhs(rhs) { set_array(false); }
    AssignAST(std::string var_name, int pos, std::vector<BasicAST*> rhs) : var(var_name), rhs(rhs), array_pos(pos) { set_array(true); }
    std::string get_name() { return this->var; }
    llvm::Value* codegen(Main_block*);
};



class CallVarAST : public BasicAST {
    std::string var;
    public:
    std::string get_var() { return this->var; }

    void print() { Debug("(print)Calling var", this->var.c_str()); }
    CallVarAST(std::string var_name) : var(var_name) {}
    llvm::Value* codegen(Main_block*);
};

class BinopAST : public BasicAST {
    char op;
    BasicAST* op1;
    BasicAST* op2;

    public:
    void print() { Debug("(print) bin op"); }
    BinopAST(char op, BasicAST* op1, BasicAST* op2) : op(op),op1(op1),op2(op2) {}
    llvm::Value* codegen(Main_block*);
};


#define COND_EQUAL 0
#define COND_NOT_EQUAL 1
#define COND_LESS 2
#define COND_LESS_EQUAL 3
#define COND_GREATER 4
#define COND_GREATER_EQUAL 5

class FlowBlock : public BasicAST {
    int type;
    public:
    void set_type(int t) { this->type = t; }
    int get_type() { return this->type; }
    virtual void add_statement(BasicAST*)=0;

    virtual void change_state()=0; //used in the control flow to go to the else state
};

class CondAST : public FlowBlock {
    llvm::Function* func; //functio this condition belongs to
    int state; //shows if cond is true or in false state

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
    void set_state(int s) { this->state = s; }

    void print() { Debug("(print) if block"); }
    void add_condition(BasicAST* lhs, int operand, BasicAST* rhs);
    void add_statement(BasicAST*);
    void change_state() { state++; }
    int get_state() { return this->state; }

    CondAST() { set_state(0); };
    llvm::Value* codegen(Main_block*);
};

class EndFunctionAST : public BasicAST {
    public:
    EndFunctionAST() { Debug("(print) end function"); }
    void print() { }
    llvm::Value* codegen(Main_block *);
};

class SpecialBlock {
    std::vector<FlowBlock*> blocks;

    FlowBlock* get_block();
    public:
    void add_statement(BasicAST*);
    void add_block(FlowBlock*, int);
    void change_state();
    FlowBlock* pop_block();
};
class FunctionAST : public BasicAST {
    llvm::FunctionType* llvm_func_type;
    llvm::Function* func;
    int state; //show if there is an if or a loop (1 == if, 2 == loop)

    std::string name; //head
    int func_type; //head
    std::map<std::string, llvm::Type*> args; //head
    std::vector<int> arg_mask;

    std::vector<BasicAST*> list_statement; //body
    SpecialBlock* sblocks; //body
    std::map<std::string, llvm::Value*> local_var; //body

    llvm::BasicBlock* bb;
    public:
    void print() {  Debug("(print) function", get_name().c_str()); }
    FunctionAST(std::string name) : name(name) { this->state = 0; this->sblocks = new SpecialBlock; }

    void add_arg(std::string, llvm::Type*); //add argument to this function
    void add_var(VariableAST* var); //add var to local variables of this function
    void add_statement(BasicAST*);
    void add_special(FlowBlock*, int);
    void add_return(Main_block*);
    void add_mask(int m) { this->arg_mask.push_back(m); }
    void pop_special();

    void inc_state() { this->state = this->state + 1; }
    void dec_state() { this->state = this->state - 1; }

    int change_var_value(std::string, llvm::Value*);
    void change_state() { sblocks->change_state(); };

    void set_func(llvm::Function* f) { this->func = f; }
    std::string get_name() { return this->name; }
    std::vector<llvm::Type *> get_args_type();
    llvm::Value* get_var(std::string var) { return this->local_var[var]; }
    llvm::Function* get_func() { return this->func; }
    int get_state() { return this->state; }
    std::vector<int> get_mask() { return this->arg_mask; }
    llvm::Value* codegen(Main_block *);
    void show_dump() { func->dump(); }
};

struct structure {
    std::string var_name;
    llvm::Type* type;
    int state;
};
class Structure {
    std::vector<struct structure> structures;
    int state;

    public:
    Structure() { state = 0; }
    void print() { Debug("(print) structures"); }
    void add_structure(std::string, llvm::Type*);
    std::vector<std::string> get_vars(Main_block*,std::string);
    void inc_state() { state++; }
    void codegen(Main_block* );

};

class Main_block {
    std::string name;
    std::unique_ptr<llvm::Module> mod;
    std::vector<llvm::BasicBlock*> block;
    std::map<std::string, llvm::Value*> local_var;
    Structure* s;

    llvm::Function *main;
    std::vector<FunctionAST*> functions;
    int function_position;
    public:
    Main_block();

    void set_name(std::string name) { this->name = name; }

    void codegen();
    void show_dump() { mod->dump(); }
    void add_var(VariableAST* var); //add variable to last function inserted
    void add_gvar(VariableAST* var);
    void add_array();
    void add_func(FunctionAST* func) { functions.push_back(func); }; //add new function to main block
    void add_arg(std::string, llvm::Type*);
    void add_block(llvm::BasicBlock *bb) { block.push_back(bb); }
    void add_statement(BasicAST*); //add a statement to the current block
    void add_special(FlowBlock*, int); //add a special block
    void add_structure(std::string v,llvm::Type* t) { s->add_structure(v,t); }
    void add_mask(int);
    void pop_special(); //pop the last special block in the current function
    void pop_block() { block.pop_back(); }
    void pop_function();
    void inc_structure() { s->inc_state(); }
    void inc_function() { function_position++; }

    void change_state(); //used in the control flow
    llvm::BasicBlock* get_block() { return this->block.back(); }
    FunctionAST* get_func();
    FunctionAST* get_func(int);
    FunctionAST* get_main_func() { return this->functions.front(); }
    std::vector<FunctionAST*> get_functions() { return this->functions; }
    llvm::Module* get_module() { return mod.get(); }
    std::vector<std::string> get_struct_vars(std::string func_name) { return s->get_vars(this,func_name); }
    void reset();
    llvm::GenericValue runCode();
};
///TODO
/*
Add function semi done to Main_block in the cosntructor
Always pass the Main_block to the codegen of the other classes
Classes can get the current BasicBlock, module, etc...

*/

#endif // __ast_h
