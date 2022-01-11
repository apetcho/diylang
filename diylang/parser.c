#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>
#include<stdbool.h>
#include<ctype.h>

#define NELEMS(arr)     (sizeof(arr)/sizeof(arr[0]))

// TokenType => TokenEnum_t
typedef enum {
    TokEOI, TokMUL, TokDIV, TokMOD, TokADD, TokSUB, TokNEG,
    TokNOT, TokLT, TokLE, TokGT, TokGE, TokEQ, TokNE, TokASSIGN,
    TokAND, TokOR, TokIF,TokELSE, TokWHILE, TokPRINT, TokPUTC, TokLPAREN,
    TokRPAREN, TokLBRACE, TokRBRACE, TokSEMICOLON, TokCOMMA, TokIDENT,
    TokINT, TokSTR
} TokenEnum_t;

// ---
typedef enum {
    NodeIDENT, NodeSTR, NodeINT, NodeSEQ, NodeIF, NodePRTC, NodePRTS,
    NodePRTI, NodeWHILE, NodeASSIGN, NodeNEG, NodeNOT, NodeMUL, NodeDIV,
    NodeMOD, NodeADD, NodeSUB, NodeLT, NodeLE, NodeGT, NodeGE, NodeEQ,
    NodeNE, NodeAND, NodeOR
} NodeEnum_t;

// tok_s => Token_t
typedef struct {
    TokenEnum_t token;
    int error_line;
    int error_col;
    char *name;
    /* FIXME: Use this following fields instead? */
    //union{
    //    int value;      // value for constants
    //    char *name;     // identifier's name
    //};
} Token_t;

// ---
typedef struct Tree{
    NodeEnum_t node;
    struct Tree *left;
    struct Tree *right;
    char *value;
}Tree;


//! keep the following this way
// Because dependency
struct {
    char *text;
    char *enumtxt;
    TokenEnum_t token;
    bool rightassoc;
    bool isbinary;
    bool isunary;
    int predecence;
    NodeEnum_t node;
}attr[] = {
    {"EOI", "End_of_Input", TokEOI, false, false, false, -1, -1},
    {"*", "OP_multiply", TokMUL, false, true, false, 13, NodeMUL},
    {"/", "OP_divide", TokDIV, false, true, false, 13, NodeDIV},
    {"%", "OP_mod", TokMOD, false, true, false, 13, NodeMOD},
    {"+", "OP_add", TokADD, false, true, false, 12, NodeADD},
    {"-", "OP_substract", TokSUB, false, true, false, 12, NodeSUB},
    {"-", "OP_negate", TokNEG, false, false, true, 14, NodeNEG},
    {"!", "OP_not", TokNOT, false, false, true, 14, NodeNOT},
    {"<", "OP_less", TokLT, false, true, false, 10, NodeLT},
    {"<=", "OP_lessequal", TokLE, false, true, false, 10, NodeLE},
    {">", "OP_greater", TokGT, false, true, false, 10, NodeGT},
    {">=", "OP_greaterequal", TokGE, false, true, false, 10, NodeGE},
    {"==", "OP_equal", TokEQ, false, true, false, 9, NodeEQ},
    {"!=", "OP_notequal", TokNE, false, true, false, 9, NodeNE},
    {"=", "OP_assign", TokASSIGN, false, false, false, -1, NodeASSIGN},
    {"&&", "OP_and", TokAND, false, true, false, 5, NodeAND},
    {"||", "OP_or", TokOR, false, true, false, 4, NodeOR},
    {"if", "KWD_if", TokIF, false, false, false, -1, NodeIF},
    {"else", "KWD_else", TokELSE, false, false, false, -1, -1},
    {"while", "KWD_while", TokWHILE, false, false, false, -1, NodeWHILE},
    {"print", "KWD_print", TokPRINT, false, false, false, -1, -1},
    {"putc", "KWD_putc", TokPUTC, false, false, false, -1, -1},
    {"(", "LPAREN", TokLPAREN, false, false, false, -1, -1},
    {")", "RPAREN", TokRPAREN, false, false, false, -1, -1},
    {"{", "LBRACE", TokLBRACE, false, false, false, -1, -1},
    {"}", "RBRACE", TokRBRACE, false, false, false, -1, -1},
    {";", "SEMICOLON", TokSEMICOLON, false, false, false, -1, -1},
    {",", "COMMA", TokCOMMA, false, false, false, -1, -1},
    {"Ident", "Identifier", TokIDENT, false, false, false, -1, NodeIDENT},
    {"Integer literal", "Integer", TokINT, false, false, false, -1, NodeINT},
    {"String literal", "String", TokSTR, false, false, false, -1, NodeSTR}
};


// 
char *display_nodes[] = {
    "IDENTIFIER", "STRING", "INTEGER", "SEQUENCE", "IF", "PRTC", "PRTS",
    "PTRI", "WHILE", "ASSIGN", "NEGATE", "NOT", "MULTIPLY", "DIVIDE",
    "MOD", "ADD", "SUBSTRACT", "LESS", "LESSEQUAL", "GREATER",
    "GREATEREQUAL", "EQUAL", "NOTEQUAL", "AND", "OR"
};

// --
static Token_t token;
static FILE *srcfile;
static FILE *dstfile;

//
Tree* paren_expr();

// ***
void error(int err_line, int err_col, const char *fmt, ...){
    // TODO
}

// ***
char *read_line(int *len){
    // TODO
}

// ***
char* rtrim(char *text, int *len){
    // TODO
}

// *** get_enum() => get_enum_token()
TokenEnum_t get_enum_token(const char *name){
    // TODO
}

// *** gettok() => get_token()
Token_t get_token(){
    // TODO
}

// ***
Tree* make_node(NodeEnum_t nodetype, Tree* left, Tree* right){
    // TODO
}

// ***
Tree* make_leaf(NodeEnum_t nodetype, char *value){
    // TODO
}

// ***
void expect(const char msg[], TokenEnum_t symbol){
    // TODO
}


// ***
Tree *expr(int p){
    // TODO
}

Tree *paren_expr(){
    // TODO
}

// *** stmt() => statement()
Tree* statement(){
    // TODO
}

// ***
Tree* parse(){
    // TODO
}

// *** prt_ast() => print_ast()
void print_ast(Tree *tree){
    // TODO
}

// *** init_io() => initialize()
void initialize(FILE **fp, FILE *std, const char mode[], const char filename[]){
    // TODO
}


// ------------------------------------------------------------------------
//                      M A I N     D R I V E R
// ------------------------------------------------------------------------
int main(int argc, char **argv){
    initialize(&srcfile, stdin, "r", argc > 1 ? : "");
    initialize(&dstfile, stdout, "wb", argc > 2 ? argv[2]: "");
    print_ast(parse());

    return EXIT_SUCCESS;
}