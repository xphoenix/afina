#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
 
int main()
{
    pid_t p; 
    int rval;
    char str[100];
    int MasterSock, SlaveSock;
 
    struct sockaddr_in servaddr;
 
    MasterSock = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(MasterSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
 
    bzero( &servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(1234);
 
    bind(MasterSock, (struct sockaddr *) &servaddr, sizeof(servaddr));
 
    listen(MasterSock, 10);
 
    while(1)
    {
    	SlaveSock = accept(MasterSock, (struct sockaddr*) NULL, NULL);

        if( SlaveSock == -1)
            perror("accept");

        if ( (p = fork()) == 0 ) {
            close(MasterSock);

            do {
                bzero(str, sizeof(str));
                if ((rval = read(SlaveSock, str, 1024)) < 0)
                    perror("reading stream error");
                else if (rval == 0)
                    printf("client disconnected\n");
                else
                    printf("-->%s\n", str);
            } while( rval > 0);

            return 0;
        }

        close( SlaveSock );
    }
}
