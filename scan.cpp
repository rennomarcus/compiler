#include "scan.hpp"

Scan_file::Scan_file(std::string name) {
    this->name = name;
    ifstream *tmp_file = new ifstream(name);

    myfile = tmp_file;
    create_table();
}
Scan_file::~Scan_file(void) {
    Debug("closing file");
    myfile->close();
}

int Scan_file::get_tok() {
    return this->token;
}

std::string Scan_file::get_value() {
    return this->value;
}

int Scan_file::get_previous_tok() {
    return previous.token;
}
std::string Scan_file::get_previous_value() {
    return previous.value;
}
void Scan_file::assign_attributes(int t, std::string v) {
    this->token = t;
    this->value = v;
}

char Scan_file::remove_comment(char ch) {
    if (ch == '/') {
        (*myfile).get(ch);
        if (ch == '/') {
            while (ch != '\n') {
                (*myfile).get(ch);
                if (myfile->eof()) {
                    return '\0';
                }
            }
        }
        if (ch == '*') {
            bool leave = false;
            while (!leave) {
                (*myfile).get(ch);
                while (ch != '*') {
                    (*myfile).get(ch);
                    if (myfile->eof()) {
                        return '\0';
                    }
                }
                (*myfile).get(ch);
                if (ch == '/')
                    leave = true;
                if (myfile->eof()) {
                    return '\0';
                }
            }
            (*myfile).get(ch);
        }
    }
    while (isspace(ch)) {
        (*myfile).get(ch);
    }
    return ch;
}
int Scan_file::scan_tok() {
    char ch;
    std::string IdentifierStr;
    previous.token = get_tok();
    previous.value = get_value();
    if (myfile->is_open())
    {

        this->token = T_UNKNOWN;
        (*myfile).get(ch);


        if (myfile->eof()) {
            assign_attributes(T_EOF, "");
            return T_EOF;
        }
        while (isspace(ch)) {
            if (ch == '\n') {
                g_line++;
            }
            (*myfile).get(ch);
            if (myfile->eof()) {
                assign_attributes(T_EOF, "");
                return T_EOF;
            }
        }
        ch = remove_comment(ch);
        if (ch == '\0') {
            assign_attributes(T_EOF, "\0");
            return T_EOF;
        }
        if (ch == '.') {
            assign_attributes(T_PERIOD, ".");
            return T_PERIOD;
        }
        if (ch == '-') {
            (*myfile).get(ch);
            if (isdigit(ch)) {
                (*myfile).putback(ch);
                if (previous.token == T_INTEGER || previous.token == T_FLOAT || previous.token == T_IDENTIFIER) {
                    ch = '-';
                    this->token = ch;
                    assign_attributes(ch, "" + ch);
                    return ch;
                }
                ch = '-';
            }
            else {
                (*myfile).putback(ch);
                ch = '-';
                this->token = ch;
                assign_attributes(ch, "" + ch);
                return ch;
            }
        }
        if (ch == T_LBRACKET && previous.token == T_IDENTIFIER) {
            (*myfile).get(ch);
            if (isdigit(ch)) {
                while (isdigit(ch)) {
                    IdentifierStr += ch;
                    (*myfile).get(ch);
                }
                assign_attributes(T_ARRAY, IdentifierStr.c_str());
                return T_ARRAY;
            } else { Error("Invalid array"); }
        }
        if (ch == '"') {
            std::string message = "";
            (*myfile).get(ch);
            while(ch != '"') {
                message += ch;
                (*myfile).get(ch);
            }
            assign_attributes(T_STRING_MESSAGE, message);
            return T_STRING_MESSAGE;
        }
        if (isalpha(ch)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
            IdentifierStr = tolower(ch);
            while (!isspace(ch)) {
                (*myfile).get(ch);
                if (isalnum(ch) || ch == '_')
                    IdentifierStr += tolower(ch);
                else {
                    (*myfile).putback(ch);
                    break;
                }
            }
            assign_attributes(lookup(IdentifierStr), IdentifierStr);
            return this->token;
        }

        if (isdigit(ch) || ch == '.' || ch == '-') {   // Number: [0-9.]+
            std::string NumStr;
            std::string period = ".";
            do {
                NumStr += ch;
                (*myfile).get(ch);
            } while (isdigit(ch) || ch == '.');

            std::size_t found = NumStr.find(period);
            if (found!=std::string::npos){
                double value = strtod(NumStr.c_str(), 0);
                assign_attributes(T_FLOAT, NumStr.c_str());
                return T_FLOAT;
            }
            else {
                (*myfile).putback(ch);
            }
            assign_attributes(T_INTEGER, NumStr.c_str());
            return T_INTEGER;
        }

        if (ch == EOF)
            return T_EOF;

        if (ch == ':') {
            (*myfile).get(ch);
            if (ch != '=')
                (*myfile).putback(ch);
            else {
                assign_attributes(T_ASSIGN, "=");
                return T_ASSIGN;
            }
        }
        this->token = ch;
        assign_attributes(ch, "" + ch);
        return ch;

    }
    else{
        cout << "error?";
            return T_EOF;
    }

    return T_EOF;
}
void Scan_file::insert_reserved(char *t_value,int t_type) {
    this->g_table[t_type] = new char[strlen(t_value)];
    this->g_table[t_type] = t_value;
}
void Scan_file::create_table() {
    insert_reserved("while", TABLE_VALUE(T_WHILE));
    insert_reserved("if", TABLE_VALUE(T_IF));
    insert_reserved("then", TABLE_VALUE(T_THEN));
    insert_reserved("else", TABLE_VALUE(T_ELSE));
    insert_reserved("for", TABLE_VALUE(T_FOR));
    insert_reserved("return", TABLE_VALUE(T_RETURN));
    insert_reserved("program", TABLE_VALUE(T_PROGRAM));
    insert_reserved("is", TABLE_VALUE(T_IS));
    insert_reserved("procedure", TABLE_VALUE(T_PROCEDURE));
    insert_reserved("global", TABLE_VALUE(T_GLOBAL));
    insert_reserved("begin", TABLE_VALUE(T_BEGIN));
    insert_reserved("end", TABLE_VALUE(T_END));
    insert_reserved("number", TABLE_VALUE(T_NUMBER));
    insert_reserved("integer", TABLE_VALUE(T_INTEGER));
    insert_reserved("float", TABLE_VALUE(T_FLOAT));
    insert_reserved("string", TABLE_VALUE(T_STRING));
    //272 = IDENTIFIER
    insert_reserved("in", TABLE_VALUE(T_IN));
    insert_reserved("out", TABLE_VALUE(T_OUT));
    insert_reserved("inout", TABLE_VALUE(T_INOUT));
    insert_reserved("true", TABLE_VALUE(T_TRUE));
    insert_reserved("false", TABLE_VALUE(T_FALSE));
    //external functions
    insert_reserved("putbool", TABLE_VALUE(F_PUTBOOL));
    insert_reserved("putinteger", TABLE_VALUE(F_PUTINTEGER));
    insert_reserved("putfloat", TABLE_VALUE(F_PUTFLOAT));
    insert_reserved("putstring", TABLE_VALUE(F_PUTSTRING));
    insert_reserved("putchar", TABLE_VALUE(F_PUTCHAR));
    insert_reserved("getbool", TABLE_VALUE(F_GETBOOL));
    insert_reserved("getinteger", TABLE_VALUE(F_GETINTEGER));
    insert_reserved("getfloat", TABLE_VALUE(F_GETFLOAT));
    insert_reserved("getstring", TABLE_VALUE(F_GETSTRING));
    insert_reserved("getchar", TABLE_VALUE(F_GETCHAR));




}

void Scan_file::display_table() {
    int i;
    std::string val = "procedure";
    for (i = 0; i < MAX_RESERVED_KEYS; i++) {
        cout << "#" << i << " " << g_table[i] << endl;
    }
}

int Scan_file::lookup(std::string value) {
    int i;

    for (i = 0; i < MAX_RESERVED_KEYS; i++) {
        if (value.compare(this->g_table[i]) == 0)
            return SYMBOL_VALUE(i);
    }
    return T_IDENTIFIER;
}


