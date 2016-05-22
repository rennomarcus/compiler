#include "parser.hpp"


using namespace llvm;

//return the type of the variable for allocation and arguments of functions
Type* typeOf(int type) {
    if (type == T_INTEGER) {
        return Type::getInt32Ty(getGlobalContext());
    }
    if (type == T_FLOAT) {
        return Type::getFloatTy(getGlobalContext());
    }
    return Type::getVoidTy(getGlobalContext());
}

//top level parser
void parser(Scan_file* scan, Main_block* mblock) {

    switch (scan->get_tok()) {
        //case T_EOF:
            //return 0;
        case T_PROGRAM:
            HandleProgram(scan, mblock);
            break;
        case T_GLOBAL: case T_INTEGER: case T_FLOAT: case T_STRING:
            HandleVariableDeclaration(scan, mblock);
            break;
        case T_PROCEDURE:
            HandleFunction(scan, mblock);
            break;
        case T_END:
            //pop block and set BB
            HandleEnd(scan, mblock);
            break;
        case T_IF:
            HandleIfBlock(scan, mblock);
            break;
        case T_ELSE:
            HandleElseBlock(scan, mblock);
            break;
        case T_FOR:
            HandleForBlock(scan, mblock);
            break;
        case T_RETURN:
            HandleReturn(scan, mblock);
            break;
        default:
            HandleTopLevelExpression(scan, mblock);
            break;

    }

}

void HandleReturn(Scan_file* scan, Main_block* mblock) {
    Debug(">>>>>>>>>>>>>>>>>>>>>>>>> Im here");
    if (scan->scan_tok() == T_SEMICOLON) {
        EndFunctionAST* endfunc = new EndFunctionAST(true);
        mblock->add_statement(endfunc);
    } else {
        Error("Missing semicolon after return.");
    }
}

//check (if, then, else) blocks
void HandleForBlock(Scan_file* scan, Main_block* mblock) {
    ForAST* forblock = new ForAST();
    CondAST* cond;
    Debug("If block");
    if (scan->get_tok() == T_FOR) {
        if (scan->scan_tok() == T_LPAREN) {
            scan->scan_tok();
            forblock->add_incr_var(scan->get_value());
            HandleStatement(scan, mblock);
            cond = HandleIfCond(scan, mblock, 0);
            forblock->add_cond(cond);
            mblock->add_special(forblock, 2);
        } else {
            Error("Missing left parenthesis after the FOR keyword");
        }
    }
}

//check (if, then, else) blocks
void HandleIfBlock(Scan_file* scan, Main_block* mblock) {
    CondAST* cond;
    Debug("If block");
    if (scan->get_tok() == T_IF) {
        scan->scan_tok();
        if (scan->get_tok() == T_LPAREN) {
            cond = HandleIfCond(scan, mblock, 1);
            mblock->add_special(cond, 1);
        }
    }
}
void HandleElseBlock(Scan_file* scan, Main_block* mblock) {
    mblock->change_state();
}

//create condition
CondAST* HandleIfCond(Scan_file* scan, Main_block* mblock, bool check_then) {
    int tok1 = scan->scan_tok();
    std::string value = scan->get_value();
    CondAST* cond = new CondAST();
    if (scan->scan_tok() == T_RPAREN) {
        if (tok1 == T_FALSE || value.compare("0") == 0) {
            IntegerAST* lhs = new IntegerAST(1);
            IntegerAST* rhs = new IntegerAST(0);
            cond->add_condition(lhs, COND_EQUAL, rhs);
        }
        else if (tok1 == T_TRUE || value.compare("0") != 0) {
            //create cond to compareto not 0
            IntegerAST* lhs = new IntegerAST(1);
            IntegerAST* rhs = new IntegerAST(1);
            cond->add_condition(lhs, COND_EQUAL,rhs);
        }
        else {
            Error("Invalid condition");
        }
    }
    else {
        while (scan->get_tok() != T_RPAREN) {
            BasicAST* lhs = convert_tok_ast(tok1, value);
            int operand = HandleIfOperator(scan, mblock);
            BasicAST* rhs = convert_tok_ast(scan->get_tok(), scan->get_value());
            cond->add_condition(lhs, operand, rhs);
            scan->scan_tok();
        }
        if (check_then)
            scan->scan_tok();
    }

    return cond;

}

BasicAST* convert_tok_ast(int tok_type, std::string tok_value) {
    BasicAST* ast;
    switch (tok_type) {
        case T_INTEGER:
            ast = new IntegerAST(stoi(tok_value));
            break;
        case T_FLOAT:
            ast = new FloatAST(stof(tok_value));
            break;

        default:
            ast = new CallVarAST(tok_value);
    }
    return ast;
}
int HandleIfOperator(Scan_file* scan, Main_block* mblock){
    int tok = scan->get_tok();
    switch (tok) {
        case '=':
            if (scan->scan_tok() != '=')
                Error("Invalid operator in if statement");
            scan->scan_tok();
            return COND_EQUAL;
        case '!':
            if (scan->scan_tok() != '=')
                Error("Invalid operator in if statement");
            scan->scan_tok();
            return COND_NOT_EQUAL;
        case '<':
            if (scan->scan_tok() == '=') {
                scan->scan_tok();
                return COND_LESS_EQUAL;
            }
            return COND_LESS;
        case '>':
            if (scan->scan_tok() == '=') {
                scan->scan_tok();
                return COND_GREATER_EQUAL;
            }
            return COND_GREATER;
        default:
            Error("Invalid operator in if statement");
    }
}

