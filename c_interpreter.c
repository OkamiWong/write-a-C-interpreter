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
    JZN,
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

void next() {
    token = *src++;
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
        if (op == IMM) {
            ax = *pc++;
        } else if (op == LC) {
            ax = *(char *)ax;
        } else if (op == LI) {
            ax = *(int *)ax;
        } else if (op == SC) {
            ax = *(char *)*sp++ = ax; //wtf?????????
        } else if (op == SI) {
            *(int *)*sp++ = ax;
        } else if (op == PUSH) {
            *--sp = ax;
        }
    }
}

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

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    program();
    return eval();
}