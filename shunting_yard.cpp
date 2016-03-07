#include "shunting_yard.h"


Shunting::Shunting(Scan_file *scan) {
    int tok;
    while ((tok = scan->scan_tok()) != T_SEMICOLON) {
        int tok = scan->get_tok();
        token t;
        t.token_type = tok;
        t.value = scan->get_value();
        //If the incoming symbols is an operand, print it..
        if (tok == T_INTEGER || tok == T_FLOAT || tok == T_IDENTIFIER) {
            this->equation.push_back(t);
        }
        else {
            //If the incoming symbol is a left parenthesis, push it on the stack.
            if (tok == T_LPAREN) {
                this->equation.push_back(t);
            }
            //If the incoming symbol is a right parenthesis:
            //discard the right parenthesis, pop and print the stack symbols until you see a left parenthesis.
            //Pop the left parenthesis and discard it.
            else if (tok == T_RPAREN) {
            }
            else if (this->operators.empty()) {
                //If the incoming symbol is an operator and the stack is empty or contains a left parenthesis on top, push the incoming operator onto the stack.
                this->operators.push_back(tok);
            }

            //If the incoming symbol is an operator and has either higher precedence than the operator on the top of the stack,
            //or has the same precedence as the operator on the top of the stack and is right associative -- push it on the stack.
            else {
                int current = BinopPrecedence[this->operators.back()];
                int scanned = BinopPrecedence[tok];
                if (scanned > current) {
                    this->operators.push_back(tok);
                }
                //If the incoming symbol is an operator and has either lower precedence than the operator on the top of the stack,
                //or has the same precedence as the operator on the top of the stack and is left associative
                // -- continue to pop the stack until this is not true. Then, push the incoming operator.
                else {
                    while (!this->operators.empty() && scanned <= current) {
                        token temp;
                        temp.token_type = this->operators.back();
                        this->operators.pop_back();
                        this->equation.push_back(temp);
                        if (!this->operators.empty()) {
                            current = BinopPrecedence[this->operators.back()];
                        }
                    }
                    this->operators.push_back(tok);
                }
            }
        }
    }
    while (!this->operators.empty()) {
        token temp;
        temp.token_type = this->operators.back();
        this->operators.pop_back();
        this->equation.push_back(temp);
    }
}


//Debug function
void Shunting::print() {
    token t;
    std::cout << "*************PRINTING ";
    for (auto it = equation.begin(); it != equation.end(); it++) {
        t = *it;
        if (t.token_type == T_INTEGER || t.token_type == T_FLOAT || t.token_type == T_IDENTIFIER) {
            std::cout << t.value;
        }
        else {
            std::cout << (char)t.token_type;
        }
    }
    std::cout << std::endl;
}

BasicAST* Shunting::read() {
    std::vector<BasicAST*> eq;
    token t;
    BasicAST* temp;
    if (equation.size() > 1) {
        for (auto it = equation.begin(); it != equation.end(); it++) {
            t = *it;
            if (t.token_type == T_INTEGER) {
                temp = new IntegerAST(std::stoi(t.value));
                eq.push_back(temp);
            }
            else if (t.token_type == T_IDENTIFIER) {
                temp = new CallVarAST(t.value);
                eq.push_back(temp);
            }
            else if (t.token_type == T_FLOAT) {
                temp = new FloatAST(std::stoi(t.value));
                eq.push_back(temp);
            }
            else {
                BasicAST *op1 = eq.back();
                eq.pop_back();
                BasicAST *op2 = eq.back();
                eq.pop_back();
                std::cout << "#create bin op" << (char)t.token_type << std::endl;
                temp = new BinopAST((char)t.token_type, op1,op2);
                eq.push_back(temp);
            }
        }
        return eq.back();
    }
    else {
        t = equation.back();
        if (t.token_type == T_INTEGER) {
            temp = new IntegerAST(std::stoi(t.value));
        }
        else if (t.token_type == T_IDENTIFIER) {
            temp = new CallVarAST(t.value);
        }
        else if (t.token_type == T_FLOAT) {
            temp = new FloatAST(std::stoi(t.value));
        }
        return temp;
    }
}
