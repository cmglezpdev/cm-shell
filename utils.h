//
// Created by cmglezpdev on 4/10/2023.
//

#ifndef PRACTIC_SHELL_UTILS_H
#define PRACTIC_SHELL_UTILS_H

#define CMSH_MAX_BUFFSIZE 999999
#define CMSH_TOK_BUFF_SIZE 1024
#define CMSH_TOK_DELIM " \t\r\n\a"

int redirect_in(char *fileName);
int redirect_out(char *fileName);
int redirect_out_append(char* fileName);

void print_tokens(char** tokens);

char *sub_str(char *line, int init, int end);

char* cmsh_read_file( char* file );

int cmsh_write_file(char* file, char* content, int trunc);

#endif //PRACTIC_SHELL_UTILS_H

