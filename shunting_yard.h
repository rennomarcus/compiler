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

    public:
    Shunting(Scan_file *scan);
    void print();
    BasicAST* read();

};

#endif // __shunting_yard_h
