/* Drift
 * 
 * 	- https://drift-lang.fun/
 *
 * GPL 3.0 License - bingxio <3106740988@qq.com> */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Types of all tokens */
typedef enum { 
    EOH,	   LITERAL,   NUMBER,    STRING,  CHAR,      FLOAT,   
    ADD,       SUB,       MUL,       DIV,     SUR,       AS_ADD,
    AS_SUB,    AS_MUL,    AS_DIV,    AS_SUR,  R_ARROW,   L_ARROW, 
    DOT,	   COMMA,     COLON,     EQ,      SEMICOLON, GREATER, 
    LESS,      GR_EQ,     LE_EQ,     ADDR,	  OR,        BANG,  
    BANG_EQ,   EQ_EQ,     L_BRACE,   R_BRACE, L_PAREN,   R_PAREN,
    L_BRACKET, R_BRACKET, UNDERLINE, DEF,     RET, 	     FOR,
    AOP,       IF,        EF,        NF,      NEW, 	     OUT,
    GO, 	   MOD,       USE
} token_kind;

/* keywords */
const char *keyword[12] = {
    "def", "ret", "for", "aop", "if",  "ef", 
    "nf",  "new", "out", "go",  "mod", "use"
};

/* Token properties */
typedef struct { token_kind kind; const char *literal; int line; int off; } token;

/* The lexical module returns and interprets the token stream */
typedef struct { int len; int cap; token *tokens; } lexer_result;

/* Whether the symbol is a space, carriage return, line feed, indent */
bool is_space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

/* Are the symbols numbers 0 to 9 */
bool is_digit(char c) { return c >= '0' && c <= '9'; }

