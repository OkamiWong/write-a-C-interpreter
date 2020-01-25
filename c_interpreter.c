#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    LEA,
    IMM,
    JMP,
    CALL,
    JZ,
    JNZ,
    ENT,
    ADJ,
    LEV,
    LI,
    LC,
    SI,
    SC,
    PUSH,
    OR,
    XOR,
    AND,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    SHL,
    SHR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    OPEN,
    READ,
    CLOS,
    PRTF,
    MALC,
    MSET,
    MCMP,
    EXIT
}; // virtual machine commands

enum {
    Num = 128,
    Fun,
    Sys,
    Glo,
    Loc,
    Id,
    Char,
    Else,
    Enum,
    If,
    Int,
    Return,
    Sizeof,
    While,
    Assign,
    Cond,
    Lor,
    Lan,
    Or,
    Xor,
    And,
    Eq,
    Ne,
    Lt,
    Gt,
    Le,
    Ge,
    Shl,
    Shr,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Inc,
    Dec,
    Brak
}; // tokens and classes (operators last and in precedence order)

// structure
int token;           // current token
char *src, *old_src; // pointer to sourcecode string
int poolsize;        // default size of text/data/stack
int line;            // line number

// virtual machine
int *text,     // text segment
    *old_text, // for dump text segment
    *stack;
char *data; // data segment

int *pc, *bp, *sp, ax, cycle; // virtual machine registers

enum { Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize };
int token_val;   // value of current token (mainly for number)
int *current_id, // current parsed ID
    *symbols;    // symbol table

void next() {
    char *last_pos;
    int hash;

    while (token = *src) {
        ++src;
        // parse token here
        if (token == '\n') {
            ++line;
        } else if (token == '#') {
            // skip macro, which is not supported in this project
            while (*src != 0 && *src != '\n') {
                src++;
            }
        } else if ((token >= 'a' && token <= 'z') ||
                   (token >= 'A' && token <= 'Z') || (token == '_')) {
            // parse identifier
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') ||
                   (*src >= 'A' && *src <= 'Z') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash &&
                    !memcmp((char *)current_id[Name], last_pos,
                            src - last_pos)) {
                    // found one return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        } else if (token >= '0' && token <= '9') {
            // parse number, three kinds: dec(123), hex(0x123), oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + *src++ - '0';
                }
            } else {
                // starts with number 0
                if (*src == 'x' || *src == 'X') {
                    // hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') ||
                           (token >= 'a' && token <= 'f') ||
                           (token >= 'A' && token <= 'F')) {
                        token_val =
                            token_val * 16 + (token & 15) +
                            (token >= 'A'
                                 ? 9
                                 : 0); // when token>='A', token must >'9'
                        token = *++src;
                    }
                } else {
                    // oct, starts with 0 followed by a digit
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val * 8 + *src++ - '0';
                    }
                }
            }
            token = Num;
            return;
        } else if (token == '"' || token == '\'') {
            // parse string literal, currently, the only supproted escape
            // character is '\n', store the string literal into data.
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    // escape character
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }
                if (token_val == '"') {
                    *data++ = token_val;
                }
            }
            src++;
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }
            return;
        } else if (token == '/') {
            if (*src == '/') {
                // skip comments
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // divide operator
                token = Div;
                return;
            }
        } else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        } else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        } else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        } else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<') {
            // parse '<=',"<<" or '<'
            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '<') {
                src++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        } else if (token == '>') {
            // parse '>=',">" or '>'
            if (*src == '=') {
                src++;
                token = Ge;
            } else if (*src == '>') {
                src++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        } else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        } else if (token == '&') {
            // parse '&' or '&&'
            if (*src == '&') {
                src++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        } else if (token == '^') {
            token = Xor;
            return;
        } else if (token == '%') {
            token = Mod;
            return;
        } else if (token == '*') {
            token = Mul;
            return;
        } else if (token == '[') {
            token = Brak;
            return;
        } else if (token == '?') {
            token = Cond;
            return;
        } else if (token == '~' || token == ';' || token == '{' ||
                   token == '}' || token == '(' || token == ')' ||
                   token == ']' || token == ',' || token == ':') {
            return;
        }
    }
    return;
}

