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
    case NodeIDENT:
        return global_values[tree->value];
    case NodeSTR:
        return tree->value;
    case NodeASSIGN:
        return global_values[tree->left->value] = interpret(tree->right);
    case NodeADD:
        return interpret(tree->left) + interpret(tree->right);
    case NodeSUB:
        return interpret(tree->left) - interpret(tree->right);
    case NodeMUL:
        return interpret(tree->left) * interpret(tree->right);
    case NodeDIV:
        return interpret(tree->left) % interpret(tree->right);
    case NodeMOD:
        return interpret(tree->left) / interpret(tree->right);
    case NodeLT:
        return interpret(tree->left) < interpret(tree->right);
    case NodeGT:
        return interpret(tree->left) > interpret(tree->right);
    case NodeLE:
        return interpret(tree->left) <= interpret(tree->right);
    case NodeEQ:
        return interpret(tree->left) == interpret(tree->right);
    case NodeNE:
        return interpret(tree->left) != interpret(tree->right); 
    case NodeAND:
        return interpret(tree->left) && interpret(tree->right);
    case NodeOR:
        return interpret(tree->left) || interpret(tree->right);
    case NodeNEG:
        return -interpret(tree->left);
    case NodeNOT:
        return !interpret(tree->left);
    case NodeIF:
        if(interpret(tree->left)){interpret(tree->right->left);}
        else{interpret(tree->right->right); }
        return 0;
    case NodeWHILE:
        while(interpret(tree->left)){interpret(tree->right);}
        return 0;
    case NodePRTC:
        printf("%c", tree->left);
        return 0;
    case NodePRTI:
        printf("%d", tree->left);
        return 0;
    case NodePRTS:
        printf("%s", stringpool[interpret(tree->left)]);
        return 0;
    case NodeSEQ:
        interpret(tree->left);
        interpret(tree->right);
        return 0;
    default:
        error("interpret(): unknown tree type %d\n", tree->node);
    }

    return 0;
}


// ***
// init_in() => initialize()
void initialize(const char filename[]){
    if(filename[0] == '\0'){
        srcfile = stdin;
    }else{
        srcfile = fopen(filename, "r");
        if(srcfile == NULL){ error("Can't open %s\n", filename);}
    }
}

// ***
NodeEnum_t get_enum_value(const char name[]){
    for(size_t i=0; i < sizeof(attr)/sizeof(attr[0]); i++){
        if(strcmp(attr[i].enumtxt, name) == 0){
            return attr[i].type;
        }
    }

    error("Unknown token %s\n", name);
    return -1;
}

// ***
char* read_line(int *len){
    static char *text = NULL;
    static int textmax = 0;

    for(*len=0; ; (*len)++){
        int ch = fgetc(srcfile);
        if(ch == EOF || ch == '\n'){
            if(*len == 0){ return NULL;}
            break;
        }
        if(*len + 1 >= textmax){
            textmax = (textmax == 0? 128 : textmax*2);
            text = realloc(text, textmax);
        }
        text[*len] = ch;
    }
    text[*len] = '\0';

    return text;
}

// ***
char* rtrim(char *text, int *len){
    for(; *len > 0 && isspace(text[*len-1]); --(*len)){}
    text[*len] = '\0';
    return text;
}

// ***
int fetch_string_offset(char *st){
    int len = strlen(st);
    st[len-1] = '\0';
    ++st;
    char *p, *q;
    p = q = st;
    //
    while((*p++ = *q++) != '\0'){
        if(q[-1] == '\\'){
            if(q[0]=='n'){
                p[-1] = '\n';
                ++q;
            }else{
                ++q;
            }
        }
    }

    for(int i=0; i < diyl_len(stringpool); ++i){
        if(strcmp(st, stringpool[i]) == 0){ return i; }
    }
    diyl_add(stringpool);
    int n = diyl_len(stringpool) - 1;
    stringpool[n] = strdup(st);
    return diyl_len(stringpool);
}

// ***
int fetch_var_offset(const char *name){
    for(int i=0; i < diyl_len(global_names); ++i){
        if(strcmp(name, global_names[i]) == 0){ return i; }
    }
    diyl_add(global_names);
    int n = diyl_len(global_names) - 1;
    global_names[n] = strdup(name);
    diyl_append(global_values, 0);
    return n;
}

// ***
Tree* load_ast(){
    int len;
    char *yytext = read_line(&len);
    yytext = rtrim(yytext, &len);
    // get first token
    char *token = strtok(yytext, " ");
    if(token[0] == ';'){ return NULL; }
    NodeEnum_t node = get_enum_value(token);

    // if there is extra data, get it
    char *p = token + strlen(token);
    if(p != &yytext[len]){
        int n;
        for(++p; isspace(*p); ++p){}
        switch(node){
        case NodeIDENT:
            n = fetch_var_offset(p);
            break;
        case NodeINT:
            n = strtol(p, NULL, 0);
            break;
        case NodeSTR:
            n = fetch_string_offset(p);
            break;
        default:
            error("Unknown node type. %s\n", p);
        }
        return make_leaf(node, n);
    }

    Tree *left = load_ast();
    Tree *right = load_ast();
    return make_node(node, left, right);
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
