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

int cmsh_launch(char **args, int fd_input, int fd_output, int pipes[]) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        if( fd_input != -1 ) {
            dup2(fd_input, STDIN_FILENO);
            // close(fd_input);
        }

        if( fd_output != -1 ) {
            dup2(fd_output, STDOUT_FILENO);
        }else 
        if( pipes[1] != -1 ) {
            dup2(pipes[1], STDOUT_FILENO);
            close(pipes[0]);
        }

        // Child process
        if( execvp(args[0], args) == -1 ) {
            perror("cmsh: command error\n");
            exit(EXIT_FAILURE);
        }
        // if( fd_input != -1 ) dup2(STDIN_FILENO, fd_input);
        // if( fd_output != -1 ) dup2(STDOUT_FILENO, fd_output);

    } else if (pid < 0) {
        // Error forking
        perror("cmsh: forking error\n");
    } else {
        // Parent Process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if( pipes[1] != -1 ) close(pipes[1]);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int cmsh_execute(char **args, int fd_in, int fd_out, int pipes[]) {
    if( args[0] == NULL ) {
        return 1;
    }

    for(int i = 0; i < cmsh_num_builtins(); i ++) {
        if( strcmp(args[0], builtin_str[i]) == 0 )
            return (*builtin_func[i])(args);
    }

    return cmsh_launch(args, fd_in, fd_out, pipes);
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

    int t = 0, count_args;
    int fd_input = -1, fd_output = -1;
    char** command;
    
    // Get the first command
    command = malloc(100 * sizeof(char*));
    count_args = extract_command(tokens, 0, command);
    t += count_args;

    // See if there is an input file
    if( tokens[t] != NULL && strcmp(tokens[t], "<") == 0 ) {
        if( tokens[t + 1] == NULL ) {
            return -1;
        }
        fd_input = file_descriptor_in(tokens[t + 1]);
        t += 2;
    }

    while( tokens[t] != NULL ) {

        // See if there is an output file
        if( strcmp(tokens[t], ">") == 0 || strcmp(tokens[t], ">>") == 0 ) {
            if( tokens[t + 1] == NULL ) {
                return -1;
            }
            fd_output = strcmp(tokens[t], ">") == 0 
                ? file_descriptor_out(tokens[t + 1]) 
                : file_descriptor_out_append(tokens[t + 1]);
            
            t += 2;
            break;
        }

        // Pipe
        if( strcmp(tokens[t], "|") == 0 ) {
            int pipefd[2];
            pipe(pipefd);

            int status = cmsh_execute(command, fd_input, -1, pipefd);
            // close(pipefd[1]);
            fd_input = pipefd[0]; // save to the next command
            // dup2(fd_input, pipefd[0]);
            // close(pipefd[0]); close(pipefd[1]);
            t ++;
        }

        // Get another command
        free(command);
        command = malloc(100 * sizeof(char*));
        count_args = extract_command(tokens, t, command);
        t += count_args;
    }   

    int temp[2] = {-1, -1};
    int status = cmsh_execute(command, fd_input, fd_output, temp);
    close(fd_input); close(fd_output);
    free(command);
    return status;
} 
