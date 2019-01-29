//
// Created by rohit on 1/25/2019.
//

#ifndef THREADS_EXP_SOCKETS_H
#define THREADS_EXP_SOCKETS_H

typedef struct socket_buf {
    int sv[2];
    int buf;
} sock;

typedef struct params {
    int id;
    struct socket_buf sock12;
    struct socket_buf sock23;
} params;

void init_socket(sock* sock);

void inline send_msg(sock* sock, int msg) {
    write(sock->sv[0], &msg, sizeof(msg));
//    printf("sent %d\n", msg);
}

void inline recv_msg(sock* sock) {
    read(sock->sv[1], &sock->buf, sizeof(sock->buf));
//    printf("recv %d\n", sock12->buf);
}

#endif //THREADS_EXP_SOCKETS_H
