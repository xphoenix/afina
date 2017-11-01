#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int pfd[2];

    pipe(pfd);

    switch(fork()) {
        case -1:
            return -1;
        case 0:
            close(pfd[0]);

            if(pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[2]);
            }

            execlp("ls", "ls", (char *)NULL);
            
        default:
            break;
    }

    switch(fork()) {
        case -1:
            return -1;
        case 0:
            close(pfd[1]);

            if(pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }

            execlp("wc", "wc", "-l", (char *)NULL);
        default:
            break;
    }

    close(pfd[0]);
    close(pfd[1]);
    
    wait(NULL);
    wait(NULL);

    return 0;
}