void expression(int level) {
    // pass
}

void program() {
    next(); // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}

int eval() {
    int op, *tmp;
    while (1) {
        op = *pc++;
        if (op == IMM) {
            ax = *pc++;
        } else if (op == LC) {
            ax = *(char *)ax;
        } else if (op == LI) {
            ax = *(int *)ax;
        } else if (op == SC) {
            ax = *(char *)*sp++ = ax; // wtf?????????
        } else if (op == SI) {
            *(int *)*sp++ = ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else if (op == JMP) {
            pc = (int *)*pc;
        } else if (op == JZ) {
            pc = ax ? pc + 1 : (int *)*pc;
        } else if (op == JNZ) {
            pc = ax ? (int *)*pc : pc + 1;
        } else if (op == CALL) {
            *--sp = (int)(pc + 1);
            pc = (int *)*pc;
        } else if (op == ENT) {
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        } else if (op == ADJ) {
            sp = sp + *pc++;
        } else if (op == LEV) {
            sp = bp;
            bp = (int *)*sp++;
            pc = (int *)*sp++;
        } else if (op == LEA) {
            ax = (int)(bp + *pc++);
        } else if (op == OR)
            ax = *sp++ | ax;
        else if (op == XOR)
            ax = *sp++ ^ ax;
        else if (op == AND)
            ax = *sp++ & ax;
        else if (op == EQ)
            ax = *sp++ == ax;
        else if (op == NE)
            ax = *sp++ != ax;
        else if (op == LT)
            ax = *sp++ < ax;
        else if (op == LE)
            ax = *sp++ <= ax;
        else if (op == GT)
            ax = *sp++ > ax;
        else if (op == GE)
            ax = *sp++ >= ax;
        else if (op == SHL)
            ax = *sp++ << ax;
        else if (op == SHR)
            ax = *sp++ >> ax;
        else if (op == ADD)
            ax = *sp++ + ax;
        else if (op == SUB)
            ax = *sp++ - ax;
        else if (op == MUL)
            ax = *sp++ * ax;
        else if (op == DIV)
            ax = *sp++ / ax;
        else if (op == MOD)
            ax = *sp++ % ax;
        else if (op == EXIT) {
            printf("exit(%d)\n", *sp);
            return *sp;
        } else if (op == OPEN) {
            ax = open((char *)sp[1], sp[0]);
        } else if (op = CLOS) {
            ax = close(*sp);
        } else if (op == READ) {
            ax = read(sp[2], (char *)sp[1], *sp);
        } else if (op == PRTF) {
            tmp = sp + pc[1];
            ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5],
                        tmp[-6]);
        } else if (op == MALC) {
            ax = (int)malloc(*sp);
        } else if (op == MSET) {
            ax = (int)memset((char *)sp[2], sp[1], *sp);
        } else if (op == MCMP) {
            ax = memcmp((char *)sp[2], (char *)sp[1], *sp);
        } else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

// types of variable/function
enum { CHAR, INT, PTR };
int *idmain;

int main(int argc, char **argv) {
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024;
    line = 1;

    // open source file
    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    // allocate memory for virtual machine
    if (!(text = old_text = malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }
    if (!(symbols = malloc(poolsize))) {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);
    memset(symbols, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    src = "char else enum if int return sizeof while"
          "open read close printf malloc memset memcmp exit void main";
    // add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next();
    current_id[Token] = Char; // handle void type
    next();
    idmain = current_id; // keep track of main

    // allocate memory for text
    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read source file
    if ((i = read(fd, src, poolsize - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }

    src[i] = 0; // add EOF
    close(fd);

    // test virtual machine
    /*
    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;
    */
    // test end

    program();
    return eval();
}