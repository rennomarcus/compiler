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
        case T_INTEGER: case T_FLOAT: case T_STRING:
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
        default:
            HandleTopLevelExpression(scan, mblock);
            break;

    }

}

//check (if, then, else) blocks
void HandleIfBlock(Scan_file* scan, Main_block* mblock) {
    CondAST* cond;
    std::cout << "If block" << std::endl;
    if (scan->get_tok() == T_IF) {
        scan->scan_tok();
        if (scan->get_tok() == T_LPAREN) {
            cond = HandleIfCond(scan, mblock);
            mblock->add_special(cond, 1);
        }
    }
}
void HandleElseBlock(Scan_file* scan, Main_block* mblock) {
    //scan->scan_tok();
}

//create condition
CondAST* HandleIfCond(Scan_file* scan, Main_block* mblock) {
    int tok1 = scan->scan_tok();
    std::string value = scan->get_value();
    CondAST* cond = new CondAST();
    if (scan->scan_tok() == T_RPAREN) {
        if (tok1 == T_TRUE) {
            //create cond to compareto not 0
            IntegerAST* lhs = new IntegerAST(1);
            IntegerAST* rhs = new IntegerAST(1);
            cond->add_condition(lhs, COND_EQUAL,rhs);
            std::cout << "UHU " << value << std::endl;
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
    std::cout << "top level " << scan->get_tok() << std::endl;
    switch(scan->get_tok()) {
        case T_BEGIN:
            break;
        case T_IDENTIFIER:
            //IDENT ASSIGN VALUE | 272 276 (???) assign value
            //IDENT LEFT_PARENT... | call function
            HandleStatement(scan, mblock);
            break;
    }
}

//3 + 4 * 10
std::vector<BasicAST*> HandleMath(Scan_file* scan, Main_block* mblock) {
    Shunting* equation = new Shunting(scan);
    equation->print();
    return equation->read();
}

void HandleStatement(Scan_file* scan, Main_block* mblock) {
    std::cout << "handle expr " << std::endl;

    if (scan->get_tok() == T_IDENTIFIER) {
        std::string name = scan->get_value();
        if (scan->scan_tok() == T_LPAREN) {
            //handle call function;
            std::cout << "#call function" << std::endl;
            while (scan->scan_tok() != T_RPAREN)
                ;
        }
        if (scan->get_tok() == T_ASSIGN) {

            std::cout << "#var assigning " << std::endl;
            AssignAST* assign_var = new AssignAST(name, HandleMath(scan, mblock));
            mblock->add_statement(assign_var);
        }
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
        std::cout << "Create main func\n";
        state++;
        FunctionAST *main = new FunctionAST(name);
        mblock->add_func(main);
    }
}

//handles head of the function and allocation of variables
void HandleFunction(Scan_file *scan, Main_block *mblock) {

    if (state == 2) {
        state--;
        std::cout << "leaving function\n";
        EndFunctionAST* endfunc = new EndFunctionAST();
        mblock->add_statement(endfunc);
    }
    else {
        state++;
        std::cout << "create function ";
        if (scan->scan_tok() != T_IDENTIFIER) {
            Error("Invalid syntax for procedure");
        }
        std::string name = scan->get_value();
        std::cout << name << std::endl;
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
            //mblock->add_statement(cond);
            mblock->pop_special();
            std::cout << "Leaving if block" << std::endl;
        }
        else {
            Error("Missing ; after end if statement");
        }
    }
    else if (scan->get_tok() == T_PROCEDURE) {
        if (scan->scan_tok() != T_SEMICOLON) {
            Error("Missing ; after end procedure statement");
        }
        HandleFunction(scan,mblock);
        std::cout << "Leaving procedure block" << std::endl;
    }
    else if (scan->get_tok() == T_PROGRAM) {
        if (scan->scan_tok() != T_PERIOD) {
            std::cout << "PERIOD" << scan->get_tok();
            Error("Missing . after end program statement");
        }
        std::cout << "Program finished" << std::endl;
    }
}
//handles variable allocation and variable arguments of function head
VariableAST* HandleVariableDeclaration(Scan_file *scan, Main_block *mblock) {
    std::cout << "Create var ";
    int type = scan->get_tok();
    if (scan->scan_tok() != T_IDENTIFIER) {
        Error("Missing name for variable");
        return nullptr;
    }
    std::cout << type << " " << scan->get_value() << std::endl;
    std::string var_name = scan->get_value();
    VariableAST *var = new VariableAST(var_name, type);

    //if it is an internal variable add it to the block
    if (scan->scan_tok() == T_SEMICOLON) { //go to next token = discard semicolon
        mblock->add_var(var);
        return var;
    }
    //otherwise,  it is an argument of a function. add as an argument to the function
    if (scan->get_tok() == T_IN || scan->get_tok() == T_OUT || scan->get_tok() == T_INOUT) {
        //add as arg to back block in mblock
        if (scan->get_tok() == T_IN)
            mblock->add_arg(var_name, typeOf(type));

        if (scan->get_tok() == T_OUT)
            ;///TODO create return type

        scan->scan_tok();

        return var;
    }
}
