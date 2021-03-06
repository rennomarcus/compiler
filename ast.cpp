#include "ast.hpp"

using namespace llvm;

Type* typeOf(int type, bool isarray, int array_size) {
    Type* tmp;
    switch(type) {
        case T_INTEGER:
            tmp = Type::getInt32Ty(getGlobalContext());
            break;
        case T_FLOAT:
            tmp = Type::getFloatTy(getGlobalContext());
            break;
        case T_STRING:
            tmp = Type::getInt8Ty(getGlobalContext());
            break;
        default:
            tmp = Type::getVoidTy(getGlobalContext());
    }
    if (!isarray) {
        return tmp;
    }
    else {
        return ArrayType::get(tmp, array_size);
    }
}

Main_block::Main_block() {
    this->s = new Structure();
    this->function_position = -1;
}

void Main_block::add_var(VariableAST* var) {
    FunctionAST *func = this->functions.back();
    func->add_var(var);
}
void Main_block::add_gvar(VariableAST* var) {
    FunctionAST *func = get_main_func();
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
void Main_block::add_mask(int m) {
    FunctionAST *func = this->functions.back();
    func->add_mask(m);
}

void Main_block::pop_special() {
    FunctionAST *func = this->functions.back();
    func->pop_special();
}
void Main_block::pop_function() {
    functions.erase(functions.begin()+1);
}
void Main_block::change_state() {
    FunctionAST *func = this->functions.back();
    func->change_state();
}
void Main_block::reset() {
    int total = this->functions.size() -1;
    if (this->function_position == total) {
        this->function_position = 0;
    }
}
FunctionAST* Main_block::get_func() {
    std::vector<FunctionAST*>::iterator it = functions.begin();
    return *(it + this->function_position);
}
FunctionAST* Main_block::get_func(int pos) {
    std::vector<FunctionAST*>::iterator it = functions.begin();
    return *(it + pos);
}


void Main_block::codegen() {
    this->mod = llvm::make_unique<Module>(this->name, getGlobalContext());
    std::vector<Type *> args;

    //addstruct code
    this->s->codegen(this);

    //forward declarion of functions
    for (auto it = (this->functions.begin() + 1); it != this->functions.end(); it++) {
        FunctionAST* f = (*it);
        std::vector<Type *> args_type = f->get_args_type();
        Module* mod = get_module();
        std::string struct_name("struct.");
        struct_name.append(f->get_name());
        Type* StructTy = mod->getTypeByName(struct_name);
        if (!StructTy)  {
            StructTy = Type::getVoidTy(getGlobalContext());
        }

        FunctionType *ftype = FunctionType::get(StructTy, args_type, false);
        Constant* c = mod->getOrInsertFunction(f->get_name(), ftype);
        Function *func = cast<Function>(c);
    }

    std::vector<FunctionAST*>::iterator it;
    for (it = (this->functions).begin(); it != (this->functions).end(); it++) {
        (*it)->codegen(this);
    }



    ReturnInst::Create(getGlobalContext(), get_block());
}
void Structure::add_structure(std::string var_name, Type* t) {
    struct structure s;
    s.state = state;
    s.type = t;
    s.var_name = var_name;
    this->structures.push_back(s);
}
void Structure::codegen(Main_block* mblock) {
    Debug("Creating structure");
    Module* mod = mblock->get_module();
    FunctionAST* func = mblock->get_main_func();

    int state = -1;
    StructType* structTy;
    std::vector<Type*> structTy_fields;
    for (auto it = structures.begin(); it != structures.end(); it++) {
        if (state == (*it).state) {
            structTy_fields.push_back((*it).type);
        }
        else {
            if (state > -1) {
                structTy->setBody(structTy_fields, false);
            }
            FunctionAST* tmp_func = mblock->get_func(state+2);
            std::string struct_name("struct.");
            struct_name.append(tmp_func->get_name());
            Debug("struct name", struct_name.c_str());
            structTy = StructType::create(mod->getContext(), struct_name);
            structTy_fields.clear();
            structTy_fields.push_back((*it).type);
            state = (*it).state;
        }

        if ((it+1) == structures.end()) {
            structTy->setBody(structTy_fields, false);
            std::string struct_name("struct.");
            struct_name.append(std::to_string((*it).state));
        }
    }
}

std::vector<std::string> Structure::get_vars(Main_block* mblock, std::string func_name) {
    std::vector<std::string> variables;
    int state = -1;
    for (auto it = structures.begin(); it != structures.end(); it++) {
        FunctionAST* func = mblock->get_func( (*it).state +1);
        if (func->get_name() == func_name) {
            variables.push_back( (*it).var_name );
        }
    }
    return variables;
}
void FunctionAST::add_arg(std::string var, llvm::Type* var_type) {
    if (this->args[var]) {
        Error("Argument invalid. Already assigned");
    }
    Debug("In function...", get_name().c_str());
    Debug("adding var", var.c_str());
    this->args[var] = var_type;
    this->local_var[var] = nullptr;
}
void FunctionAST::add_var(VariableAST* var) {
    if (this->local_var[var->get_name()]) {
        Error("Variable already created!");
    }
    this->local_var[var->get_name()] = nullptr;
    list_statement.push_back(var);
}
void FunctionAST::add_statement(BasicAST* stat) {
    if (get_state() == 0)
        this->list_statement.push_back(stat);
    else {
        this->sblocks->add_statement(stat);
    }
}
void FunctionAST::add_special(FlowBlock* special, int type) {
    Debug("Creating one block");
    sblocks->add_block(special, type);
    inc_state();
}
void FunctionAST::add_return(Main_block* mblock) {
    Debug("Creating return");
    Module* mod = mblock->get_module();
    BasicBlock* current = mblock->get_block();
    std::string struct_name("struct.");
    struct_name.append(get_name());
    StructType* StructTy = mod->getTypeByName(struct_name);
    if (StructTy) {
        AllocaInst* ptr_ret = new AllocaInst(StructTy, "return", current );
        //Add arguments to call function
        mblock->get_struct_vars(get_name());
        auto mask = get_mask();
        auto mask_iterator = mask.begin();
        std::vector<Value*> params;
        auto return_vars = mblock->get_struct_vars(get_name()); //get variables name of the structure which will be return
        int struct_count = 0;
        ConstantInt* const_i32_zero = ConstantInt::get(mod->getContext(), APInt(32, 0, 10));
        for (auto it = return_vars.begin(); it != return_vars.end(); it++, struct_count++) {
            //Load values to return here
            auto arg = *it;
            Value *v = get_var(arg);
            ConstantInt* const_element = ConstantInt::get(mod->getContext(), APInt(32, struct_count, 10));
            std::vector<Value*> ptr_indices;
            ptr_indices.push_back(const_i32_zero);
            ptr_indices.push_back(const_element);
            Instruction* ptr_25 = GetElementPtrInst::Create(ptr_ret, ptr_indices, "val", current);
            LoadInst* ret1 = new LoadInst(v,"ret",false, current);
            StoreInst* void_26 = new StoreInst(ret1, ptr_25, false, current);
        }

        LoadInst* ret =  new LoadInst(ptr_ret, "", false, current);
        ReturnInst::Create(getGlobalContext(), ret, current );
    } else {
        ReturnInst::Create(getGlobalContext(), current );
    }
}
void FunctionAST::pop_special() {
    Debug("Leaving one block");
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
    special->set_type(type);
    this->blocks.push_back(special);
}
FlowBlock* SpecialBlock::get_block() {
    return this->blocks.back();
}
FlowBlock* SpecialBlock::pop_block() {
    FlowBlock* sb = this->blocks.back();
    this->blocks.pop_back();
    return sb;
}
void SpecialBlock::change_state() {
    FlowBlock* sb = this->blocks.back();
    sb->change_state();
}
void SpecialBlock::add_statement(BasicAST* stat) {
    FlowBlock* sb = get_block();
    sb->add_statement(stat);
}
void CondAST::add_condition(BasicAST* lhs, int operand, BasicAST* rhs) {
    this->lhs.push_back(lhs);
    this->rhs.push_back(rhs);
    this->operand.push_back(operand);
}
void CondAST::add_statement(BasicAST* stat) {
    int state = get_state();
    Debug("Adding something in condition!");
    stat->print();
    if (!state) {
        this->true_statements.push_back(stat);
    } else {
        this->false_statements.push_back(stat);
    }
}
//generate a function and insert into the module
llvm::Value* FunctionAST::codegen(Main_block *mblock) {
    std::vector<Type *> args_type = get_args_type();
    Module* mod = mblock->get_module();
    std::string struct_name("struct.");
    struct_name.append(get_name());
    Type* StructTy = mod->getTypeByName(struct_name);
    if (!StructTy)  {
        StructTy = Type::getVoidTy(getGlobalContext());
    }

    FunctionType *ftype = FunctionType::get(StructTy, args_type, false);
    //Function *func = Function::Create(ftype, GlobalValue::InternalLinkage, this->name,  mblock->get_module());
    Constant* c = mod->getOrInsertFunction(get_name(), ftype);
    Function *func = cast<Function>(c);
    this->func = func;
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), "entry", func);
    mblock->inc_function();

    Debug("Function created", this->name.c_str());
    this->bb = bb;
    mblock->add_block(bb);
    Builder.SetInsertPoint(bb);

    auto my_arg = this->args.begin();
    for (auto argument = func->arg_begin(); argument != func->arg_end(); ++argument, my_arg++) {
        Debug("Argument", (my_arg->first).c_str());
        Value* x = argument;
        x->setName(my_arg->first);
        this->local_var[my_arg->first] = x;
    }

    for (auto it = this->args.begin(); it != this->args.end(); it++){
        Value* val = get_var(it->first);
        AllocaInst* tmp_ptr = new AllocaInst(val->getType(), "", bb);
        StoreInst* tmp_store = new StoreInst(val, tmp_ptr, false, bb);
        this->local_var[it->first] = tmp_ptr;
        //LoadInst* val_load = new LoadInst( tmp_store , "load_instr_arg", false, bb);
    }


    for (auto it = list_statement.begin(); it != list_statement.end(); it++) {
        (*it)->codegen(mblock);
    }

    return nullptr;

}
//change the value of a variable inside a function
int FunctionAST::change_var_value(std::string var, llvm::Value* value) {
    Debug("CHANGING var ", var.c_str());
    this->local_var[var] = value;
    return 1;
}
//allocate memory for a variable
Value* VariableAST::codegen(Main_block* mblock) {
    AllocaInst *alloc;
    Type* type;
    FunctionAST *func = mblock->get_func();
    Debug("Creating var ", (this->name).c_str());
    Debug("at", func->get_name().c_str());

    Module* mod = mblock->get_module();
    GlobalVariable* gvar = mod->getGlobalVariable(this->name, true);
    if (func->get_var(this->name) || gvar) {
        Error("Variable already created!*");
    }
    type = typeOf(this->var_type, get_array(), get_array_pos());

    if (get_global()) {
        gvar = new GlobalVariable(/*Module=*/*mod, /*Type=*/type,/*isConstant=*/false,
                /*Linkage=*/GlobalValue::ExternalLinkage, /*Initializer=*/0,  /*Name=*/this->name );

        Constant *const_val = Constant::getNullValue(type);
        gvar->setInitializer(const_val);

        func->change_var_value(this->name, gvar);
        return gvar;
    }
    alloc = new AllocaInst(type , this->name.c_str(), mblock->get_block());


    func->change_var_value(this->name, alloc);
    return alloc;
}

