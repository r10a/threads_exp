//
// Created by rohit on 11/16/18.
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

int print(char *str) {
    write(1, str, strlen(str));
}

int print_int(int num) {
    char number[20];
    sprintf(number, "%d", num);
    print(number);
}
