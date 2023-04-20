//
// Created by cmglezpdev on 4/10/2023.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#include "utils.h"


int file_descriptor_in(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_RDONLY);
    }

    return fd;
}

int file_descriptor_out(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    }

    return fd;
}

int file_descriptor_out_append(char* fileName) {
	char* endptr = 0;
	
	int fd = (int)strtol(fileName,&endptr,10);
	
	if (*(endptr+1) != '\0'){
		fd = open(fileName,O_WRONLY | O_APPEND | O_CREAT, 0600);
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

char* cmsh_read_file( char* file ) {
    int fd = file_descriptor_in(file);
    char* doc = malloc(CMSH_MAX_BUFFSIZE * sizeof(char));
    size_t size = read(fd, doc, CMSH_MAX_BUFFSIZE);

    close(fd);
    return doc;
}

int cmsh_write_file(char* file, char* content, int trunc) {

    if( trunc == 1 ) {
        int fd = file_descriptor_out(file);
        write(fd, content, strlen(content));
        close(fd);
        return 1;
    }

    char* file_content = cmsh_read_file(file);
    file_content = realloc(file_content, strlen(file_content) + strlen(content) + 1);
    if( file_content == NULL ) {
        perror("cmsh: Allocation error");
        exit(EXIT_FAILURE);
        return -1;
    }

    strcat(file_content, content);
    int fd = file_descriptor_out(file);
    write(fd, file_content, strlen(file_content));
    close(fd);
    free(file_content);

    return 1;
}

int is_empty_command(char* command) {
    if( command == NULL ) return 1;
    for(int i = 0; i < strlen(command); i ++) {
        if ( command[i] != ' ' && command[i] != '\n' && command[i] != '#' )
            return 0;
    }
    return 1;
}

int is_number(char* line) {
    for(int i = 0; i < strlen(line); i ++) {
        if( line[i] >= '0' & line[i] <= '9' ) continue;
        return 0;
    }
    return 1;
}

char* operators[] = {">", "<", ">>", "|"};

int is_operator(char* token) {
    for(int i = 0; i < 4; i ++) {
        if( strcmp(token, operators[i]) == 0 ) return 1;
    }
    return 0;
}


// TEMPORALS

void print_tokens(char** args) {
    printf("TOKENS PRINT FUNCTION \n");
    for(int i = 0; args[i] != NULL; i ++)
        printf("%s ", args[i]);
    printf("\n");
}
