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
            // exit(EXIT_SUCCESS);
        } else {
            perror("cmsh: getline\n");
            return NULL;
        }
    }

    return  line;
}


char **cmsh_split_line(const char* input, char* delim) {
    char * line = malloc(strlen(input));
    strcpy(line, input);

    int buffsize = CMSH_TOK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;
    if( !tokens ) {
        free(line);
        perror("cmsh: allocation error\n");
        return NULL;
    }

    token = strtok(line, delim);
    while(token != NULL) {
        tokens[position] = malloc(strlen(token));
        strcpy(tokens[position ++], token);

        if( position >= buffsize ) {
            buffsize += CMSH_TOK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));

            if( !tokens ) {
                perror("cmsh: allocation error\n");
                return NULL;
            }
        }

        token = strtok(NULL, delim);
    }

    free(token); free(line);
    tokens[position] = NULL;
    return tokens;
}

char* remplace_command_again(char* line) {
    // remplace the again commands by the corresponding command in the history
    char **tokens = cmsh_split_line(line, CMSH_TOK_DELIM);
    char *command = malloc(CMSH_TOK_BUFF_SIZE);
    ( line[0] == ' ' ) ? strcpy(command, " ") : strcpy(command, "");

    for(int t = 0; tokens[t] != NULL; t ++) {
        if( strcmp(tokens[t], "again") == 0 ) {
            if( tokens[t + 1] == NULL || !is_number(tokens[t + 1]) ) {
                perror("cmsh: the again command needs a number like parameter\n");
                return NULL;
            }

            char* history_command = get_again((int)strtol(tokens[++t], NULL, 10));
            if( history_command == NULL )
                return NULL;

            strcat(command, history_command);
            strcat(command, " ");
            continue;
        }
    
        strcat(command, tokens[t]);
        strcat(command, " ");
    }

    free(line); free(tokens);
    return command;
}