/* Is the symbol a legal identifier */
bool is_ident(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/* Analyze the types of strings */
token_kind to_keyword(const char *literal) {
    for (int i = 0; i < 12; i ++) {
        if (strcmp(literal, keyword[i]) == 0) {
            if (i ==  0) return DEF; if (i ==  1) return RET;
            if (i ==  2) return FOR; if (i ==  3) return AOP;
            if (i ==  4) return IF;  if (i ==  5) return EF;
            if (i ==  6) return NF;  if (i ==  7) return NEW;
            if (i ==  8) return OUT; if (i ==  9) return GO;
            if (i == 10) return MOD; if (i == 11) return USE;
        }
    }
    return LITERAL;
}

/* Build token */
token new_token(token_kind k, const char *literal, int line, int off) {
    token t = {.kind = k, .literal = literal, .line = line, .off = off};
    return t;
}

/* Add elements,
 * and automatically expand according to the length */
void *append(void *ptr, int *len, int *cap, size_t size) {
    if (*cap == 0 || *len + 1 > *cap) {
        if (*cap == 0) {
            *cap = 4;
        } else {
            *cap *= 2;
        }
        ptr = realloc(ptr, size * (*cap)); /* Double capacity */
    }
    return ptr;
}

/* Append token */
void append_token(lexer_result *lex, token tok) {
    lex->tokens = append(lex->tokens, &lex->len, &lex->cap, sizeof(token));
    lex->tokens[lex->len++] = tok;
}

/* Determine whether the next character is the specified type */
bool next(const char *buf, int *p, char c) {
    if (buf[*p + 1] == c) {
        *p += 2;
        return true;
    }
    (*p) ++;
    return false;
}

/* Lexical analysis */
lexer_result *lexer(const char *buf, int fsize) {
    lexer_result *l = (lexer_result *) malloc(sizeof(lexer_result)); /* Result */
    bool new_line = true; /* Indent judgement */
    for (int i = 0, line = 1, off = 0; i < fsize;) {
        char c = buf[i];
        while (is_space(c)) { /* Space */
            if (c == '\n') {
                line ++;
                off = -1;
                new_line = true;
            }
            c = buf[++ i];
            /* If it's a new line,
             * And it's dealing with spaces, increase indent */
            if (new_line) off ++;
        }
        if (new_line) new_line = false; /* To catch indent */
        if (is_digit(c)) { /* Digit */
            int p = 0;
            bool f = false;
            while (is_digit(c) || c == '.') {
                if (c == '.') f = true;
                p ++;
                c = buf[++ i];
            }
            char *literal = (char *) malloc(p * sizeof(char));
            for (int x = 0, j = p; x < j; x ++) {
                literal[x] = buf[i - p];
                p --;
                if (x + 1 == j) literal[x + 1] = '\0';
            }
            append_token(l, new_token(f ? FLOAT : NUMBER, literal, line, off));
            continue;
        }
        if (is_ident(c)) { /* Ident */
            int p = 0;
            while (is_ident(c)) {
                p ++;
                c = buf[++ i];
            }
            char *literal = (char *) malloc((p + 1) * sizeof(char));
            for (int x = 0, j = p; x < j; x ++) {
                literal[x] = buf[i - p];
                p --;
                if (x + 1 == j) literal[x + 1] = '\0';
            }
            append_token(l, new_token(
                        to_keyword(literal), literal, line, off));
            continue;
        }
        token t = {.line = line, .off = off}; /* Others */
        switch (c) {
            case '+':
                if (next(buf, &i, '=')) {t.kind = AS_ADD; t.literal = "+=";}
                else {t.kind = ADD; t.literal = "+";}
                break;
            case '-':
                if (next(buf, &i, '=')) {t.kind = AS_SUB; t.literal = "-=";}
                else {
                    if (buf[i] == '>') {
                        t.kind = R_ARROW; t.literal = "->";
                        i ++;
                    } else {
                        t.kind = SUB; t.literal = "-";
                    }
                }
                break;
            case '*':
                if (next(buf, &i, '=')) {t.kind = AS_MUL; t.literal = "*=";}
                else {t.kind = MUL; t.literal = "*";}
                break;
            case '/':
                if (next(buf, &i, '=')) {t.kind = AS_DIV; t.literal = "/=";}
                else {t.kind = DIV; t.literal = "/";}
                break;
            case '%':
                if (next(buf, &i, '=')) {t.kind = AS_SUR; t.literal = "%=";}
                else {t.kind = SUR; t.literal = "%";}
                break;
            case '<':
                if (next(buf, &i, '=')) {t.kind = LE_EQ; t.literal = "<=";}
                else {
                    if (buf[i] == '-') {
                        t.kind = L_ARROW; t.literal = "<-";
                        i ++;
                    } else {
                        t.kind = LESS; t.literal = "<";
                    }
                }
                break;
            case '>':
                if (next(buf, &i, '=')) {t.kind = GR_EQ; t.literal = ">=";}
                else {t.kind = GREATER; t.literal = ">";}
                break;
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\0':
                continue;
            case '#':
                while (buf[++ i] != '\n');
                continue;
            case '\'': { /* Char */
                i ++;
                char *literal = (char *) malloc(sizeof(char));
                literal[0] = buf[i ++];
                c = buf[i];
                if (c != '\'') {
                    fprintf(stderr, 
                        "<lexer %d>: missing single quotation mark to the right.\n",
                        line);
                    exit(EXIT_FAILURE);
                } else {
                    i += 2;
                }
                t.kind = CHAR; t.literal = literal;
            }
                break;
            case '"': { /* String */
                c = buf[++ i];
                int p = 0;
                while (c != '"') {
                    c = buf[++ i];
                    p ++;
                    if (i == fsize) {
                        fprintf(stderr,
                            "<lexer %d>: missing closing double quote.\n", line);
                        exit(EXIT_FAILURE);
                    }
                }
                char *literal = (char *) malloc(p * sizeof(char));
                for (int x = 0, j = p; x < j; x ++) {
                    literal[x] = buf[i - p];
                    p --;
                    if (x + 1 == j) literal[x + 1] = '\0';
                }
                i ++;
                t.kind = STRING; t.literal = literal;
            }
                break;
            case '.':
                i ++;
                t.kind = DOT; t.literal = ".";
                break;
            case ',':
                i ++;
                t.kind = COMMA; t.literal = ",";
                break;
            case ':':
                i ++;
                t.kind = COLON; t.literal = ":";
                break;
            case '=':
                if (next(buf, &i, '=')) {t.kind = EQ_EQ; t.literal = "==";}
                else {t.kind = EQ; t.literal = "=";}
                break;
            case ';':
                i ++;
                t.kind = SEMICOLON; t.literal = ";";
                break;
            case '&':
                i ++;
                t.kind = ADDR; t.literal = "&";
                break;
            case '|':
                i ++;
                t.kind = OR; t.literal = "|";
                break;
            case '!':
                if (next(buf, &i, '=')) {t.kind = BANG_EQ; t.literal = "!=";}
                else {t.kind = BANG; t.literal = "!";}
                break;
            case '{':
                i ++;
                t.kind = L_BRACE; t.literal = "{";
                break;
            case '}':
                i ++;
                t.kind = R_BRACE; t.literal = "}";
                break;
            case '[':
                i ++;
                t.kind = L_BRACKET; t.literal = "[";
                break;
            case ']':
                i ++;
                t.kind = R_BRACKET; t.literal = "]";
                break;
            case '(':
                i ++;
                t.kind = L_PAREN; t.literal = "(";
                break;
            case ')':
                i ++;
                t.kind = R_PAREN; t.literal = ")";
                break;
            default:
                fprintf(stderr, "<lexer %d>: Unknown character '%c'.\n",
                    line, c);
                exit(EXIT_FAILURE);
        }
        append_token(l, t);
    }
    /* Terminator */
    append_token(l, new_token(EOH, "EOH", l->tokens[l->len - 1].line + 1, 0));
    return l;
}

/* All bytecodes */
typedef enum {
    LOAD_CONST,    ASSIGN_TO,  STORE_NAME, LOAD_OF,
    INDEX_TO,      REPLACE_TO, GET_OF,     SET_TO,
    CALL_FUNC,     SET_NAME,   NEW_OBJ,    BUILD_FUNC,
    BUILD_WHOLE,   BUILD_ENUM, SET_MODULE, USE_MOD,
    BUILD_ARR,     BUILD_TUP,  BUILD_MAP,  TO_ADD,
    TO_SUB,        TO_MUL,     TO_DIV,     TO_SUR,
    TO_GR,         TO_LE,      TO_GR_EQ,   TO_LE_EQ,
    TO_EQ_EQ,      TO_NOT_EQ,  TO_AND,     TO_OR,
    TO_BANG,       TO_NOT,     JUMP_TO,    TRUE_JUMP_TO,
    FALSE_JUMP_TO, RET_NONE,   RET_VAL,
} op_code;

typedef struct {} comp_result; 

comp_result *compile(lexer_result *lex) {
    comp_result *com = (comp_result *) malloc(sizeof(comp_result));
    for (int i = 0; i < lex->len; i ++) {
        token tok = lex->tokens[i];
        switch (tok.kind) {
            default:
                printf("EXPR\n");
        }
    }
    return com;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: ./drift (file)\n");
        exit(EXIT_FAILURE);
    }
    const char *path = argv[1];
    FILE *fp = fopen(path, "r"); /* Open file of path */
    if (fp == NULL) {
        printf("<compiler>: Failed to read buffer of file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp); /* Returns the size of file */
    fseek(fp, 0, SEEK_SET);
    char *buf = (char *) malloc(fsize * sizeof(char));
    
    fread(buf, fsize, sizeof(char), fp); /* Read file to buffer*/
    buf[fsize] = '\0';

    /* Lexical analysis */
    lexer_result *lex = lexer(buf, fsize);
    for (int i = 0; i < lex->len; i ++) {
        token t = lex->tokens[i];
        printf("[%3d]\t%-5d %-20s %-20d %d\n", i, t.kind, t.literal, t.line, t.off);
    }
    comp_result *com = compile(lex);
    
    fclose(fp); /* Close file */
    free(buf); /* Close buffer */
    return 0;
}
