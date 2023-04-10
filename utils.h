//
// Created by cmglezpdev on 4/10/2023.
//

#ifndef PRACTIC_SHELL_UTILS_H
#define PRACTIC_SHELL_UTILS_H

int redirect_out(char *fileName);
int redirect_in(char *fileName);

void print_tokens(char** tokens);

char *sub_str(char *line, int init, int end);

#endif //PRACTIC_SHELL_UTILS_H

