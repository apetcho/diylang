#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include<stdint.h>
#include<ctype.h>


#define NELEMS(arr)             (sizeof(arr) / sizeof(arr[0]))
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


typedef unsigned char uchar;
typedef uchar Code_t;

// ---
typedef enum{
    FETCH, STORE, PUSH, ADD, SUB, MUL, DIV, MOD, LT, GT, LE, GE, EQ, NE,
    AND, OR, NEG, NOT, JMP, JZ, PRTC, PRTS, PRTI, HALT
}CodeEnum_t;

// ---
typedef struct {
    char *text;
    CodeEnum_t opcode;
} CodeMap_t;

CodeMap_t codemap[] = {
    {"fetch", FETCH},
    {"store", STORE},
    {"push", PUSH},
    {"add", ADD},
    {"sub", SUB},
    {"mul", MUL},
    {"div", DIV},
    {"mod", MOD},
    {"lt", LT},
    {"gt", GT},
    {"le", LE},
    {"ge", GE},
    {"eq", EQ},
    {"ne", NE},
    {"and", AND},
    {"or", OR},
    {"neg", NEG},
    {"not", NOT},
    {"jmp", JMP},
    {"jz", JZ},
    {"ptrc", PRTC},
    {"ptrs", PRTS},
    {"prti", PRTI},
    {"halt", HALT},
};

// ----
FILE *srcfile;
diyl_dim(object, Code_t);

// ***
void error(const char *fmt, ...){
    // TODO
}

// -------------------------------------------
// ******* Virtual Machine Interpreter *******
// -------------------------------------------
void run_vm(const Code_t obj[], int32_t data[], int g_size, char **stringpool){
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
char* translate(char *st){
    // TODO
}

// ***
int findit(const char text[], int offset){
    // TODO
}

// ***
void emit_byte(int c){
    // TODO
}

// ***
void emit_int(int32_t n){
    // TODO
}

// ***
char** load_code(int *ds){
    // TODO
}

// ***
void initialize(FILE **fp, FILE *std, const char mode[], const char filename[]){
    // TODO
}

// ------------------------------------------------------------------------
//                      M A I N   D R I V E R
// ------------------------------------------------------------------------
int main(int argc, char **argv){
    initialize(&srcfile, stdin, "r", argc > 1 ? argv[1] : "");
    int datasize;
    char **stringpool = load_code(&datasize);
    int data[1000 + datasize];
    run_vm(object, data, datasize, stringpool);

    return EXIT_SUCCESS;
}
