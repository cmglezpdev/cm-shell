//
// Created by cmglezpdev on 4/10/2023.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>

#include "utils.h"

int redirect_out(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    }

    return fd;
}

int redirect_in(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_RDONLY);
    }

    return fd;
}

char *sub_str(char *line, int init, int end) {
    char *new_line = (char *) malloc(end - init + 1);

    int i;
    for (i = 0; i < end - init + 1; i++) {
        new_line[i] = line[i + init];
    }
    new_line[i] = 0;

    return new_line;
}



// TEMPORALS

void print_tokens(char** args) {
    printf("TOKENS PRINT FUNCTION \n");
    for(int i = 0; args[i] != NULL; i ++)
        printf("%s ", args[i]);
    printf("\n");
}
