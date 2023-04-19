#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "utils.h"
#include "parser.h"
#include "builtin.h"


char *delete_comment(char *line) {
    int k = 0;
    for(; k < strlen(line) && line[k] != '#'; k ++);
    if( k == strlen(line) ) return line;
    return sub_str(line, 0, k - 1);
}

char *cmsh_read_line( void ) {
    char *line = NULL;
    size_t buffsize = 0;

    if( getline(&line, &buffsize, stdin) == -1 ) {
        if( feof(stdin) ) {
            exit(EXIT_SUCCESS);
        } else {
            perror("cmsh: getline\n");
            exit(EXIT_FAILURE);
        }
    }

    return  line;
}


char **cmsh_split_line(const char* input) {
    char * line = malloc(strlen(input));
    strcpy(line, input);

    int buffsize = CMSH_TOK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;

    if( !tokens ) {
        fprintf(stderr, "cmsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    line = delete_comment(line);
    
    token = strtok(line, CMSH_TOK_DELIM);
    while(token != NULL) {
        tokens[position ++] = token;

        if( position >= buffsize ) {
            buffsize += CMSH_TOK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));

            if( !tokens ) {
                fprintf(stderr, "cmsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CMSH_TOK_DELIM);
    }

    free(token);
    tokens[position] = NULL;
    return tokens;
}

char** add_new_args_from_file(char* command, char* file) {
    int buffsize = CMSH_TOK_BUFF_SIZE;
    char* doc = cmsh_read_file(file);
    char** tokens = cmsh_split_line(doc);
    char** args = malloc(buffsize * sizeof(char*));
    int size;

    args[0] = command;
    for(size = 1; tokens[size - 1] != NULL; size ++) {
        if( size >= buffsize ) {
            buffsize *= 2;
            args = realloc(args, buffsize * sizeof(char*));
        }
        args[size] = malloc(strlen(tokens[size - 1]) * sizeof(char));
        strcpy(args[size], tokens[size - 1]);
    }
    args[size] = NULL;
    free(doc); free(tokens);
    return args;
}
