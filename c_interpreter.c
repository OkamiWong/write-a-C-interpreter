#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int token;//current token
char *src, *old_src;//pointer to sourcecode string
int poolsize;//default size of text/data/stack
int line;//line number

void next() {
    token = *src++;
    return;
}

void expression(int level) {
    // pass
}

void program() {
    next();//get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}

int eval() { return 0; }

int main(int argc, char **argv) {
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024;
    line = 1;

    //open source file
    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    //malloc memory
    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    //read source file
    if ((i = read(fd, src, poolsize - 1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }

    src[i] = 0;//add EOF
    close(fd);


    program();
    return eval();
}