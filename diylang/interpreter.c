#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdarg.h>
#include<ctype.h>

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


// ****
typedef enum {
    NodeIDENT, NodeSTR, NodeINT, NodeSEQ, NodeIF, NodePRTC, NodePRTS,
    NodePRTI, NodeWHILE, NodeASSIGN, NodeNEG, NodeNOT, NodeMUL, NodeDIV,
    NodeMOD, NodeADD, NodeSUB, NodeLT, NodeLE, NodeGT, NodeGE, NodeEQ,
    NodeNE, NodeAND, NodeOR
} NodeEnum_t;

// ---
typedef struct Tree Tree;
struct Tree {
    NodeEnum_t node;
    struct Tree *left;
    struct Tree *right;
    int value;
};

// ***
struct {
    char *enumtxt;
    NodeEnum_t type;
} attr[] = {
    {"IDENTIFIER", NodeIDENT},
    {"STRING", NodeSTR},
    {"INTEGER", NodeINT},
    {"SEQUENCE", NodeSEQ},
    {"IF", NodeIF},
    {"PRTC", NodePRTC},
    {"PRTS", NodePRTS},
    {"PRTI", NodePRTI},
    {"WHILE", NodeWHILE},
    {"ASSIGN", NodeASSIGN},
    {"NEGATE", NodeNEG},
    {"NOT", NodeNOT},
    {"MULTIPLY", NodeMUL},
    {"DIVIDE", NodeDIV},
    {"MOD", NodeMOD},
    {"ADD", NodeADD},
    {"SUBSTRACT", NodeSUB},
    {"LESSTHAN", NodeLT},
    {"LESSEQUAL", NodeLE},
    {"GREATERTHAN", NodeGT},
    {"GREATEREQUAL", NodeGE},
    {"EQUAL", NodeEQ},
    {"NOTEQUAL", NodeNE},
    {"AND", NodeAND},
    {"OR", NodeOR},
};

// ---
FILE *srcfile;
diyl_dim(stringpool, const char*);
diyl_dim(global_names, const char*);
diyl_dim(global_values, int);


// ***
void error(const char *fmt, ...){
    va_list ap;
    char buf[1000];
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    printf("Error: %s\n", buf);
    exit(1);
}

// ***
Tree* make_node(NodeEnum_t ntype, Tree* left, Tree* right){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = ntype;
    tree->left = left;
    tree->right = right;
    return tree;
}

// ***
Tree* make_leaf(NodeEnum_t ntype, int value){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = ntype;
    tree->value = value;
    return tree;
}

// ***
int interpret(Tree *tree){
    if(!tree){return 0;}
    switch(tree->node){
    case NodeINT:
        return tree->value;
    }
}


// ***
// init_in() => initialize()
void initialize(const char filename[]){
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
char* rtrim(char *text, int *len){
    // TODO
}

// ***
int fetch_string_offset(char *s){
    // TODO
}

// ***
int fetch_var_offset(const char *name){
    // TODO
}

// ***
Tree* load_ast(){
    // TODO
}

// ------------------------------------------------------------------------
//                          M A I N   D R I V E R
// ------------------------------------------------------------------------
int main(int argc, char **argv){
    initialize(argc > 1 ? argv[1] : "");
    Tree* ast = load_ast();
    interpret(ast);

    return EXIT_SUCCESS;
}
