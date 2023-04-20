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
        // Child process

        if( fd_input != -1 ) {
            dup2(fd_input, STDIN_FILENO);
        }
        if( fd_output != -1 ) {
            dup2(fd_output, STDOUT_FILENO);
        }else 
        if( pipes[1] != -1 ) {
            dup2(pipes[1], STDOUT_FILENO);
            close(pipes[0]);
        }

        // builtin commands with output
        for(int i = 0; i < cmsh_num_builtins_out(); i ++) {
            if( strcmp(args[0], builtin_str_out[i]) == 0 ) {
                exit((*builtin_func_out[i])(args));
            }
        }
        
        // Linux internal command 
        if( execvp(args[0], args) == -1 ) {
            perror("cmsh: command error\n");
        }
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

    return EXIT_SUCCESS;
}


int cmsh_execute(char **args, int fd_in, int fd_out, int pipes[]) {
    if( args[0] == NULL ) {
        return EXIT_SUCCESS;
    }

    for(int i = 0; i < cmsh_num_builtins(); i ++) {
        if( strcmp(args[0], builtin_str[i]) == 0 )
            return (*builtin_func[i])(args);
    }

    return cmsh_launch(args, fd_in, fd_out, pipes);
}


int extract_command(char** tokens, int start, char** command) {
    int i = 0, end = start;
    for(; tokens[end] != NULL && !is_operator(tokens[end]); end ++, i ++) {
        command[i] = malloc(strlen(tokens[end]));
        strcpy(command[i], tokens[end]);
    }

    command[end - start] = NULL;
    return end - start;
}

int cmsh_commands_process(char* line) {
    // see if is an empty command 
    if( is_empty_command(line) ) return EXIT_SUCCESS;    
    line = delete_comment(line);
    line = remplace_command_again(line);
    if( line == NULL ) return EXIT_FAILURE;
    char **tokens = cmsh_split_line(line, CMSH_TOK_DELIM);
    if( tokens == NULL ) return EXIT_FAILURE;

    // see if i'll save the commnand
    int save = line[0] != ' ' 
        ? !strcmp(tokens[0], "history") || !strcmp(tokens[0], "exit")
        ? 2 : 1 : 0; 
    // save in the history
    if( save == 2 ) save_in_history(line);

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
                !strcmp(tokens[t], ">")
                    ? perror("cmsh: The command > needs an output file\n")
                    : perror("cmsh: The command >> needs an output file\n");
                return EXIT_FAILURE;
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
            if( status == EXIT_FAILURE ) return EXIT_FAILURE;
            fd_input = pipefd[0]; // save to the next command
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
    if( save == 1 ) save_in_history(line);

    close(fd_input); close(fd_output);
    free(command); free(line); free(tokens);
    return status;
} 
