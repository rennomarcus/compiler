#ifndef __shunting_yard_h
#define __shunting_yard_h

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "global.hpp"
#include "scan.hpp"
#include "ast.hpp"
struct token_struct {
    int token_type;
    std::string value;

} typedef token;
class Shunting {
    std::vector<token> equation;
    std::vector<char> operators;
    int parenthesis;

    public:
    void inc_parenthesis() { parenthesis++; }
    void dec_parenthesis() { parenthesis--; }
    void set_parenthesis(int value) { parenthesis = value; }
    int get_parenthesis() { return parenthesis; }
    Shunting(Scan_file *scan);
    void gen_equation(Scan_file *scan);
    void print();
    std::vector<BasicAST*> read();

};

#endif // __shunting_yard_h