//check assigning and call functions. then it will add to current function
void HandleTopLevelExpression(Scan_file* scan, Main_block* mblock) {
    Debug("top level", std::to_string(scan->get_tok()).c_str() );
    switch(scan->get_tok()) {
        case T_BEGIN:
            break;
        case T_IDENTIFIER:
            //IDENT ASSIGN VALUE | 272 276 (???) assign value
            //IDENT LEFT_PARENT... | call function
            HandleStatement(scan, mblock);
            break;
        default:
            ExternalFunctions(scan, mblock);
    }
}

//3 + 4 * 10
std::vector<BasicAST*> HandleMath(Scan_file* scan, Main_block* mblock) {
    Shunting* equation = new Shunting(scan);
    equation->print();
    return equation->read();
}

//"message"
StringAST* HandleString(Scan_file* scan, Main_block* mblock) {
    StringAST* message = new StringAST(scan->get_value());
    if (scan->scan_tok() != T_SEMICOLON)
        Error("Missing ';' after string");
    return message;
}
void HandleCallFunc(Scan_file* scan, Main_block* mblock, CallFuncAST* func) {
    bool reading = false;
    while (scan->get_tok() != T_SEMICOLON) {
        scan->scan_tok();
        std::string tmp_arg = scan->get_value();
        BasicAST* tmp_basic;
        if (scan->get_tok() == T_STRING_MESSAGE) {
            tmp_arg = scan->get_value();
            tmp_basic = new StringAST(tmp_arg);
            reading = true;
        } else if (scan->get_tok() != T_RPAREN && scan->get_tok() != T_SEMICOLON) {
            reading = true;
            std::vector<BasicAST*> tmp = HandleMath(scan, mblock);

            for (auto it = tmp.begin(); it != tmp.end(); it++) {
                tmp_basic = *it;
                mblock->add_statement(*it);
            }
        }
        if (reading) {
            func->add_rarg(tmp_basic); //used as input
            func->add_arg(tmp_arg); //used as output
        }
    }
}
void HandleStatement(Scan_file* scan, Main_block* mblock) {
    Debug("handle expr");
    if (scan->get_tok() == T_IDENTIFIER) {
        std::string name = scan->get_value();
        if (scan->scan_tok() == T_LPAREN) {
            //handle call function;
            Debug("#call function", name.c_str());
            CallFuncAST* func = new CallFuncAST(name);
            HandleCallFunc(scan, mblock, func);
            mblock->add_statement(func);
        }
        if (scan->get_tok() == T_ASSIGN) {
            Debug("#var assigning");
            AssignAST* assign_var;
            if (scan->scan_tok() == T_STRING_MESSAGE) {
                assign_var = new AssignAST(name, HandleString(scan, mblock));

            } else {
                assign_var = new AssignAST(name, HandleMath(scan, mblock));
            }
            mblock->add_statement(assign_var);
        }
        if (scan->get_tok() == T_ARRAY) {
            Debug("#var array assign. Pos: ");
            int array_size = std::stoi(scan->get_value());
            if (scan->scan_tok() == T_ASSIGN) {
                scan->scan_tok();
                AssignAST* assign_var = new AssignAST(name, array_size, HandleMath(scan, mblock));
                assign_var->set_array(true);
                mblock->add_statement(assign_var);
            } else {
                Error("Invalid assignment");
            }
        }
    }
}
//type == 1 for print, ==0 for read...
void setExternalFunction(Scan_file* scan, Main_block* mblock, CallFuncAST* func, int type) {
    if (type == 1)
        func->set_name("printf");
    else
        func->set_name("scanf");

    int value = scan->get_tok();
    if (scan->scan_tok() != T_LPAREN)
        Error("Invalid function call");

    if (scan->scan_tok() != T_IDENTIFIER && scan->get_tok() != T_INTEGER && scan->get_tok() != T_FLOAT && scan->get_tok() != T_STRING_MESSAGE)
        Error("Invalid function call");
    if (scan->get_tok() == '"') {
        Debug("==================Constant string");
    }
    func->set_message(value);

    if (scan->get_tok() == T_STRING_MESSAGE) {
        func->set_string_message(scan->get_value());
    } else {
        func->set_var(scan->get_value());
    }
    if (scan->scan_tok() == T_ARRAY) {
        func->set_array(true);
        func->set_array_pos(std::stoi(scan->get_value()));
    }
    mblock->add_statement(func);
}
void ExternalFunctions(Scan_file* scan, Main_block* mblock) {
    CallFuncAST* func = new CallFuncAST;
    func->set_external();
    func->set_message(scan->get_tok());
    switch (scan->get_tok()) {
        case F_PUTBOOL: case F_PUTCHAR: case F_PUTINTEGER: case F_PUTSTRING:
            setExternalFunction(scan,mblock,func,1);
            break;
        case F_GETINTEGER: case F_GETSTRING:
            setExternalFunction(scan,mblock,func,2);
            break;
    }
}

