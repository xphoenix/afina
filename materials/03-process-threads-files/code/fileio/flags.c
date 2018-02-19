//#include "apue.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int val;

    if (argc != 2) {
        printf("usage: a.out <descriptor#>\n");
        return 1;
    }

    if ((val = fcntl(atoi(argv[1]), F_GETFL, 0)) < 0) {
        printf("fcntl error for fd %d\n", atoi(argv[1]));
        return 1;
    }

    switch (val & O_ACCMODE) {
    case O_RDONLY:
        printf("read only");
        break;

    case O_WRONLY:
        printf("write only");
        break;

    case O_RDWR:
        printf("read write");
        break;

    default:
        printf("unknown access mode");
        return 1;
    }

    if (val & O_APPEND)
        printf(", append");
    if (val & O_NONBLOCK)
        printf(", nonblocking");
    if (val & O_SYNC)
        printf(", synchronous writes");

    putchar('\n');
    return 0;
}
