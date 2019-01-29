//
// Created by rohit on 1/25/2019.
//

#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <netinet/in.h>
#include "sockets.h"

//static int sv[2]; /* the pair of socket descriptors */
//static int buf; /* for data exchange between processes */

void init_socket(sock* sock) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock->sv) == -1) {
//    if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sock12->sv) == -1) {
//    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sock12->sv) == -1) {
        perror("socketpair");
        exit(1);
    }
//    struct timeval tv;
//    tv.tv_sec = 0;
//    tv.tv_usec = 100;
//    setsockopt(sock->sv[1], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
//    setsockopt(sock->sv[0], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

}

/*
void inline send_msg(sock* sock, int msg) {
    write(sock->sv[0], &msg, sizeof(msg));
//    printf("sent %d\n", msg);
}

void inline recv_msg(sock* sock) {
    read(sock->sv[1], &sock->buf, sizeof(sock->buf));
//    printf("recv %d\n", sock12->buf);
}*/
