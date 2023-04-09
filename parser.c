#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "parser.h"
#include "builtin.h"

void print_tokens(char** args) {
    printf("COMMAND \n");
    for(int i = 0; args[i] != NULL; i ++)
        printf("%s ", args[i]);
    printf("\n");
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


char **cmsh_split_line(char * line) {
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

char** cmsh_split_lines(char** lines) {
    int BUFFSIZE = CMSH_TOK_BUFF_SIZE;
    char** tokens = malloc(BUFFSIZE * sizeof(char*));
    char** temp_args;

    int k = 0, count_args = 0;
    for( ;lines[k] != NULL; k++) {
        temp_args = cmsh_split_line(lines[k]);

        for(int i = 0; temp_args[i] != NULL; i ++, count_args ++) {
            if( count_args >= BUFFSIZE ) {
                BUFFSIZE *= 2;
                tokens = realloc(tokens, BUFFSIZE * sizeof(char*));
            }
            tokens[count_args] = malloc(strlen(temp_args[i]) * sizeof(char));
            strcpy(tokens[count_args], temp_args[i]);
        }
    }

    tokens[count_args] = NULL;
    free(temp_args);
    return tokens;
}

char** cmsh_read_file( char* file ) {

    FILE *fp = fopen(file, "r");
    if( fp == NULL ) {
        perror("cmsh: Error opening file");
        exit(EXIT_FAILURE);
    }

    int BUFFSIZE = CMSH_TOK_BUFF_SIZE;
    char** doc = malloc(BUFFSIZE * sizeof(char*));
    char* line;
    int read, count = 0;
    size_t len;

    while((read = getline(&line, &len, fp)) != -1) {
        if(count >= BUFFSIZE ) {
            BUFFSIZE *= 2;
            doc = realloc(doc, BUFFSIZE * sizeof(char*));
            if( doc == NULL ) {
                perror("cmsh: Error allocation memmory");
                exit(EXIT_FAILURE);
            }
        }

        doc[count] = malloc((read + 1) * sizeof(char));
        if( doc[count] == NULL ) {
            perror("cmsh: Error allocation memmory");
            exit(EXIT_FAILURE);
        }
        strcpy(doc[count], line);
        count ++;
    }

    doc[count] = NULL;
    free(line); fclose(fp);
    return doc;
}