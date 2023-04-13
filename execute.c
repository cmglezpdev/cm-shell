//
// Created by cmglezpdev on 4/8/2023.
//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>
#include<fcntl.h>

#include "execute.h"
#include "builtin.h"
#include "utils.h"
#include "parser.h"

int cmsh_launch(char **args, char* input, char* output, int append) {
    pid_t pid, wpid;
    int status;

    int fd[2] = {-1, -1};
    if( input != NULL ) 
        fd[0] = file_descriptor_in(input);
    if( output != NULL ) 
        fd[1] = append == 1 ? file_descriptor_out_append(output) : file_descriptor_out(output);

    pid = fork();
    if(pid == 0) {
        if( fd[0] != -1 ) {
            args = add_new_args_from_file(args[0], input);
            close(fd[0]);
        }

        if( fd[1] != -1 ) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
        }

        // Child process
        if( execvp(args[0], args) == -1 ) {
            perror("cmsh: command error\n");
            exit(EXIT_FAILURE);
        }
        if( fd[0] != -1 ) dup2(STDIN_FILENO, fd[0]);
        if( fd[1] != -1 ) dup2(STDOUT_FILENO, fd[1]);

    } else if (pid < 0) {
        // Error forking
        perror("cmsh: forking error\n");
    } else {
        // Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int cmsh_execute(char **args, char* input, char* output, int append) {
    if( args[0] == NULL ) {
        return 1;
    }

    for(int i = 0; i < cmsh_num_builtins(); i ++) {
        if( strcmp(args[0], builtin_str[i]) == 0 )
            return (*builtin_func[i])(args);
    }

    return cmsh_launch(args, input, output, append);
}

char* operators[] = {">", "<", ">>", "|"};
int is_operator(char* token) {
    for(int i = 0; i < 4; i ++) {
        if( strcmp(token, operators[i]) == 0 ) return 1;
    }
    return 0;
}

int extract_command(char** tokens, int start, char** command) {
    int i = 0, end = start;
    for(; tokens[end] != NULL && is_operator(tokens[end]) == 0; end ++, i ++) {
        command[i] = malloc(strlen(tokens[end]));
        strcpy(command[i], tokens[end]);
    }

    command[end - start] = NULL;
    return end - start;
}

int cmsh_commands_process(char **tokens) {
    if( tokens[0] == NULL ) {
        return 1;
    }

    char** command;
    int t = 0;
    while( tokens[t] != NULL ) {
        command = malloc(100 * sizeof(char *));
        int count_args = extract_command(tokens, t, command);
        t += count_args;

        if( tokens[t] == NULL ) {
            return cmsh_execute(command, NULL, NULL, 0);
            free(command);
            continue;
        }

        char* input = NULL;
        char* output = NULL;

        if( tokens[t] != NULL && strcmp(tokens[t], "<") == 0 ) {
            if( tokens[t + 1] == NULL ) {
                return -1;
            }
            input = tokens[t + 1];
            t += 2;
        }

        if( tokens[t] != NULL && ( strcmp(tokens[t], ">") == 0 || strcmp(tokens[t], ">>") == 0 ) ) {
            if( tokens[t + 1] == NULL ) {
                return -1;
            }
            output = tokens[t + 1];
            t += 2;
        }
        int append = strcmp(tokens[t - 2], ">>") == 0 ? 1 : 0;

        int status = cmsh_execute(command, input, output, append);
        free(command);
        if(status == -1) return -1;
    }

    return 1;
}
