#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include<string.h>
 
int main(int argc,char **argv)
{
    int Socket;
    int n;
    char recvline[100];
    struct sockaddr_in servaddr;
 
    Socket = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof servaddr);
 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(1234);
 
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
 
    connect( Socket, (struct sockaddr *)&servaddr,sizeof(servaddr) );
 
    while(1)
    {
        bzero( recvline, 100);
        read( Socket, recvline, 100);

        printf("%s",recvline);

	break;
    }
 
}