Value* AssignAST::string_var(Main_block* mblock, std::string var_name) {
    Module* mod = mblock->get_module();
    FunctionAST* func = mblock->get_func();
    StringAST* str = get_string();
    std::string val = str->get_value();
    int size_var = val.size();
    llvm::Value *var;
    var = func->get_var(get_name());
    GlobalVariable* gvar = mod->getGlobalVariable(get_name(), true);
    if (!var) {
        var = gvar;
    }

    ArrayType* array_string = ArrayType::get( IntegerType::getInt8Ty(mod->getContext()) , size_var+1);
    PointerType* ptr_string = PointerType::get(array_string, 0);
    llvm::CastInst* cast1 = new llvm::BitCastInst(var, ptr_string, "", mblock->get_block());
    Constant *const_array = ConstantDataArray::getString(mod->getContext(), val, true);
    StoreInst* store_value = new StoreInst(const_array,cast1, mblock->get_block());

    if (!gvar) {
        func->change_var_value(var_name, cast1);
    }


    return store_value;
}

//generate code when assign a value to variable
Value* AssignAST::codegen(Main_block* mblock) {
    Debug("Assigning var", get_name().c_str());
    FunctionAST *func = mblock->get_func();
    Module* mod = mblock->get_module();
    llvm::Value* local_var = func->get_var(get_name());

    if (!is_string())  {
        std::vector<BasicAST*> rhs = this->rhs;
        llvm::Value* temp;
        for (auto it = rhs.begin(); it != rhs.end(); it++) {
            temp = (*it)->codegen(mblock);
        }

        llvm::Value* code = temp;
        Debug("*assigning ", this->get_name().c_str());
        std::vector<Value*> ptr_indices;
        if (get_array()) {
            ConstantInt* const_int32_zero = ConstantInt::get(mod->getContext(), APInt(32, 0, 10));
            ConstantInt* const_int32 = ConstantInt::get(mod->getContext(), APInt(32, get_array_pos(), 10));
            ptr_indices.push_back(const_int32_zero);
            ptr_indices.push_back(const_int32);
        }

        if (local_var) {
            if (get_array()) {
                Instruction* ptr_getelement = GetElementPtrInst::Create(local_var, ptr_indices, "", mblock->get_block());
                return new StoreInst(  code, ptr_getelement, false, mblock->get_block());
            }
            return new StoreInst(code, local_var, false, mblock->get_block());
        } else {
            GlobalVariable* gvar = mod->getGlobalVariable(get_name(), true);
            if (gvar) {
                if (get_array()) {
                    Instruction* ptr_getelement = GetElementPtrInst::Create(gvar, ptr_indices, "", mblock->get_block());
                    return new StoreInst(  code, ptr_getelement, false, mblock->get_block());
                }
                return new StoreInst(code, gvar, false, mblock->get_block());
            }
        }
    }
    else {
        return string_var(mblock, get_name());
    }
    return nullptr;
}
//generate the message for the putXXXX method
void CallFuncAST::set_message(int type) {
    std::string message;
    switch (type) {
        case F_PUTBOOL:
            message = "%d\x0A";
            this->g_var = "bool";
            break;
        case F_PUTCHAR:
            message = "%c\x0A";
            this->g_var = "char";
            break;
        case F_PUTFLOAT:
            message = "%f\x0A";
            this->g_var = "float";
            break;
        case F_PUTINTEGER:
            message = "%d\x0A";
            this->g_var = "int";
            break;
        case F_PUTSTRING:
            message = "%s\x0A";
            this->g_var = "string";
            break;
        case F_GETINTEGER:
            message = "%d";
            this->g_var = "integer";
            break;
        case F_GETSTRING:
            message = "%s";
            this->g_var = "string";
    }
    this->message = message;
}

