#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 10

int main(int argc, char *argv[])
{
    int pfd[2];
    char buf[BUF_SIZE];

    ssize_t numRead;

    if (argc != 2) {
        printf("Expected 1 command line argument");
        return -1;
    }

    if (pipe(pfd) == -1) {
        printf("Error: pipe");
        return -1;
    }

    switch(fork()) {
        case -1:
            printf("Error: fork");
            return -1;
        case 0:
            if(close(pfd[1]) == -1) {
                printf("Error: close");
                return -1;
            }

            for(;;) {
                numRead = read(pfd[0], buf, BUF_SIZE);
                if (numRead == -1)
                    return -1; /* Error */
                if (numRead == 0)
                    break; /* EOF */
                if (write(STDOUT_FILENO, buf, numRead) != numRead)
                    return -1; /* wirte failed */
            }

            write(STDOUT_FILENO, "\n", 1);
            if (close(pfd[0]) == -1)
                return -1; /* Error */

            _exit(EXIT_SUCCESS);
        
        default:
            if (close(pfd[0] == -1))
                return -1; /* Error */

            if (write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1]))
                return -1; /* wirte error */

            if (close(pfd[1]) == -1)
                return -1; /* close error */

            wait(NULL);
            exit(EXIT_SUCCESS);
    }
}