//handle beginning and closing program
void HandleProgram(Scan_file *scan, Main_block *mblock) {
    scan->scan_tok();
    std::string name = scan->get_value();
    if (scan->scan_tok() != T_IS && state == 0) {
        Error("Missing \'is\' in program declaration");
    }
    else if (state == 0) {
        Debug("Create main func\n");
        state++;
        FunctionAST *main = new FunctionAST("main");
        mblock->add_func(main);
    }
}

//handles head of the function and allocation of variables
void HandleFunction(Scan_file *scan, Main_block *mblock) {

    if (state == 2) {
        state--;
        Debug("leaving function\n");
        EndFunctionAST* endfunc = new EndFunctionAST();
        mblock->add_statement(endfunc);
        mblock->inc_structure();
    }
    else {
        state++;
        Debug("create function ");
        if (scan->scan_tok() != T_IDENTIFIER) {
            Error("Invalid syntax for procedure");
        }
        std::string name = scan->get_value();
        if (scan->scan_tok() != T_LPAREN) {
            Error("Missing '('");
        }
        FunctionAST *func = new FunctionAST(name);
        mblock->add_func(func);
        scan->scan_tok();
        while (scan->get_tok() != T_RPAREN) {
            if (scan->get_tok() == T_INTEGER || scan->get_tok() == T_FLOAT)
                HandleVariableDeclaration(scan, mblock);
            if (scan->get_tok() != T_COMMA && scan->get_tok() != T_RPAREN) {
                Error("Invalid syntax in arguments");
            }
            else if (scan->get_tok() == T_COMMA) {
                scan->scan_tok();
            }
        }
    }
}

void HandleEnd(Scan_file *scan, Main_block *mblock) {
    if (scan->scan_tok() == T_IF) {
        if (scan->scan_tok() == T_SEMICOLON) {
            mblock->pop_special();
            Debug("Leaving if block");
        }
        else {
            Error("Missing ; after END IF statement");
        }
    }
    if  (scan->get_tok() == T_FOR) {
        if (scan->scan_tok() == T_SEMICOLON) {
            mblock->pop_special();
            Debug("Leaving for block");
        }
        else {
            Error("Missing ; after END FOR statement");
        }
    }
    else if (scan->get_tok() == T_PROCEDURE) {
        if (scan->scan_tok() != T_SEMICOLON) {
            Error("Missing ; after end procedure statement");
        }
        HandleFunction(scan,mblock);
        Debug("Leaving procedure block");
    }
    else if (scan->get_tok() == T_PROGRAM) {
        if (scan->scan_tok() != T_PERIOD) {
            Error("Missing . after end program statement");
        }
        Debug("Program finished");
    }
}
//handles variable allocation and variable arguments of function head
VariableAST* HandleVariableDeclaration(Scan_file *scan, Main_block *mblock) {
    Debug("Create var ");
    bool is_global = false;
    if (scan->get_tok() == T_GLOBAL) {
        scan->scan_tok();
        is_global = true;
    }

    int type = scan->get_tok();
    if (scan->scan_tok() != T_IDENTIFIER) {
        Error("Missing name for variable");
        return nullptr;
    }
    std::string var_name = scan->get_value();
    Debug("Var type", std::to_string(type).c_str());
    Debug("Var name", var_name.c_str());


    VariableAST *var = new VariableAST(var_name, type);
    if (type == T_STRING) {
        var->set_array(true);
        var->set_array_pos(255);
    }

    if (is_global)
        var->set_global();

    bool isarray = false;
    int array_size = 0;
    if (scan->scan_tok() == T_ARRAY) {
        isarray = true;
        array_size = std::stoi(scan->get_value());
        var->set_array(true);
        var->set_array_pos(array_size);
        Debug("ARRAY size", std::to_string(array_size).c_str());
        scan->scan_tok();
    }
    //if it is an internal variable add it to the block
    if (scan->get_tok() == T_SEMICOLON) { //go to next token = discard semicolon
        if (!is_global)
            mblock->add_var(var);
        else
            mblock->add_gvar(var);

        return var;
    }
    //otherwise,  it is an argument of a function. add as an argument to the function
    if (scan->get_tok() == T_IN || scan->get_tok() == T_OUT || scan->get_tok() == T_INOUT) {
        //add as arg to back block in mblock
        if (scan->get_tok() == T_IN) {
            mblock->add_arg(var_name, typeOf(type));
            mblock->add_mask(1);
        }

        if (scan->get_tok() == T_OUT) {
            mblock->add_var(var);
            mblock->add_structure(var_name, typeOf(type));
            mblock->add_mask(0);
        }


        scan->scan_tok();

        return var;
    }
}
