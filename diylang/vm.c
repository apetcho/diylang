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
    FETCH, STORE, PUSH, ADD, SUB, MUL, DIV, MOD, LT, GT, LE, GE,
    EQ, NE, AND, OR, NEG, NOT, JMP, JZ, PRTC, PRTS, PRTI, HALT
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
    va_list ap;
    char buf[1000];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("Error: %s\n", buf);
    exit(1);
}

// -------------------------------------------
// ******* Virtual Machine Interpreter *******
// -------------------------------------------
void run_vm(const Code_t obj[], int32_t data[], int g_size, char **stringpool){
    int32_t *sp = &data[g_size+1];
    const Code_t *pc = obj;

again:
    switch(*pc++){
    case FETCH:
        *sp++ = data[*(int32_t*)pc];
        pc += sizeof(int32_t);
        goto again;
    case STORE:
        data[*(int32_t*)pc] = *--sp;
        pc += sizeof(int32_t);
        goto again;
    case PUSH:
        *sp++ = *(int32_t*)pc;
        pc += sizeof(int32_t);
        goto again;
    case ADD:
        sp[-2] += sp[-1];
        --sp;
        goto again;
    case SUB:
        sp[-2] -= sp[-1];
        --sp;
        goto again;
    case MUL:
        sp[-2] *= sp[-1];
        --sp;
        goto again;
    case MOD:
        sp[-2] /= sp[-1];
        --sp;
        goto again;
    case LT:
        sp[-2] = sp[-2] < sp[-1];
        --sp;
        goto again;
    case GT:
        sp[-2] = sp[-2] > sp[-1];
        --sp;
        goto again;
    case LE:
        sp[-2] = sp[-2] <= sp[-1];
        --sp;
        goto again;
    case GE:
        sp[-2] = sp[-2] >= sp[-1];
        --sp;
        goto again;
    case EQ:
        sp[-2] = sp[-2] == sp[-1];
        --sp;
        goto again;
    case NE:
        sp[-2] = sp[-2] != sp[-1];
        --sp;
        goto again;
    case AND:
        sp[-2] = sp[-2] && sp[-1];
        --sp;
        goto again;
    case OR:
        sp[-2] = sp[-2] || sp[-1];
        --sp;
        goto again;
    case NEG:
        sp[-1] = -sp[-1];
        goto again;
    case NOT:
        sp[-1] = !sp[-1];
        goto again;
    case JMP:
        pc += *(int32_t*)pc;
        goto again;
    case JZ:
        pc += (*--sp == 0) ? *(int32_t*)pc : (int32_t)(sizeof(int32_t));
        goto again;
    case PRTC:
        printf("%c", sp[-1]);
        --sp;
        goto again;
    case PRTS:
        printf("%s", stringpool[sp[-1]]);
        --sp;
        goto again;
    case PRTI:
        printf("%d", sp[-1]);
        --sp;
        goto again;
    case HALT:
        break;
    default:
        error("Unknown opcode %d\n", *(pc-1));
    }// switch
}

// ***
char* read_line(int *len){
    static char *text = NULL;
    static int textmax = 0;

    for(*len=0; ; (*len)++){
        int ch = fgetc(srcfile);
        if(ch==EOF || ch == '\n'){
            if(*len == 0){ return NULL; }
            break;
        }
        if((*len) + 1 >= textmax){
            textmax = (textmax == 0) ? 128 : textmax * 2;
            text = realloc(text, textmax);
        }
        text[(*len)] = ch;
    }
    text[(*len)] = '\0';
    return text;
}


// ***
char* rtrim(char *text, int *len){
    for(; *len > 0 && isspace(text[*len-1]); --(*len)){}
    text[*len] = '\0';
    return text;
}

// ***
char* translate(char *st){
    char *p, *q;
    if(st[0] == ""){++st;}
    p = q = st;

    while((*p++ = *q++) != '\0'){
        if(q[-1] == '\\'){
            if(q[0] == 'n'){
                p[-1] = '\0';
                ++q;
            }else if(q[0] == '\\'){ ++q; }
        }
        if(q[0] == "" && q[1] == '\0'){ ++q; }
    }

    return st;
}

// ***
int findit(const char text[], int offset){
    for(size_t i=0; i < sizeof(codemap)/sizeof(codemap[0]); i++){
        if(strcmp(codemap[i].text, text)==0){
            return codemap[i].opcode;
        }
    }
    error("Unknown instruction %s at %d\n", text, offset);
    return -1;
}

// ***
void emit_byte(int c){
    diyl_append(object, (uchar)c);
}

// ***
void emit_int(int32_t n){
    union {
        int32_t n;
        unsigned char c[sizeof(int32_t)];
    } x;
    x.n = n;
    for(size_t i=0; i < sizeof(x.n); ++i){
        emit_byte(x.c[i]);
    }
}

/*
Datasize: 5 Strings: 3
"is prime\n"
"Total primes found:"
"\n"
154 jmp (-73) 82
164 jz (32) 197
175 push 0
159 fetch [4]
149 store [3]
*/
// ***
char** load_code(int *ds){
    int line_len, nstr;
    char **stringpool;
    char *text = read_line(&line_len);
    text = rtrim(text, &line_len);

    strtok(text, " ");              // skip "Datasize:"
    *ds = atoi(strtok(NULL, " "));  // get actual data_size
    strtok(NULL, " ");              // skip "Strings:"
    nstr = atoi(strtok(NULL, " ")); // get number of strings

    stringpool = malloc(nstr * sizeof(char*));
    for(int i=0; i < nstr; ++i){
        text = read_line(&line_len);
        text = rtrim(text, &line_len);
        text = translate(text);
        stringpool[i] = strdup(text);
    }

    for(;;){
        int len;
        text = read_line(&line_len);
        if(text == NULL){ break; }
        text = rtrim(text, &line_len);
        int offset = atoi(strtok(text, " "));   // get the offset
        char *instr = strtok(NULL, " ");        // get the instruction
        int opcode = findit(instr, offset);
        emit_byte(opcode);
        char *operand = strtok(NULL, " ");

        switch(opcode){
        case JMP: case JZ:
            operand++;
            len = strlen(operand);
            operand[len-1] = '\0';
            emit_int(atoi(operand));
            break;
        case PUSH:
            emit_int(atoi(operand));
            break;
        case FETCH: case STORE:
            operand++;
            len = strlen(operand);
            operand[len-1] = '\0';
            emit_int(atoi(operand));
            break;
        }// switch
    }

    return stringpool;
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
