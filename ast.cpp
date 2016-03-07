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
void Main_block::add_statement(BasicAST* stat) {
    FunctionAST *func = this->functions.back();
    func->add_statement(stat);
}
void Main_block::add_arg(std::string name, llvm::Type* var_type) {
    FunctionAST *func = this->functions.back();
    func->add_arg(name, var_type);
}

void Main_block::codegen() {
    this->mod = llvm::make_unique<Module>(this->name, getGlobalContext());
    std::vector<Type *> args;
    //args.push_back(typeOf("void"));

    /*FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), args, false);
    Function *main_function = Function::Create(ftype, GlobalValue::InternalLinkage, this->name, this->mod.get() );
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", main_function);
    Builder.SetInsertPoint(bb);*/
    std::vector<FunctionAST*>::iterator it;
    for (it = (this->functions).begin(); it != (this->functions).end(); it++) {
        (*it)->codegen(this);
        //???? should i put this down here????
        drop_block();
        Builder.SetInsertPoint(get_block());
    }

    /*
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
    //test->add_statement(var);*/
}
void FunctionAST::add_arg(std::string var, llvm::Type* var_type) {
    if (this->args[var]) {
        Error("Argument invalid. Already assigned");
    }
    this->args[var] = var_type;
}
void FunctionAST::add_var(VariableAST* var) {
    if (this->local_var[var->get_name()]) {
        Error("Variable already created!");
    }
    else { //remove this else, if Error stops program.
        this->local_var[var->get_name()] = nullptr;
        list_statement.push_back(var);
    }
}
void FunctionAST::add_statement(BasicAST* stat) {
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

llvm::Value* FunctionAST::codegen(Main_block *mblock) {
    std::vector<Type *> args_type = get_args_type();
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), args_type, false);
    Function *func = Function::Create(ftype, GlobalValue::InternalLinkage, this->name,  mblock->get_module());
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", func);
    std::cout << "Function created: " << this->name << std::endl;
    auto my_arg = this->args.begin();
    for (auto args = func->arg_begin(); args != func->arg_end(); ++args) {
        Value *x = args;
        x->setName(my_arg->first);

        my_arg++;
    }

    this->bb = bb;
    mblock->add_block(bb);
    Builder.SetInsertPoint(bb);
    for (auto it = list_statement.begin(); it != list_statement.end(); it++) {
        (*it)->codegen(mblock);
    }

    return nullptr;

}

int FunctionAST::change_var_value(std::string var, llvm::Value* value) {
    std::cout << "CHANGING " << var << std::endl;
    this->local_var[var] = value;
    return 1;
}

Value* VariableAST::codegen(Main_block* mblock) {
    AllocaInst *alloc;
    FunctionAST *func = mblock->get_func();
    std::cout << "Creating var: " << this->name << std::endl;
    switch(this->var_type) {
        case T_INTEGER:
            alloc = new AllocaInst(Type::getInt32Ty(getGlobalContext()) , this->name.c_str(), mblock->get_block());
            break;
        case T_FLOAT:
            alloc = new AllocaInst(Type::getFloatTy(getGlobalContext()) , this->name.c_str(), mblock->get_block());
            break;
    }

    func->change_var_value(this->name, alloc);
    return alloc;
}

Value* AssignAST::codegen(Main_block* mblock) {
    StoreInst* test;
    FunctionAST *func = mblock->get_func();

    llvm::Value* code = (this->value)->codegen(mblock);
    std::cout << "*******assigning" << this->get_name() << std::endl;
    if (func->change_var_value(get_name(),code)) {
        std::cout << "******" << std::endl;
        llvm::Value* var = func->get_var(this->get_name());
        return new StoreInst(code, var, false, mblock->get_block());
    }
    return nullptr;
}

Value* CallExprAST::codegen(Main_block* mblock) {
    //CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    return nullptr;
}

Value* IntegerAST::codegen(Main_block* mblock) {
    std::cout << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), value, true);
}
Value* FloatAST::codegen(Main_block* mblock) {
    return nullptr;
}

Value* BinopAST::codegen(Main_block* mblock) {
    return nullptr;
}

Value* CallVarAST::codegen(Main_block* mblock) {
    return nullptr;
}
