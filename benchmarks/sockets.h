//
// Created by rohit on 1/25/2019.
//

#ifndef THREADS_EXP_SOCKETS_H
#define THREADS_EXP_SOCKETS_H

typedef struct socket_buf {
    int sv[2];
    int buf;
} sock;


inline void init_socket(sock* sock);

inline void send_msg(sock* sock, int msg);

inline void recv_msg(sock* sock);

#endif //THREADS_EXP_SOCKETS_H
