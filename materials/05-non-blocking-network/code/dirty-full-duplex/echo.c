#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>

#include "echo.h"

int epollfd;

void modifyEpollContext(int epollfd, int operation, int fd, uint32_t events, void *data) {
    struct epoll_event server_listen_event;
    server_listen_event.events = events;
    server_listen_event.data.ptr = data;

    if (-1 == epoll_ctl(epollfd, operation, fd, &server_listen_event)) {
        printf("Failed to add an event for socket%d Error:%s", fd, strerror(errno));
        exit(1);
    }
}
void *handle(void *ptr) {
    uint32_t new_event = 0;

    std::string data;
    data.resize(4096);
    struct EchoEvent *echoEvent = static_cast<struct EchoEvent *>(ptr);

    if (EPOLLIN == echoEvent->event) {
        int n = read(echoEvent->fd, &data[0], 4096);
        if (0 == n) {
            /*
             * Client closed connection.
             */
            printf("\nClient closed connection.\n");
            close(echoEvent->fd);
            free(echoEvent);
        } else if (-1 == n) {
            close(echoEvent->fd);
            free(echoEvent);
        } else {
            // echoEvent->length = n;
            // printf("\nRead data:%s Length:%d, Adding write event.\n", data, n);
            data.resize(n);
            if (echoEvent->output.empty()) {
                new_event |= EPOLLOUT;
            }

            echoEvent->output.push_back(std::move(data));
            if (echoEvent->output.size() < 100) {
                new_event &= ~EPOLLIN;
            }
        }
    }

    if (EPOLLOUT == echoEvent->event) {
        assert(!echoEvent->output.empty());

        // QUEUE:
        // [xxxxxxxxxxx]
        // [xxxxxxxxxxxxxxxxxxxxxxxxx]
        // [xxxxxx]
        int ret;
        auto it = echoEvent->output.begin();
        do {
            std::string &qhead = *it;
            ret = write(echoEvent->fd, &qhead[0] + echoEvent->head_offset, qhead.size() - echoEvent->head_offset);

            if (ret > 0) {
                echoEvent->head_offset += ret;
                if (echoEvent->head_offset >= it->size()) {
                    it++;
                    echoEvent->head_offset = 0;
                }
            }
        } while (ret > 0 && it != echoEvent->output.end());

        it--;
        echoEvent->output.erase(echoEvent->output.begin(), it);

        if (-1 == ret) {
            /*
             * Some other error occured.
             */
            close(echoEvent->fd);
            free(echoEvent);
        }

        if (echoEvent->output.size() < 100) {
            new_event |= EPOLLIN;
        }

        if (!echoEvent->output.empty()) {
            new_event |= EPOLLOUT;
        }
    }

    if (echoEvent->event != new_event) {
        echoEvent->event = new_event;
        /*
         * We have read the data. Add an write event so that we can
         * write data whenever the socket is ready to be written.
         */
        modifyEpollContext(epollfd, EPOLL_CTL_MOD, echoEvent->fd, echoEvent->event, echoEvent);
    }
}

void makeSocketNonBlocking(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL, NULL);
    if (-1 == flags) {
        printf("fcntl F_GETFL failed.%s", strerror(errno));
        exit(1);
    }

    flags |= O_NONBLOCK;
    if (-1 == fcntl(fd, F_SETFL, flags)) {
        printf("fcntl F_SETFL failed.%s", strerror(errno));
        exit(1);
    }
}

int main(int argc, char **argv) {

    int serverfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);

    /*
     * Create server socket. Specify the nonblocking socket option.
     *
     */
    serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (-1 == serverfd) {
        printf("Failed to create socket.%s", strerror(errno));
        exit(1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * Bind the server socket to the required ip-address and port.
     *
     */
    if (-1 == bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        printf("Failed to bind.%s", strerror(errno));
        exit(1);
    }

    /*
     * Mark the server socket has a socket that will be used to .
     * accept incoming connections.
     */
    if (-1 == listen(serverfd, MAXCONN)) {
        printf("Failed to listen.%s", strerror(errno));
        exit(1);
    }

    /*
     * Create epoll context.
     */

    epollfd = epoll_create(MAXCONN);
    if (-1 == epollfd) {
        printf("Failed to create epoll context.%s", strerror(errno));
        exit(1);
    }

    /*
     * Create read event for server socket.
     */
    modifyEpollContext(epollfd, EPOLL_CTL_ADD, serverfd, EPOLLIN, &serverfd);

    /*
     * Main loop that listens for event.
     */
    struct epoll_event *events = calloc(MAXEVENTS, sizeof(struct epoll_event));
    while (1) {
        int n = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if (-1 == n) {
            printf("Failed to wait.%s", strerror(errno));
            exit(1);
        }

        int i;
        for (i = 0; i < n; i++) {
            if (events[i].data.ptr == &serverfd) {
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    /*
                     * EPOLLHUP and EPOLLERR are always monitored.
                     */
                    close(serverfd);
                    exit(1);
                }

                /*
                 * New client connection is available. Call accept.
                 * Make connection socket non blocking.
                 * Add read event for the connection socket.
                 */
                int connfd = accept(serverfd, (struct sockaddr *)&clientaddr, &clientlen);
                if (-1 == connfd) {
                    printf("Accept failed.%s", strerror(errno));
                    exit(1);
                } else {
                    printf("Accepted connection, adding a read event");
                    makeSocketNonBlocking(connfd);

                    struct EchoEvent *echoEvent = calloc(1, sizeof(struct EchoEvent));
                    echoEvent->fd = connfd;
                    echoEvent->event = EPOLLIN;

                    /*
                     * Add a read event.
                     */
                    modifyEpollContext(epollfd, EPOLL_CTL_ADD, echoEvent->fd, echoEvent->event, echoEvent);
                }
            } else {
                /*
                 *A event has happend for one of the connection sockets.
                 *Remove the connection socket from the epoll context.
                 * When the event is handled by handle() function ,
                 *it will add the required event to listen for this
                 *connection socket again to epoll
                 *context
                 */
                if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                    struct EchoEvent *echoEvent = (struct EchoEvent *)events[i].data.ptr;
                    printf("\nClosing connection socket\n");
                    close(echoEvent->fd);
                    free(echoEvent);
                } else {
                    struct EchoEvent *echoEvent = (struct EchoEvent *)events[i].data.ptr;
                    handle(echoEvent);
                }
            }
        }
    }

    free(events);
    exit(0);
}
