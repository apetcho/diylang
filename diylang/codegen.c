#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>
#include<stdint.h>
#include<ctype.h>

typedef unsigned char uchar;

// ---
typedef enum {
    NodeIDENT, NodeSTR, NodeINT, NodeSEQ, NodeIF, NodePRTC, NodePRTS,
    NodePRTI, NodeWHILE, NodeASSIGN, NodeNEG, NodeNOT, NodeMUL, NodeDIV, 
    NodeMOD, NodeADD, NodeSUB, NodeLT, NodeLE, NodeGT, NodeGE, NodeEQ,
    NodeNE, NodeAND, NodeOR
} NodeEnum_t;

// ---
typedef enum{
    FETCH, STORE, PUSH, ADD, SUB, MUL, DIV, MOD, LT, GT, LE, GE, EQ, NE,
    AND, OR, NEG, NOT, JMP, JZ, PRTC, PRTS, PRTI, HALT
}CodeEnum_t;

typedef uchar Code_t;

// ---
typedef struct Tree{
    NodeEnum_t node;
    struct Tree *left;
    struct Tree *right;
    char *value;
}Tree;

// ***
#define diyl_dim(name, type)                                        \
    type *name = NULL;                                              \
    int _qy_ ## name ## _p = 0;                                     \
    int _qy_ ## name ## _max = 0

#define diyl_rewind(name)       _qy_ ## name ## _p = 0
#define diyl_redim(name)                                            \
    do{                                                             \
        if(_qy_ ## name ## _p >= _qy_ ## name ## _max){             \
            name = realloc(name, (_qy_##name##_max += 32)*sizeof(name[0])); \ 
        }                                                                   \
    }while(0)

#define diyl_append(name, x)                                        \
    do{                                                             \
        diyl_redim(name);                                           \
        name[_qy_ ## name ## _p++] = x;                             \
    }while(0)

#define diyl_len(name)  _qy_ ## name ## _p


#define diyl_add(name)                                              \
    do{                                                             \
        diyl_redim(name);                                           \
        _qy_ ## name ## _p++;                                       \
    }while(0)

// ---
FILE *srcfile;
FILE *dstfile;
static int here;
diyl_dim(object, Code_t);
diyl_dim(globals, const char*);
diyl_dim(string_pool, const char*);

// ---
struct {
    char *enumtxt;
    NodeEnum_t node;
    CodeEnum_t opcode;
} attr[] = {
    {"IDENTIFIER", NodeIDENT, -1},
    {"STRING", NodeSTR, -1},
    {"INTEGER", NodeINT, -1},
    {"SEQUENCE", NodeSEQ, -1},
    {"IF", NodeIF, -1},
    {"PRTC", NodePRTC, -1},
    {"PRTS", NodePRTS, -1},
    {"PRTI", NodePRTI, -1},
    {"WHILE", NodeWHILE, -1},
    {"ASSIGN", NodeASSIGN, -1},
    {"NEGATE", NodeNEG, NEG},
    {"NOT", NodeNOT, NOT},
    {"MULTIPLY", NodeMUL, MUL},
    {"MOD", NodeMOD, MOD},
    {"ADD", NodeADD, ADD},
    {"SUBSTRACT", NodeSUB, SUB},
    {"LESSTHAN", NodeLT, LT},
    {"LESSEQUAL", NodeLE, LE},
    {"GREATERTHAN", NodeGT, GT},
    {"GREATEREQUAL", NodeGE, GE},
    {"EQUAL", NodeEQ, EQ},
    {"NOTEQUAL", NodeNE, NE},
    {"AND", NodeAND, AND},
    {"OR", NodeOR, OR},
};

// ***
void error(const char *fmt, ...){
    // TODO
}

// ***
CodeEnum_t type_to_opcode(NodeEnum_t type){
    // TODO
}

// ***
Tree *make_node(NodeEnum_t ntype, Tree *left, Tree *right){
    // TODO
}

// ***
Tree *make_leaf(NodeEnum_t ntype, char *value){
    // TODO
}

// ***
// ----- Code Generator -----
void emit_byte(int c){
    diyl_append(object, (uchar)c);
    ++here;
}

// ***
void emit_int(int32_t n){
    // TODO
}

// ***
int hole(){
    // TODO
}

// ***
void fix(int src, int dst){
    // TODO
}

// ***
int fetch_var_offset(const char *id){
    // TODO
}

// ***
int fetch_string_offset(const char *st){
    // TODO
}

// ***
void code_gen(Tree *tree){
    // TODO
}

// ***
void code_finish(){
    // TODO
}

// ***
void list_code(){
    // TODO
}

// ***
// init_io() => initialize()
void initialize(FILE **fp, FILE *std, const char mode[], const char filename[]){
    // TODO
}

// ***
NodeEnum_t get_enum_value(const char name[]){
    // TODO
}

// ***
char* read_line(int *len){
    // TODO
}

// ***
char *rtrim(char *text, int *len){
    // TOD
}

// ***
Tree* load_ast(){
    // TODO
}


// --------------------------------------------------------------------------
//                          M A I N     D R I V E R
// --------------------------------------------------------------------------
int main(int argc, char **argv){
    initialize(&srcfile, stdin, "r", argc > 1 ? argv[1] : "");
    initialize(&dstfile, stdout, "wb", argc > 2 ? argv[2] : "");

    code_gen(load_ast());
    code_finish();
    list_code();

    return EXIT_SUCCESS;
}
