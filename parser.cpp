#include "parser.hpp"


using namespace llvm;

//return the type of the variable for allocation and arguments of functions
Type *typeOf(int type) {
    if (type == T_INTEGER) {
        return Type::getInt32Ty(getGlobalContext());
    }
    if (type == T_FLOAT) {
        return Type::getFloatTy(getGlobalContext());
    }
    return Type::getVoidTy(getGlobalContext());
}

//top level parser
llvm::Value* parser(Scan_file *scan, Main_block *mblock) {

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
        default:
            HandleTopLevelExpression(scan, mblock);
            break;

    }
    llvm::Value* test;
    return test;
}

//check assigning and call functions. then it will add to current function
void HandleTopLevelExpression(Scan_file *scan, Main_block *mblock) {
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

llvm::Value* HandleStatement(Scan_file *scan, Main_block *mblock) {
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
            //HandleMath(scan, mblock);
            std::cout << "#var assigning " << std::endl;
            while (scan->scan_tok() != T_SEMICOLON)
                ;
        }
    }

    llvm::Value* test;
    return test;
}

/*
llvm::Value* HandleMath(Scan_file *scan, Main_block *mblock) {
    std::cout << scan->get_tok() << std::endl;

    if (scan->get_tok() == T_IDENTIFIER) {
        return this->local_var[scan->get_value()];
    }

}*/

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


