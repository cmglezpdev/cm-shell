#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "parser.h"
#include "builtin.h"


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

    return line;
}



char **cmsh_split_line(char * line) {
    int buffsize = CMSH_TOK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;

    if( !tokens ) {
        fprintf(stderr, "cmsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

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

    tokens[position] = NULL;
    return tokens;
}