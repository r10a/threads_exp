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
    struct socket_buf sock;
} params;

void init_socket(sock* sock);

void send_msg(sock* sock, int msg);

void recv_msg(sock* sock);

#endif //THREADS_EXP_SOCKETS_H
