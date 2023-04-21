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

int extract_command(char* line, char** tokens, int start, char** command) {
    int i = 0, end = start;
    if( tokens[start] == NULL ) return 0;

    // set command
    if( !strcmp(tokens[start], "set") ) {
        command[i] = malloc(strlen(tokens[end]));
        strcpy(command[i++], tokens[end++]);

        // Take the name of the varaible
        if( tokens[end] != NULL && !is_operator(tokens[end]) ) {
            command[i] = malloc(strlen(tokens[end]));
            strcpy(command[i++], tokens[end++]);
        } else {
            command[i] = NULL;
            return end - start;
        }

        //take the value of the variable
        if( tokens[end] != NULL && tokens[end][0] == '`' ) {
            command[i] = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char));
            int first = 1;
            // Have an expresion to evaluate
            for(; tokens[end] != NULL; end ++) {
                first ? strcpy(command[i], tokens[end]) : strcat(command[i], tokens[end]);
                first = 0;  
                if( tokens[end][ strlen(tokens[end]) - 1 ] == '`' ) {
                    i ++, end++;
                    break;
                }
                strcat(command[i], " ");
            }

            if( tokens[end] == NULL && command[i - 1][ strlen(command[i - 1]) -1 ] != '`' ) {
                perror("cmsh: The value is worng(Waiting ` ` close)\n");
                return -1;
            }

            command[i] = NULL;
            return end - start;

        } else {
            // Take the rest of the command like a string 
            int n = strlen(line), ctokens = 0;
            for(int j = 0; j < n; j++) {
                if( contain(line[j], CMSH_TOK_DELIM) ) continue;
            
                if( ctokens == end ) {
                    command[i] = sub_str(line, j, strlen(line) - 1);
                    command[i + 1] = NULL;
                    // print_tokens(command);
                    n = 0;
                    for(; tokens[n] != NULL; n++);
                    return n - start;
                }

                // start a token
                while(j < n && !contain(line[j], CMSH_TOK_DELIM)) j ++; j--;
                ctokens ++;
            }   
        }
    }

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

    char* original = malloc(strlen(line));
    strcpy(original, line);

    int positions[CMSH_TOK_BUFF_SIZE];
    char **tokens = cmsh_split_line(line, CMSH_TOK_DELIM, positions);
    if( tokens == NULL ) return EXIT_FAILURE;

    int t = 0, count_args;
    int fd_input = -1, fd_output = -1;
    char** command;

    // Get the first command
    command = malloc(100 * sizeof(char*));
    count_args = extract_command(original, tokens, 0, command);
    if( count_args == -1 ) {
        free(original); free(tokens); free(command);
        return EXIT_FAILURE;
    }
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
            continue;
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
        count_args = extract_command(original, tokens, t, command);
        if( count_args == -1 ) {
            free(line); free(original); free(tokens); free(command);
            return EXIT_FAILURE;
        }
        t += count_args;
    }   

    int temp[2] = {-1, -1};
    int status = cmsh_execute(command, fd_input, fd_output, temp);

    close(fd_input); close(fd_output);
    free(command); free(tokens); free(original);
    return status;
} 


int cmsh_instructions_process(char* line) {
   // see if is an empty command 
    if( is_empty_command(line) ) return EXIT_SUCCESS;    
    line = delete_comment(line);

    char* original = malloc(strlen(line));
    strcpy(original, line);
    char* instruction;
    int status = -1;

    line = remplace_command_again(line);
    if( line == NULL ) return EXIT_FAILURE;
    int positions[CMSH_TOK_BUFF_SIZE];
    char **tokens = cmsh_split_line(line, CMSH_TOK_DELIM, positions);
    if( tokens == NULL ) return EXIT_FAILURE;

    // see if i'll save the commnand
    int save = line[0] != ' ' 
        ? !strcmp(tokens[0], "history") && tokens[1] == NULL || !strcmp(tokens[0], "exit")
        ? 2 : 1 : 0; 

    // save in the history
    if( save == 2 ) save_in_history(line);

    int ptoken = 0, pindex = 0;
    for(; tokens[ptoken] != NULL; ptoken ++) {
        int etoken = ptoken;
        pindex = positions[ptoken];

        while( tokens[etoken] != NULL && !is_concat_operator(tokens[etoken]) ) etoken ++;
        instruction = sub_str(line, positions[ptoken], positions[etoken - 1] + strlen(tokens[etoken - 1]) - 1);
        ptoken = etoken;

        if( tokens[ptoken] == NULL ) {
            status = cmsh_commands_process(instruction);
            break;
        }
        
        if( strcmp(tokens[ptoken], ";") == 0 ) {
            int ss = cmsh_commands_process(instruction);
            status = ( ss == EXIT_SUCCESS ) 
                    ? EXIT_SUCCESS
                    : ( status == EXIT_SUCCESS )
                    ? EXIT_SUCCESS : EXIT_FAILURE;
            continue;
        }
        
        if( strcmp(tokens[ptoken], "||") == 0 ) {
            int ss = cmsh_commands_process(instruction);
            if( status == -1 ) status = ss;
            if( status == EXIT_SUCCESS ) continue;
            status = ss;
            continue;
        }

        if( strcmp(tokens[ptoken], "&&") == 0 ) {
            int ss = cmsh_commands_process(instruction);
            if( status == -1 ) status = ss;
            if( status == EXIT_FAILURE ) continue;
            status = ss;
        }
    }

    // save in the history
    if( save == 1 ) save_in_history(line); 
    free(line); free(tokens); free(instruction);
    free(original);
    return status;
}