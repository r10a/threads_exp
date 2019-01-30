//
// Created by rohit on 1/25/2019.
//

#ifndef THREADS_EXP_SOCKETS_H
#define THREADS_EXP_SOCKETS_H

#define CACHE_LINE_SIZE 64

typedef struct pipe_t {
    int p[2];
    long long buf __attribute__((aligned(CACHE_LINE_SIZE)));
} pipe_t;

typedef struct params {
    int id;
    struct pipe_t pipe12;
    struct pipe_t pipe23;
    struct pipe_t pipe32;
    struct pipe_t pipe21;
} params;

void init_pipe(pipe_t* pipe);

void inline send_msg(pipe_t* pipe, int msg) {
//    write(sock->sv[0], &msg, sizeof(msg));
//    printf("sent %d\n", msg);
}

void inline recv_msg(pipe_t* pipe) {
//    read(sock->sv[1], &sock->buf, sizeof(sock->buf));
//    printf("recv %d\n", sock12->buf);
}

#endif //THREADS_EXP_SOCKETS_H
