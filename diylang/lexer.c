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

// Get token type
// gettok() => get_token()
Token_t get_token(void){
    Token_t retval;
    // skip white space
    while(isspace(current_char)){ next_char();}
    int err_line = lineno;
    int err_col = column;
    switch(current_char){
    case '{':
        next_char();
        retval.token = TokLBRACE;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '}':
        next_char();
        retval.token = TokRBRACE;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '(':
        next_char();
        retval.token = TokLPAREN;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case ')':
        next_char();
        retval.token = TokRPAREN;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '+':
        next_char();
        retval.token = TokADD;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '-':
        next_char();
        retval.token = TokSUB;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '*':
        next_char();
        retval.token = TokMUL;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '%':
        next_char();
        retval.token = TokMOD;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case ';':
        next_char();
        retval.token = TokSEMICOLON;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case ',':
        next_char();
        retval.token = TokCOMMA;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    case '/':
        next_char();
        retval = divide_or_comment(err_line, err_col);
        break;
    case '\'':
        next_char();
        retval = char_literal(current_char, err_line, err_col);
        break;
    case '<':
        next_char();
        retval = follow('=', TokLE, TokLT, err_line, err_col);
        break;
    case '>':
        next_char();
        retval = follow('=', TokGE, TokGT, err_line, err_col);
        break;
    case '=':
        next_char();
        retval = follow('=', TokEQ, TokASSIGN, err_line, err_col);
        break;
    case '!':
        next_char();
        retval = follow('=', TokNE, TokNOT, err_line, err_col);
        break;
    case '&':
        next_char();
        retval = follow('&', TokAND, TokEOI, err_line, err_col);
        break;
    case '|':
        next_char();
        retval = follow('|', TokOR, TokEOI, err_line, err_col);
        break;
    case '"':
        next_char();
        retval = string_literal(current_char, err_line, err_col);
        break;
    case EOF:
        next_char();
        retval.token = TokEOI;
        retval.error_line = err_line;
        retval.error_col = err_col;
        retval.value = 0;
        break;
    default:
        retval = identifier_or_int(err_line, err_col);
        break;
    }// end switch

    return retval;
}

// **** Tokenize the given input
// run() => tokenize()
void tokenize(void){
    Token_t symbol;
    do{
        symbol = get_token();
        fprintf(dstfile,
            "%5d, %5d %.15s",
            symbol.error_line, symbol.error_col,
            &"End_of_input    "
             "OP_multiply     "
             "OP_divide       "
             "OP_mod          "
             "OP_add          "
             "OP_substract    "
             "OP_negate       "
             "OP_not          "
             "OP_less         "
             "OP_lessequal    "
             "OP_greater      "
             "OP_greaterequal "
             "OP_equal        "
             "OP_notequal     "
             "OP_assign       "
             "OP_and          "
             "OP_or           "
             "KWD_if          "
             "KWD_else        "
             "KWD_while       "
             "KWD_print       "
             "KWD_putc        "
             "LPAREN          "
             "RPAREN          "
             "LBRACE          "
             "RBRACE          "
             "SEMICOLON       "
             "COMMA           "
             "IDENTIFIER      "
             "INTEGER         "
             "STRING          "
            [symbol.token * 16]
        );

        if(symbol.token == TokINT){
            fprintf(dstfile, " %4d", symbol.value);
        }else if(symbol.token == TokIDENT){
            fprintf(dstfile, " %s", symbol.name);
        }else if(symbol.token == TokSTR){
            fprintf(dstfile, "\"%s\"", symbol.name);
        }
        fprintf(dstfile, "\n");
    }while(symbol.token != TokEOI);

    if(dstfile != stdout){ fclose(dstfile); }
}

// *****
// init_io() => initialze()
void initialize(FILE **fp, FILE *std, const char mode[], const char filename[]){
    if(filename[0] == '\0'){
        fp = std;
    }else if((*fp = fopen(filename, mode)) == NULL){
        error(0, 0, "Can't open %s\n", filename);
    }
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
