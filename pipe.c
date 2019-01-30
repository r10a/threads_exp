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
#include "pipe.h"

//static int sv[2]; /* the pair of socket descriptors */
//static int buf; /* for data exchange between processes */

void init_pipe(pipe_t* pip) {
    if (pipe(pip->p) == -1) {
        perror("pipe");
        exit(1);
    }
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
