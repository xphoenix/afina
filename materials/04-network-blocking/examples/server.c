#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
 
int main()
{
 
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

	char *str = "HELLO!\r\n";
 
        write(SlaveSock, str, strlen(str)+1);

	shutdown(SlaveSock, 2);
	close(SlaveSock);
    }
}