//get the function defintion in the LLVM module
FunctionAST* CallFuncAST::get_func(Main_block* mblock){
    this->functions = mblock->get_functions();

    for (auto it = functions.begin(); it != functions.end(); it++) {
        FunctionAST* f = *(it);
        if (f->get_name() == get_name()) {
            return *(it);
        }
    }
    return nullptr;
}

void call_malloc(Module* mod) {

    PointerType* PointerTy = PointerType::get(IntegerType::get(mod->getContext(), 8), 0);
    std::vector<Type*>FuncTy_args;

    FuncTy_args.push_back(IntegerType::get(mod->getContext(), 64));
    FunctionType* FuncTy = FunctionType::get(/*Result=*/PointerTy,/*Params=*/FuncTy_args,/*isVarArg=*/false);

    Function* func_malloc = mod->getFunction("malloc");
    if (!func_malloc) {
        func_malloc = Function::Create(/*Type=*/FuncTy,/*Linkage=*/GlobalValue::ExternalLinkage,/*Name=*/"malloc", mod); // (external, no body)
        func_malloc->setCallingConv(CallingConv::C);
    }
}
//generate code when a function is called
Value* CallFuncAST::codegen(Main_block* mblock) {
    Debug("Calling function: ", get_name().c_str());

    Module* mod = mblock->get_module();
    Function* func = mod->getFunction(get_name());
    FunctionAST* func_block = mblock->get_func();
    CallInst *call;

    if (!func) {
        PointerType* PointerTy = PointerType::get(IntegerType::get(mod->getContext(), 8), 0);
        std::vector<Type*>FuncTy_args;
        FuncTy_args.push_back(PointerTy);
        FunctionType* FuncTy = FunctionType::get(/*Result=*/IntegerType::get(mod->getContext(), 32),
                                                /*Params=*/FuncTy_args,
                                                /*isVarArg=*/true);
        func = Function::Create(FuncTy, GlobalValue::ExternalLinkage, get_name(), mblock->get_module()); // (external, no body)
        func->setCallingConv(CallingConv::C);

    }
    if (get_external()) { //external function like putInteger(i32); which is equal to printf("%d", i32);
        Debug("Calling external function");
        std::string var_name;
        int message_size;
        CallInst* ptr_malloc;
        Function* func_malloc;
        StoreInst* scanf_store;
        ConstantInt* const_int64_16 = ConstantInt::get(mod->getContext(), APInt(64, StringRef("10"), 10));
        if (get_name().compare("scanf") == 0) {
            call_malloc(mod);
            func_malloc = mod->getFunction("malloc");
            ptr_malloc = CallInst::Create(func_malloc, const_int64_16, "", mblock->get_block());
            ptr_malloc->setCallingConv(CallingConv::C);
            ptr_malloc->setTailCall(false);
            var_name = "scanf." + get_gvar();
            message_size = 3;
        } else {
            var_name =  "printf." + get_gvar();
            message_size = 4;
        }

        GlobalVariable* gvar = mod->getGlobalVariable(var_name, true);

        if (!gvar) {
            ArrayType* ArrayTy = ArrayType::get(IntegerType::get(mod->getContext(), 8), message_size);
            gvar = new GlobalVariable(/*Module=*/*mod, /*Type=*/ArrayTy,/*isConstant=*/true,
                /*Linkage=*/GlobalValue::PrivateLinkage, /*Initializer=*/0,  /*Name=*/var_name );
        }

        Constant *const_array = ConstantDataArray::getString(mod->getContext(), get_message(), true);
        gvar->setInitializer(const_array);

        ConstantInt* const_int32_zero = ConstantInt::get(mod->getContext(), APInt(32, StringRef("0"), 10));

        std::vector<Constant*> const_indices;
        const_indices.push_back(const_int32_zero);
        const_indices.push_back(const_int32_zero);
        Constant* const_ptr = ConstantExpr::getGetElementPtr(gvar , const_indices);
        std::vector<Value*> params;
        params.push_back(const_ptr);


        if (get_name().compare("scanf") == 0) {
            Value* val;
            GlobalVariable* gval = mod->getGlobalVariable(get_var(), true);
            if (gval)
                val = gval;
            else
                val = func_block->get_var(get_var());

            if (get_gvar().compare("string") == 0) {
                Type* string_ptr = Type::getInt8PtrTy(mod->getContext(), 0);
                Type* string_ptr_ptr = PointerType::get(string_ptr, 0);
                BitCastInst* b_string = new llvm::BitCastInst(val, string_ptr_ptr, "", mblock->get_block() );
                scanf_store = new StoreInst(ptr_malloc, b_string, mblock->get_block());
                LoadInst* scan_val = new LoadInst(b_string, "scanf_val", mblock->get_block());
                val = scan_val;
                if (!gval)
                    func_block->change_var_value(get_var(), scan_val);
            }


            ///TODO: make it work with global variables...
            params.push_back(val);
        }
        else {
            if (isdigit(get_var()[0])) {
                Constant* val = ConstantInt::get(Type::getInt32Ty(getGlobalContext()), std::stoi(get_var()) );
                params.push_back(val);
            } else if (has_string) {
                StringAST* s_message = new StringAST(get_string_message());
                params.push_back(s_message->codegen(mblock));
            }
            else {
                std::vector<Value*> ptr_indices;
                Value* val;
                GlobalVariable* gval = mod->getGlobalVariable(get_var(), true);
                if (gval)
                    val = gval;
                else
                    val = func_block->get_var(get_var());
                LoadInst* val_load;
                bool load = false;
                if (get_array()) {
                    load = true;
                    ConstantInt* const_int32_zero = ConstantInt::get(mod->getContext(), APInt(32, 0, 10));
                    ConstantInt* const_int32 = ConstantInt::get(mod->getContext(), APInt(32, get_array_pos(), 10));
                    ptr_indices.push_back(const_int32_zero);
                    ptr_indices.push_back(const_int32);
                    Instruction* ptr_getelement = GetElementPtrInst::Create(val, ptr_indices, "", mblock->get_block());
                    val_load = new LoadInst(ptr_getelement, "array_element", false, mblock->get_block());
                } else {
                    if (get_message().compare("%s\n") == 0) {
                        Type* string_ptr = Type::getInt8PtrTy(mod->getContext(), 0);
                        BitCastInst* b_string = new llvm::BitCastInst(val, string_ptr, "", mblock->get_block() );
                        params.push_back(b_string);
                    } else {
                        load = true;
                        val_load = new LoadInst( val, "", false, mblock->get_block());
                    }
                }
                if (load)
                    params.push_back(val_load);
            }
        }
        call = CallInst::Create(func, params, "", mblock->get_block());

    }
    else { //call internal functions
        Debug("Internal function call", func_block->get_name().c_str());
        FunctionAST* callee_func = get_func(mblock);
        auto mask = callee_func->get_mask();
        auto mask_iterator = mask.begin();
        int count_mask = 0;
        std::vector<Value*> params;
        for (auto it = real_args.begin(); it != real_args.end(); it++, count_mask++) {
            if (*(mask_iterator + count_mask)) {
                params.push_back( (*it)->codegen(mblock) );
            }
        }
        //load return for the function called for the OUT variables
        std::string struct_name("struct.");
        struct_name.append(callee_func->get_name());
        Type* StructTy = mod->getTypeByName(struct_name);
        if (!StructTy)  {
            StructTy = Type::getVoidTy(getGlobalContext());
            call = CallInst::Create(func, params, "", mblock->get_block());
        }
        else {
            AllocaInst* ptr_struct = new AllocaInst(StructTy, "ret_structure", mblock->get_block());
            call = CallInst::Create(func, params, "", mblock->get_block());
            StoreInst* store_func_return = new StoreInst(call, ptr_struct, "function_return", mblock->get_block());
            //load values of the return structure to the function which called this function
            mask = callee_func->get_mask();
            mask_iterator = mask.begin();
            count_mask = 0;
            int count_getelement = 0;
            args = get_args();
            ConstantInt* const_int32_zero = ConstantInt::get(mod->getContext(), APInt(32, 0, 10));
            for (auto it = args.begin(); it != args.end(); it++, count_mask++) {
                if (!*(mask_iterator + count_mask)) {
                    auto arg = (*it);
                    ConstantInt* const_int32 = ConstantInt::get(mod->getContext(), APInt(32, count_getelement, 10));
                    std::vector<Value*> ptr_indices;
                    ptr_indices.push_back(const_int32_zero);
                    ptr_indices.push_back(const_int32);
                    Instruction* ptr_getelement = GetElementPtrInst::Create(ptr_struct, ptr_indices, "", mblock->get_block());
                    LoadInst* load_struct = new LoadInst(ptr_getelement, "element_func", false, mblock->get_block());
                    StoreInst* void_26 = new StoreInst(  load_struct, func_block->get_var(arg), false, mblock->get_block());
                    count_getelement++;
                }
            }
        }



    }
    return nullptr;
}

