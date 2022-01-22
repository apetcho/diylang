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
    FETCH, STORE, PUSH, ADD, SUB, MUL, DIV, MOD, LT, GT, LE, GE,
    EQ, NE, AND, OR, NEG, NOT, JMP, JZ, PRTC, PRTS, PRTI, HALT
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
    va_list ap;
    char buf[1000];
    //
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("Error: %s\n", buf);
    exit(1);
}

// ***
CodeEnum_t type_to_opcode(NodeEnum_t type){
    return attr[type].opcode;
}

// ***
Tree *make_node(NodeEnum_t ntype, Tree *left, Tree *right){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = ntype;
    tree->left = left;
    tree->right = right;
    return tree;
}

// ***
Tree *make_leaf(NodeEnum_t ntype, char *value){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = ntype;
    tree->value = value;
    return tree;
}

// ***
// ----- Code Generator -----
void emit_byte(int c){
    diyl_append(object, (uchar)c);
    ++here;
}

// ***
void emit_int(int32_t n){
    union{
        int32_t n;
        unsigned char c[sizeof(int32_t)];
    }x;
    x.n = n;
    for(size_t i=0; i < sizeof(x.n); ++i){emit_byte(x.c[i]);}
}

// ***
int hole(){
    int t = here;
    emit_int(0);
    return t;
}

// ***
void fix(int src, int dst){
    *(int32_t*)(object + src) = dst - src;
}

// ***
int fetch_var_offset(const char *id){
    for(int i=0; i < diyl_len(globals); ++i){
        if(strcmp(id, globals[i]) == 0){ return i;}
    }

    diyl_add(globals);
    int n = n = diyl_len(globals) - 1;
    globals[n] = strdup(id);
    return n;
}

// ***
int fetch_string_offset(const char *st){
    for(int i=0; i < diyl_len(string_pool); i++){
        if(strcmp(st, string_pool[i]) == 0){ return i; }
    }
    diyl_add(string_pool);
    int n = diyl_len(string_pool) - 1;
    string_pool[n] = strdup(st);
    return n;
}

// ***
void code_gen(Tree *tree){
    int p1, p2, n;

    if(tree == NULL){ return;}
    switch(tree->node){
    case NodeIDENT:
        emit_byte(FETCH);
        n = fetch_var_offset(tree->value);
        emit_int(n);
        break;
    case NodeINT:
        emit_byte(PUSH);
        emit_int(atoi(tree->value));
        break;
    case NodeSTR:
        emit_byte((PUSH));
        n = fetch_string_offset(tree->value);
        emit_int(n);
        break;
    case NodeASSIGN:
        n = fetch_var_offset(tree->left->value);
        code_gen(tree->right);
        emit_byte(STORE);
        emit_int(n);
        break;
    case NodeIF:
        code_gen(tree->left);
        emit_byte(JZ);
        p1 = hole();
        code_gen(tree->right->left);
        if(tree->right->right != NULL){
            emit_byte(JMP);
            p2 = hole();
        }
        fix(p1, here);
        if(tree->right->right != NULL){
            code_gen(tree->right->right);
            fix(p2, here);
        }
        break;
    case NodeWHILE:
        p1 = here;
        code_gen(tree->left);
        emit_byte(JZ);
        p2 = hole();
        code_gen(tree->right);
        emit_byte(JMP);
        fix(hole(), p1);
        fix(p2, here);
        break;
    case NodeSEQ:
        code_gen(tree->left);
        code_gen(tree->right);
        break;
    case NodePRTC:
        code_gen(tree->left);
        emit_byte(PRTC);
        break;
    case NodePRTI:
        code_gen(tree->left);
        emit_byte(PRTI);
        break;
    case NodePRTS:
        code_gen(tree->left);
        emit_byte(PRTS);
        break;
    case NodeLT: case NodeGT: case NodeLE: case NodeEQ: case NodeNE:
    case NodeAND: case NodeOR: case NodeSUB: case NodeADD: case NodeDIV:
    case NodeMUL: case NodeMOD:
        code_gen(tree->left);
        code_gen(tree->right);
        emit_byte(type_to_opcode(tree->node));
        break;
    case NodeNEG: case NodeNOT:
        code_gen(tree->left);
        emit_byte(type_to_opcode(tree->node));
        break;
    default:
        error("Error in code generator - found %d, expecting operator",
            tree->node);
    }
}

// ***
void code_finish(){
    emit_byte(HALT);
}

// ***
void list_code(){
    fprintf(
        dstfile,
        "Datasize: %d strings: %d\n",
        diyl_len(globals), diyl_len(string_pool)
    );
    for(int i=0; i < diyl_len(string_pool); ++i){
        fprintf(dstfile, "%s\n", string_pool[i]);
    }

    Code_t *pc = object;
    // --
again:
    fprintf(dstfile, "%5d ", (int)(pc - object));
    switch(*pc++){
    case FETCH:
        fprintf(dstfile, "fetch [%d]\n", *(int32_t*)pc);
        pc += sizeof(int32_t);
        goto again;
    case STORE:
        fprintf(dstfile, "store [%d]\n", *(int32_t*)pc);
        pc += sizeof(int32_t);
        goto again;
    case PUSH:
        fprintf(dstfile, "push %d\n", *(int32_t*)pc);
        pc += sizeof(int32_t);
        goto again;
    case ADD:
        fprintf(dstfile, "add\n");
        goto again;
    case SUB:
        fprintf(dstfile, "sub\n");
        goto again;
    case MUL:
        fprintf(dstfile, "mul\n");
        goto again;
    case DIV:
        fprintf(dstfile, "div\n");
        goto again;
    case MOD:
        fprintf(dstfile, "mod\n");
        goto again;
    case LT:
        fprintf(dstfile, "lt\n");
        goto again;
    case GT:
        fprintf(dstfile, "gt\n");
        goto again;
    case LE:
        fprintf(dstfile, "le\n");
        goto again;
    case GE:
        fprintf(dstfile, "ge\n");
        goto again;
    case EQ:
        fprintf(dstfile, "eq\n");
        goto again;
    case AND:
        fprintf(dstfile, "and\n");
        goto again;
    case OR:
        fprintf(dstfile, "or\n");
        goto again;
    case NOT:
        fprintf(dstfile, "not\n");
        goto again;
    case NEG:
        fprintf(dstfile, "neg\n");
        goto again;
    case JMP:
        fprintf(dstfile, "jmp\n");
        goto again;
    }//switch
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
