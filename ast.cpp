#include "ast.hpp"

using namespace llvm;

 /*Main_block::Main_block(std::string name) {
    this->name = name;
   std::vector<Type *> args;
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
    Function *main_function = Function::Create(ftype, GlobalValue::InternalLinkage, name, this->mod.get() );
    this->main = main_function;

}
int Main_block::add_var(std::string name,std::string var_type) {
    if (this->local_var[name]) {
        std::string error_message = "Variable \'" + name + "\' already assigned";
        Error(error_message.c_str());
        return -1;
    }

}
*/

void Main_block::add_var(VariableAST* var) {
    FunctionAST *func = this->functions.back();
    func->add_var(var);
}
void Main_block::add_arg(std::string name, llvm::Type* var_type) {
    FunctionAST *func = this->functions.back();
    func->add_arg(name, var_type);
}

void Main_block::codegen() {
    this->mod = llvm::make_unique<Module>(this->name, getGlobalContext());
    std::vector<Type *> args;
    //args.push_back(typeOf("void"));
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
    Function *main_function = Function::Create(ftype, GlobalValue::InternalLinkage, this->name, this->mod.get() );
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", main_function);
    Builder.SetInsertPoint(bb);

    FunctionType *ftype2 = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args, false);
    Function *other = Function::Create(ftype2, GlobalValue::InternalLinkage, "other",  this->get_module());
    BasicBlock *bb2 = BasicBlock::Create(getGlobalContext(), "entry", other);
    Builder.SetInsertPoint(bb2);
    std::vector<Value *> args_call;
    Value* x =  ConstantInt::get(Type::getInt32Ty(getGlobalContext()),3);
    Value* y =  ConstantInt::get(Type::getInt32Ty(getGlobalContext()),2);
    Value* tmp = Builder.CreateBinOp(Instruction::Add, x, y, "tmp");
    Builder.CreateRet(tmp);


    Builder.SetInsertPoint(bb);
    std::vector<Value *> args_call2;
    Value* returned = Builder.CreateCall(other,args_call2,"call_other");
    returned = Builder.CreateBinOp(Instruction::Add, returned, x, "returned");


    FunctionAST *test = new FunctionAST("test");
    VariableAST *var = new VariableAST("x", T_INTEGER);
    test->add_statement(var);
}
void FunctionAST::add_arg(std::string var, llvm::Type* var_type) {
    if (this->args[var]) {
        Error("Argument invalid. Already assigned");
    }
    this->args[var] = var_type;
}
void FunctionAST::add_statement(StatementAST* stat) {
    this->list_statement.push_back(stat);
}

std::vector<Type *> FunctionAST::get_args_type() {
    std::vector<Type *> types;
    std::map<std::string, llvm::Type*>::iterator it;
    for(it = this->args.begin(); it!= this->args.end(); it++)
    {
        std::pair<std::string, llvm::Type*> p = *it;
        types.push_back(p.second);
    }
    return types;
}

void FunctionAST::codegen(llvm::Module* mod) {
    /*FunctionType *ftype2 = FunctionType::get(Type::getInt32Ty(getGlobalContext()), args_type, false);
    Function *func = Function::Create(ftype2, GlobalValue::InternalLinkage, this->name,  mod);
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", func);
    this->bb = bb;*/
}



Value* VariableAST::codegen(llvm::BasicBlock* bb) {
    switch(this->var_type) {
        case T_INTEGER:
            AllocaInst *alloc = new AllocaInst(Type::getInt32Ty(getGlobalContext()) , this->name.c_str(), bb);

    }

}