//generate code for an integer constant
Value* IntegerAST::codegen(Main_block* mblock) {
    Debug("Creating integer", std::to_string(value).c_str());
    return ConstantInt::get(typeOf(T_INTEGER,0,0), value);
}
//generate code for a float constant
Value* FloatAST::codegen(Main_block* mblock) {
    Debug("Creating float", std::to_string(value).c_str());
    return ConstantFP::get(typeOf(T_FLOAT,0,0), value);
}
//generate code for a float constant
Value* StringAST::codegen(Main_block* mblock) {
    Debug("Creating string", value.c_str());
    Module* mod = mblock->get_module();

    AllocaInst* tmp_string = new AllocaInst(typeOf(T_STRING,1, value.size()+1), "", mblock->get_block());
    Constant *const_array = ConstantDataArray::getString(mod->getContext(), value, true);
    StoreInst* store_value = new StoreInst(const_array, tmp_string, mblock->get_block());
    PointerType* ptr_string = PointerType::get(typeOf(T_STRING,0,0), 0);
    llvm::CastInst* cast1 = new llvm::BitCastInst(tmp_string, ptr_string, "", mblock->get_block());

    return cast1;
}

//generate code for an arithmetic binary expression
Value* BinopAST::codegen(Main_block* mblock) {
    Debug("Creating Binary operation: ", std::to_string(op).c_str());
    llvm::Value* lhs = (this->op1)->codegen(mblock);
    llvm::Value* rhs = (this->op2)->codegen(mblock);
    Value* binop;
    if (this->op == '+') {
        binop = BinaryOperator::Create(Instruction::Add, rhs, lhs, "add", mblock->get_block());
        if (get_type()) {
            Value *var = mblock->get_func()->get_var(get_var());
            StoreInst* store = new StoreInst(binop, var, false, mblock->get_block());
        }
    }

    else if (this->op == '-')
        binop = BinaryOperator::Create(Instruction::Sub, rhs, lhs, "sub", mblock->get_block());
    else if (this->op == '*')
        binop = BinaryOperator::Create(Instruction::Mul, rhs, lhs, "mul", mblock->get_block());
    else if (this->op == '/')
        binop = BinaryOperator::Create(Instruction::SDiv, rhs, lhs, "div", mblock->get_block());

    return binop;
}

