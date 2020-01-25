#include <stdio.h>
#include <stdlib.h>

int main() {
    int ax = 110;
    int *sp;
    sp = malloc((int)100);
    if (sp == NULL)return 0;
    int raw_ax = ax;
    ax = *(char *)*sp++ = ax;
    printf("%d %d\n", raw_ax, ax);
    return 0;
}