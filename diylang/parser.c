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
    {"Ident", "IDENTIFIER", TokIDENT, false, false, false, -1, NodeIDENT},
    {"Integer literal", "INTEGER", TokINT, false, false, false, -1, NodeINT},
    {"String literal", "STRING", TokSTR, false, false, false, -1, NodeSTR}
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
    va_list ap;
    char buf[1000];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("(%d, %d) error: %s\n", err_line, err_col, buf);
    exit(1);
}

// ***
char *read_line(int *len){
    static char *text = NULL;
    static int textmax = 0;

    for(*len=0; ; (*len)++){
        int ch = fgetc(srcfile);
        if(ch == EOF || ch == '\n'){
            if(*len == 0){ return NULL;}
            break;
        }
        if(*len+1 >= textmax){
            textmax = (textmax == 0 ? 128: textmax * 2);
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

// *** get_enum() => get_enum_token()
TokenEnum_t get_enum_token(const char *name){
    for(size_t i=0; i < NELEMS(attr); i++){
        if(strcmp(attr[i].enumtxt, name) == 0){
            return attr[i].token;
        }
    }

    error(0, 0, "Unknown token %s\n", name);
    // return 0;
}

// *** gettok() => get_token()
Token_t get_token(){
    int len;
    Token_t token;
    char *yytext = read_line(&len);
    yytext = rtrim(yytext, &len);

    // []*{lineno}[]+{colno}[]+token[]+optional

    // get line and column
    token.error_line = atoi(strtok(yytext, " "));
    token.error_col = atoi(strtok(NULL, " "));

    // get the token name
    char *name = strtok(NULL, " ");
    token.token = get_enum_token(name);

    // if there is extran data, get it.
    char *p = name + strlen(name);
    if(p != &yytext[len]){
        for(++p; isspace(*p); ++p){}
        token.token = strdup(p);
    }
    return token;
}

// ***
Tree* make_node(NodeEnum_t nodetype, Tree* left, Tree* right){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = nodetype;
    tree->left = left;
    tree->right = right;
    return tree;
}

// ***
Tree* make_leaf(NodeEnum_t nodetype, char *value){
    Tree *tree = calloc(sizeof(*tree), 1);
    tree->node = nodetype;
    tree->value = strdup(value);
    return tree;
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