//generate code whenever you change the value of a var
Value* CallVarAST::codegen(Main_block* mblock) {
    Debug("Getting var ", get_var().c_str());
    std::string var_name = get_var();
    FunctionAST *func = mblock->get_func();
    Module* mod = mblock->get_module();
    GlobalVariable* gvar = mod->getGlobalVariable(var_name, true);
    Value* val;
    std::vector<Value*> ptr_indices;

    if (get_array()) {
        ConstantInt* const_int32_zero = ConstantInt::get(mod->getContext(), APInt(32, 0, 10));
        ConstantInt* const_int32 = ConstantInt::get(mod->getContext(), APInt(32, get_array_pos(), 10));
        ptr_indices.push_back(const_int32_zero);
        ptr_indices.push_back(const_int32);
    }

    if (!gvar) {
        if (get_array()) {
            Instruction* ptr_getelement = GetElementPtrInst::Create(val, ptr_indices, "", mblock->get_block());
            return new LoadInst(  ptr_getelement, "", false, mblock->get_block());
        }
        return new LoadInst(val, "", false, mblock->get_block());
    } else {
        if (get_array()) {
            Instruction* ptr_getelement = GetElementPtrInst::Create(gvar, ptr_indices, "", mblock->get_block());
            return new LoadInst(  ptr_getelement, "", false, mblock->get_block());
        }
        return new LoadInst(gvar, "", false, mblock->get_block());
    }
}

