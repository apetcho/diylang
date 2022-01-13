#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include<ctype.h>
#include<string.h>
#include<errno.h>
#include<stdbool.h>
#include<limits.h>


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

#define diyl_len(name)  _qy_ ## name ## _p

// TokenType => TokenEnum_t
typedef enum {
    TokEOI, TokMUL, TokDIV, TokMOD, TokADD, TokSUB, TokNEG,
    TokNOT, TokLT, TokLE, TokGT, TokGE, TokEQ, TokNE, TokASSIGN,
    TokAND, TokOR, TokIF,TokELSE, TokWHILE, TokPRINT, TokPUTC, TokLPAREN,
    TokRPAREN, TokLBRACE, TokRBRACE, TokSEMICOLON, TokCOMMA, TokIDENT,
    TokINT, TokSTR
} TokenEnum_t;

// tok_s => Token_t
typedef struct {
    TokenEnum_t token;
    int error_line;
    int error_col;
    union{
        int value;      // value for constants
        char *name;     // identifier's name
    };
} Token_t;


//
static FILE *srcfile;
static FILE *dstfile;
static int lineno = 1;
static int column = 0;
static int current_char = ' ';
diyl_dim(text, char);

// gettok() => get_token()
Token_t get_token();

// ****
static void error(int err_line, int err_col, const char *fmt, ...){
    char buf[1000];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printf("(%d, %d) \x1[31merror\x1b[0m: %s\n", err_line, err_col, buf);
    exit(EXIT_FAILURE);
}

// *** Get next char from inpput
// next_ch() => next_char()
static int next_char(void){
    current_char = get(srcfile);
    ++column;
    if(current_char == '\n'){
        ++lineno;
        column = 0;
    }
    return current_char;
}

// ****
// char_lit() => char_literal()
static Token_t char_literal(int n, int err_line, int err_col){
    if(current_char == '\''){
        error(err_line, err_col, "char_literal(): empty character constant");
    }
    if(current_char == '\\'){
        next_char();
        if(current_char == 'n'){
            n = 10;
        }else if(current_char == '\\'){
            n = '\\';
        }else{
            error(
                err_line, err_col,
                "char_literal(): unknown escape sequence\\%c", current_char
            );
        }
    }

    if(next_char() != '\''){
        error(err_line, err_col, "multi-character constant");
    }
    next_char();
    Token_t retval;
    retval.token = TokINT;
    retval.error_col = err_col;
    retval.error_line = err_line;
    retval.value = n;
    return retval;
}

// ****
// Process divide or comments
// div_or_cmt() => divide_or_comment()
static Token_t divide_or_comment(int err_line, int err_col){
    Token_t retval;
    if(current_char != '*'){
        retval.token = TokDIV;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        return retval;
    }
    // comment found
    next_char();
    for(;;){
        if(current_char == '*'){
            if(next_char() == '/'){
                next_char();
                return get_token();
            }
        }else if(current_char == EOF){
            error(err_line, err_col, "EOF in comment");
        }else{
            next_char();
        }
    }
}

// ****
// string_lit() => string_literal()
static Token_t string_literal(int start, int err_line, int err_col){
    // TODO
}

// ****
// kwd_cmp() => keyword_compare()
static int keyword_compare(const void *ptrs, const void *ptr){
    // TODO
}

// ****
// get_ident_type() => get_identifier_type()
static Token_t get_identifier_type(const char *identifier){
    // TODO
}

// ****
// ident_or_int() => identifier_or_int()
static Token_t identifier_or_int(int err_line, int err_col){
    // TODO
}


// **** look ahead for ">=", etc. ****
// follow()
static Token_t follow(
    int expect, TokenEnum_t ifyes, TokenEnum_t ifno,
    int err_line, int err_col
){
    // TODO
}

// gettok() => get_token()
Token_t get_token(void){
    // TODO
}

// ****
// run() => tokenize()
void tokenize(void){
    // TODO
}

// *****
// init_io() => initialze()
void initialize(FILE **fp, FILE *std, const char mode[], const char filename[]){
    // TODO
}


// ----------------------------------------------------------------------
//                          M A I N   D R I V E R
// ----------------------------------------------------------------------
int main(int argc, char **argv){
    initialize(&srcfile, stdin, "r", argc > 1 ? argv[1]: "");
    initialize(&dstfile, stdout, "wb", argc > 2 ? argv[2]: "");
    tokenize();

    return EXIT_SUCCESS;
}
