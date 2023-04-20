//
// Created by cmglezpdev on 4/10/2023.
//

#ifndef PRACTIC_SHELL_UTILS_H
#define PRACTIC_SHELL_UTILS_H

#define CMSH_MAX_BUFFSIZE 999999
#define CMSH_TOK_BUFF_SIZE 1024
#define CMSH_TOK_DELIM " \t\r\n\a"

int file_descriptor_in(char *fileName);
int file_descriptor_out(char *fileName);
int file_descriptor_out_append(char* fileName);

void print_tokens(char** tokens);

char *sub_str(char *line, int init, int end);

int is_empty_command(char* command);

int is_number(char* line);

char* cmsh_read_file( char* file );

int cmsh_write_file(char* file, char* content, int trunc);

extern char *operators[4];

int is_operator(char* token);

int contain(char c, char* line);

int is_sub(char* patt, char* line, int pos);

#endif //PRACTIC_SHELL_UTILS_H