//generate return of a function and remove function from the statement list
Value* EndFunctionAST::codegen(Main_block* mblock) {
    Debug("End of function code");
    FunctionAST *func = mblock->get_func();
    func->add_return(mblock);
    if (!get_return()) {
        mblock->reset(); //work around, because function was not returning correctly... not the best practice

        mblock->pop_block();
    }

    Builder.SetInsertPoint(mblock->get_block());
    if (get_return()) {
        llvm::BasicBlock* ret = BasicBlock::Create(getGlobalContext(), "not_ret", func->get_func());
        mblock->pop_block();
        mblock->add_block(ret);
        Builder.SetInsertPoint(ret);
    }
    return nullptr;
}

//generate condtional statements
Value* CondAST::codegen(Main_block* mblock) {
    Debug("Creating conditional block");
    llvm::Function* func = mblock->get_func()->get_func();
    llvm::BasicBlock* current_block = mblock->get_block();
    llvm::BasicBlock* cond_true = BasicBlock::Create(getGlobalContext(), "cond_true", func);
    llvm::BasicBlock* cond_false = BasicBlock::Create(getGlobalContext(), "cond_false", func);
    llvm::BasicBlock* merge_cond;

    if (!get_in_for()) merge_cond = BasicBlock::Create(getGlobalContext(), "cond_cont", func);

    Value* lhs = (this->lhs.back())->codegen(mblock);
    Value* rhs = (this->rhs.back())->codegen(mblock);
    Value* comp;

    //check types. if one is  pointer, load its value
    Type* t1 = lhs->getType();
    Type* t2 = rhs->getType();
    if (t1->isPointerTy()) {
        lhs =  new LoadInst(lhs, "", false, current_block);
    }
    if (t2->isPointerTy()) {
        rhs = new LoadInst(rhs, "", false, current_block);
    }
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
    //create entry when condition is true
    Builder.SetInsertPoint(cond_true);
    mblock->add_block(cond_true);
    std::vector<BasicAST*> tmp = get_true_statement();
    for (auto it = tmp.begin(); it != tmp.end(); it++) {
        (*it)->codegen(mblock);
    }
    mblock->pop_block();
    if (!get_in_for()) Builder.CreateBr(merge_cond);

    //create entry when condition is false
    Builder.SetInsertPoint(cond_false);
    mblock->add_block(cond_false);
    tmp = get_false_statement();
    for (auto it = tmp.begin(); it != tmp.end(); it++) {
        (*it)->codegen(mblock);
    }
    mblock->pop_block();
    if (!get_in_for()) {
        Builder.CreateBr(merge_cond);

        mblock->pop_block();
        mblock->add_block(merge_cond);
        Builder.SetInsertPoint(merge_cond);
    }

    Debug("Leaving conditional block");
    return nullptr;
}

