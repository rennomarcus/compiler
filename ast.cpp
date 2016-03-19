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
void Main_block::add_special(FlowBlock* special, int type) {
    FunctionAST *func = this->functions.back();
    func->add_special(special, type);
}
void Main_block::pop_special() {
    FunctionAST *func = this->functions.back();
    func->pop_special();
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
    if (get_state() == 0)
        this->list_statement.push_back(stat);
    else {
        this->sblocks->add_statement(stat);
    }
}
void FunctionAST::add_special(FlowBlock* special, int type) {
    std::cout << "Creating one block" << std::endl;
    sblocks->add_block(special, type);
    inc_state();
}
void FunctionAST::pop_special() {
    std::cout << "Leaving one block" << std::endl;
    dec_state();
    FlowBlock* sb = sblocks->pop_block();
    add_statement(sb);
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
void SpecialBlock::add_block(FlowBlock* special, int type) {
    FlowBlock* sb;
    if (type == 1) {
        sb = special;
        sb->set_type(1);
    }
    this->blocks.push_back(sb);
}
FlowBlock* SpecialBlock::get_block() {
    return this->blocks.back();
}
 FlowBlock* SpecialBlock::pop_block() {
    FlowBlock* sb = this->blocks.back();
    this->blocks.pop_back();
    return sb;
}
void SpecialBlock::add_statement(BasicAST* stat) {
    FlowBlock* sb = get_block();
    sb->add_statement(stat);
}

void CondAST::add_statement(BasicAST* stat) {
    int state = get_state();
    std::cout << "Adding something in condition!" << std::endl;
    stat->print();
    if (!state) {
        this->true_statements.push_back(stat);
    } else {
        this->false_statements.push_back(stat);
    }
}

llvm::Value* FunctionAST::codegen(Main_block *mblock) {
    std::vector<Type *> args_type = get_args_type();
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), args_type, false);
    Function *func = Function::Create(ftype, GlobalValue::InternalLinkage, this->name,  mblock->get_module());
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", func);
    this->func = func;
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
    std::cout << "Assigning var: " << get_name() << std::endl;
    FunctionAST *func = mblock->get_func();


    llvm::Value* lhs = func->get_var(get_name());
    std::vector<BasicAST*> rhs = this->rhs;
    llvm::Value* temp;
    for (auto it = rhs.begin(); it != rhs.end(); it++) {
        temp = (*it)->codegen(mblock);
    }

    llvm::Value* code = temp;
    std::cout << "*assigning " << this->get_name() << std::endl;
    if (lhs) {
        return new StoreInst(code, lhs, false, mblock->get_block());
        //lhs = code;
    }

    return nullptr;
}

Value* CallExprAST::codegen(Main_block* mblock) {
    //CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    return nullptr;
}

Value* IntegerAST::codegen(Main_block* mblock) {
    std::cout << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), value);
}
Value* FloatAST::codegen(Main_block* mblock) {
    return nullptr;
}

Value* BinopAST::codegen(Main_block* mblock) {
    std::cout << "Creating Binary operation: " << op << std::endl;
    llvm::Value* lhs = (this->op1)->codegen(mblock);
    llvm::Value* rhs = (this->op2)->codegen(mblock);
    Value* binop;
    if (this->op == '+')
        binop = BinaryOperator::Create(Instruction::Add, lhs, rhs, "add", mblock->get_block());
    else if (this->op == '-')
        binop = BinaryOperator::Create(Instruction::Sub, lhs, rhs, "sub", mblock->get_block());
    else if (this->op == '*')
        binop = BinaryOperator::Create(Instruction::Mul, lhs, rhs, "mul", mblock->get_block());
    else if (this->op == '/')
        binop = BinaryOperator::Create(Instruction::SDiv, lhs, rhs, "div", mblock->get_block());

    return binop;
}

Value* CallVarAST::codegen(Main_block* mblock) {
    std::cout << "Getting var " << get_var() << " value" << std::endl;
    std::string var_name = get_var();
    FunctionAST *func = mblock->get_func();
    return func->get_var(var_name);
}
Value* EndFunctionAST::codegen(Main_block* mblock) {
    std::cout << "changing function" << std::endl;
    FunctionAST *func = mblock->get_func();
    mblock->pop_block();
    Builder.SetInsertPoint(mblock->get_block());
    return nullptr;
}

Value* CondAST::codegen(Main_block* mblock) {
    std::cout << "Creating conditional block" << std::endl;
    llvm::Function* func = mblock->get_func()->get_func();
    llvm::BasicBlock* current_block = mblock->get_block();
    llvm::BasicBlock* cond_true = BasicBlock::Create(getGlobalContext(), "cond_true", func);
    llvm::BasicBlock* cond_false = BasicBlock::Create(getGlobalContext(), "cond_false", func);

    Value* lhs = (this->lhs.back())->codegen(mblock);
    Value* rhs = (this->rhs.back())->codegen(mblock);
    Value* comp;

    switch (this->operand.back()) {
        case 0: //equal
            comp = Builder.CreateICmpEQ(lhs, rhs, "tmp_eq");
            break;
        case 1: //not equal
            comp = Builder.CreateICmpNE(lhs, rhs, "tmp_ne");
            break;
        case 2: //signed less than
            comp = Builder.CreateICmpSLT(lhs, rhs, "tmp_slt");
            break;
        case 3: //signed less or equal
            comp = Builder.CreateICmpSLE(lhs, rhs, "tmp_sle");
            break;
        case 4: //signed greater than
            comp = Builder.CreateICmpSGT(lhs, rhs, "tmp_sgt");
            break;
        case 5: //signed greater or equal
            comp = Builder.CreateICmpSGE(lhs, rhs, "tmp_sge");
            break;

    }

    llvm::Value* cond = Builder.CreateCondBr(comp, cond_true, cond_false);

    Builder.SetInsertPoint(cond_true);
    mblock->add_block(cond_true);
    std::vector<BasicAST*> tmp = get_true_statement();
    for (auto it = tmp.begin(); it != tmp.end(); it++) {
        (*it)->codegen(mblock);
    }
    mblock->pop_block();

    Builder.SetInsertPoint(cond_false);
    mblock->add_block(cond_false);
    tmp = get_false_statement();
    for (auto it = tmp.begin(); it != tmp.end(); it++) {
        (*it)->codegen(mblock);
    }
    mblock->pop_block();

    Builder.SetInsertPoint(current_block);

    std::cout << "Leaving conditional block" << std::endl;

    return nullptr;
}

void CondAST::add_condition(BasicAST* lhs, int operand, BasicAST* rhs) {
    this->lhs.push_back(lhs);
    this->rhs.push_back(rhs);
    this->operand.push_back(operand);
}
