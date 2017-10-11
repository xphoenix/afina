#ifndef _ECHO_H
#define _ECHO_H

#include <stdint.h>

#define SERVERPORT 8080
#define MAXCONN 200
#define MAXEVENTS 100
#define MAXLEN 255

struct EchoEvent {
    int fd;
    uint32_t event;
    char data[MAXLEN];
    int length;
    int offset;
};

#endif // _ECHO_H