Value* ForAST::codegen(Main_block* mblock) {
    Debug("Creating for block");
    FunctionAST* func_ast = mblock->get_func();
    llvm::Function* func = func_ast->get_func();
    this->cond->set_in_for(true);
    llvm::BasicBlock* for_loop = BasicBlock::Create(getGlobalContext(), "for_loop", func);
    llvm::BasicBlock* for_continue = BasicBlock::Create(getGlobalContext(), "for_continue", func);

    //branch instructions of the for loop
    BranchAST* repeat = new BranchAST(for_loop);
    BranchAST* endfor = new BranchAST(for_continue);

    //init of for loop
    repeat->codegen(mblock);
    Builder.SetInsertPoint(for_loop);
    mblock->add_block(for_loop);

    //add statements to the end of for loop (increment and branch instructions)
    CallVarAST* var = new CallVarAST(get_incr_var());
    IntegerAST* one = new IntegerAST(1);
    BinopAST* incr = new BinopAST('+', var, one);
    incr->set_type(1, get_incr_var());
    add_statement(incr);
    add_statement(repeat);

    //add statements to continue the function when condition is not met anymore
    this->cond->change_state();
    add_statement(endfor);
    this->cond->codegen(mblock);
    mblock->pop_block();

    mblock->pop_block();
    mblock->add_block(for_continue);
    Builder.SetInsertPoint(for_continue);


    return nullptr;
}

Value* BranchAST::codegen(Main_block* mblock) {
    Debug("Branch Instruction");
    Builder.CreateBr(branch);
    return nullptr;
}
