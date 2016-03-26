#ifndef __scan_file
#define __scan_file

#include <iostream>
#include <fstream>
#include <string>
#include "global.hpp"

using namespace std;

#define T_COMMA ','
#define T_SEMICOLON ';'
#define T_LPAREN '('
#define T_RPAREN ')'
#define T_LBRACKET '['
#define T_RBRACKET ']'
#define T_PERIOD '.'
//comparison
#define T_EQUALS 267

//operators
#define T_DIVIDE '/'
#define T_MULT '*'
#define T_SUM '+'
#define T_SUB '-'

// reserved words
#define TABLE_VALUE(x) x-256
#define SYMBOL_VALUE(x) x+256

//program structure
#define T_PROGRAM 256
#define T_IS 257
#define T_PROCEDURE 258
#define T_BEGIN 259
#define T_END 260
#define T_RETURN 261

//control
#define T_IF 262
#define T_THEN 263
#define T_ELSE 264
#define T_FOR 265
#define T_WHILE 266


// identifiers, constants, etc.
#define T_GLOBAL 267
#define T_NUMBER 268
#define T_INTEGER 269
#define T_FLOAT 270
#define T_STRING 271
#define T_IDENTIFIER 272
#define T_IN 273
#define T_OUT 274
#define T_INOUT 275
#define T_ASSIGN 276
#define T_TRUE 277
#define T_FALSE 278

#define F_PUTBOOL 279
#define F_PUTINTEGER 280
#define F_PUTFLOAT 281
#define F_PUTSTRING 282
#define F_PUTCHAR 283

#define T_EOF 349 // code used when at end of file
#define T_UNKNOWN 350 // token was unrecognized by scanner

#define error 666

#define MAX_RESERVED_KEYS 28



class Scan_file {
    std::string name;
    ifstream *myfile;
    int token;
    std::string value;

    std::string g_table[MAX_RESERVED_KEYS]; //symbol table

    int lookup(std::string);
    void create_table();
    void insert_reserved(char *,int);
    char remove_comment(char ch);

    public:
    Scan_file(std::string name);
    ~Scan_file();
    int scan_tok();
    int get_tok();
    std::string get_value();
    void display_table();
};


#endif //__scan_file
