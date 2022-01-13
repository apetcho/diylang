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
    diyl_rewind(text);

    while(next_char() != start){
        if(current_char == '\n'){
            error(err_line, err_col, "EOL in string");
        }
        if(current_char == EOF){
            error(err_line, err_col, "EOF in string");
        }
        diyl_append(text, (char)current_char);
    }

    diyl_append(text, '\0');
    next_char();
    Token_t retval;
    retval.token = TokSTR;
    retval.error_line = err_line;
    retval.error_col = err_col;
    retval.name = text;
    return retval;
}

// ****
// kwd_cmp() => keyword_compare()
static int keyword_compare(const void *ptr1, const void *ptr2){
    return strcmp(*(char**)ptr1, *(char**)ptr2);
}

// ****
// get_ident_type() => get_identifier_type()
static TokenEnum_t get_identifier_type(const char *identifier){
    static struct{
        const char *symbol;
        TokenEnum_t token;
    }keywords[] = {
        {.symbol="else", .token = TokELSE},
        {.symbol="if", .token=TokIF},
        {.symbol="print", .token=TokPRINT},
        {.symbol="putc", .token=TokPUTC},
        {.symbol="while", .token=TokWHILE},
    }, *kwp;

    kwp = bsearch(
        &identifier, keywords,
        NELEMS(keywords), sizeof(keywords[0]),
        keyword_compare
    );
    return kwp == NULL ? TokIDENT : kwp->token;
}

// ****
// ident_or_int() => identifier_or_int()
static Token_t identifier_or_int(int err_line, int err_col){
    Token_t retval;
    int n; // XXX long n??
    int isnumber = true;

    diyl_rewind(text);

    while(isalnum(current_char) || current_char == '_'){
        diyl_append(text, (char)current_char);
        if(!isdigit(current_char)){
            isnumber = false;
        }
        next_char();
    }
    if(diyl_len(text) == 0){
        error(
            err_line, err_col,
            "identifier_or_int(): unrecognized character (%d) '%c'\n",
            current_char, current_char
        );
    }

    diyl_append(text, '\0');
    if(isdigit(text[0])){
        if(!isnumber){
            error(
                err_line, err_col,
                "invalide number: %s\n",
                text
            );
        }
        n = strtol(text, NULL, 0);
        if(n == LONG_MAX && errno == ERANGE){
            error(
                err_line, err_col,
                "Number exceed maximum value."
            );
        }
        retval.token = TokINT;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = n;
        return retval;
    }
    retval.token = get_identifier_type(text);
    retval.error_line = err_line;
    retval.error_col = err_col;
    retval.name = text;
    return retval;
}


// **** look ahead for ">=", etc. ****
// follow()
static Token_t follow(
    int expect, TokenEnum_t ifyes, TokenEnum_t ifno,
    int err_line, int err_col
){
    Token_t retval;
    if(current_char == expect){
        next_char();
        retval.token = ifyes;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
    }

    if(ifno == TokEOI){
        error(
            err_line, err_col,
            "follow(): unrecognized character '%c' (%d)",
            current_char, current_char
        );
    }

    retval.token = ifno;
    retval.error_line = err_line;
    retval.error_col = err_col;
    retval.value = 0;
    return retval;
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
