/*
 * Contains definitions of constants and data structures used.
 */

#define SERVERPORT 8080

#define MAXCONN 200

#define MAXEVENTS 100

#define MAXLEN 255

struct EchoEvent
{
    int fd;
    uint32_t event;
    char data[MAXLEN];
    int length;
    int offset;

};
