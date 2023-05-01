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
#include<signal.h>

#include "execute.h"
#include "builtin.h"
#include "utils.h"
#include "parser.h"
#include "list.h"

struct List *background_pid;

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
        command = malloc(100 * sizeof(char*));
        count_args = extract_command(original, tokens, t, command);
        if( count_args == -1 ) {
            free(original); free(tokens); free(command);
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

    char* instruction = NULL;
    int status = -1;

    int positions[CMSH_TOK_BUFF_SIZE];
    char **tokens = cmsh_split_line(line, CMSH_TOK_DELIM, positions);
    if( tokens == NULL ) return EXIT_FAILURE;

    int ptoken = 0, pindex = 0;
    for(; tokens[ptoken] != NULL; ptoken ++) {
        int etoken = ptoken;
        pindex = positions[ptoken];
        
        int status_if = -1;
        if( strcmp(tokens[ptoken], "if") == 0 ) {
            int _if, _then, _else, _end;
            _if = ptoken;
            int success = get_if_then_else_end(tokens, _if, &_then, &_else, &_end);
            if( success == -1 || _if + 1 == _then || _then + 1 == _else || _then + 1 == _end || _else + 1 == _end ) {
                perror("cmsh: Bad command. The if command is wrong written\n");
                status = EXIT_FAILURE;
                break;
            }

            // if...then
            instruction = sub_str(line, positions[_if + 1], positions[_then - 1] + strlen(tokens[_then - 1]) - 1);
            status_if = cmsh_instructions_process(instruction);
            if( status_if == EXIT_FAILURE && _else == -1 ) {
                status = status_if;
                break;
            }

            if( status_if == EXIT_SUCCESS ) {
                // then...else or then ... end
                int aux = _else != -1 ? _else : _end;
                instruction = sub_str(line, positions[_then + 1], positions[aux - 1] + strlen(tokens[aux - 1]) - 1);
                status_if = cmsh_instructions_process(instruction);
            } 
            else if( _else != -1 ) {
                 // else...end
                instruction = sub_str(line, positions[_else + 1], positions[_end - 1] + strlen(tokens[_end - 1]) - 1);
                status_if = cmsh_instructions_process(instruction);
            }

            ptoken = etoken = _end + 1;
            
            if( tokens[_end + 1] == NULL ) {
                status = status_if;
                break;
            }

            if( strcmp(tokens[_end + 1], "if") != 0 && !is_concat_operator(tokens[_end + 1]) ) {
                perror("cmsh: Bad command\n");
                exit(EXIT_FAILURE);
            }

        } else {
            while( tokens[etoken] != NULL && !is_concat_operator(tokens[etoken]) ) etoken ++;
            instruction = sub_str(line, positions[ptoken], positions[etoken - 1] + strlen(tokens[etoken - 1]) - 1);
            ptoken = etoken;

            if( tokens[ptoken] == NULL ) {
                status = cmsh_commands_process(instruction);
                break;
            }
        }

        if( strcmp(tokens[ptoken], ";") == 0 ) {
            int ss = status_if != -1 ? status_if : cmsh_commands_process(instruction);
            status = ( ss == EXIT_SUCCESS ) 
                    ? EXIT_SUCCESS
                    : ( status == EXIT_SUCCESS )
                    ? EXIT_SUCCESS : EXIT_FAILURE;
            continue;
        }
        
        if( strcmp(tokens[ptoken], "||") == 0 ) {
            int ss = status_if != -1 ? status_if : cmsh_commands_process(instruction);
            status = ss;
            if( status == EXIT_SUCCESS ) {
                // skip the next command 
                ptoken ++;
                while( tokens[ptoken] != NULL && ( !is_concat_operator(tokens[ptoken])
                    || (is_concat_operator(tokens[ptoken]) && strcmp(tokens[ptoken], "||") == 0)) ) ptoken ++;
                
                if( tokens[ptoken] == NULL ) break;
                continue; 
            };

            continue;
        }

        if( strcmp(tokens[ptoken], "&&") == 0 ) {
            int ss = status_if != -1 ? status_if : cmsh_commands_process(instruction);
            status = ss;
            if( status == EXIT_FAILURE ) {
                // skip the next command 
                ptoken ++;
                while( tokens[ptoken] != NULL && ( !is_concat_operator(tokens[ptoken])
                    || (is_concat_operator(tokens[ptoken]) && strcmp(tokens[ptoken], "&&") == 0)) ) ptoken ++;
                
                if( tokens[ptoken] == NULL ) break;
                continue; 
            }
        } else {
            perror("cmsh: Bad command\n");
            break;
        }
    }

    free(tokens); 
    return status;
}

int cmsh_pre_process(char* line) {
    // see if is an empty command 
    line = delete_comment(line);
    if( is_empty_command(line) ) return EXIT_SUCCESS;
    line = remplace_command_again(line);
    if( line == NULL ) return EXIT_FAILURE;
    int positions[CMSH_TOK_BUFF_SIZE];
    char** tokens = cmsh_split_line(line, CMSH_TOK_DELIM, positions);
    char* original = malloc(strlen(line));
    strcpy(original, line);
    int status;

    // see if i'll save the commnand
    int save = contain(line[0], CMSH_TOK_DELIM) == 0
        ? (strcmp(tokens[0], "history") == 0 && tokens[1] == NULL) || strcmp(tokens[0], "exit") == 0
        ? 1 : 2 : 0; 

    // save in the history
    if( save == 1 ) save_in_history(original);

    // search a & operator
    int ampersand = -1;
    for(
        int i = 0; 
        tokens[i] != NULL; 
        ampersand = strcmp(tokens[i], "&") == 0 ? i : ampersand, i ++ 
    );

    if( ampersand != -1 && tokens[ampersand + 1] == NULL ) {
        // execute in the background
        line = sub_str(line, 0, positions[ampersand] - 1);
        pid_t pid = fork();
        if( pid == 0 ) {
            setpgid(0, 0);
            exit(status = cmsh_instructions_process(line));
        } else if(pid > 0) {
            setpgid(pid, pid);
            append(background_pid, pid);
            printf("[%d]\t%d\n", background_pid->len, pid);
        }
    } else {
        status = cmsh_instructions_process(line);
    }

    // save in the history
    if( save == 2 ) save_in_history(original); 
    
    free(original); free(tokens);
    return status;
}

int get_if_then_else_end(char** tokens, int _if, int *_then, int *_else, int *_end) {
    *_then = *_else = *_end = -1;
    int k = _if;
    int skip = 0;

    if( tokens[_if] == NULL || strcmp(tokens[_if], "if") != 0 ) return -1;

    k ++;
    while(tokens[k] != NULL) {
        if( strcmp(tokens[k], "then") == 0 && skip == 0 ) break;
        if( strcmp(tokens[k], "if") == 0 ) skip ++;
        if( strcmp(tokens[k], "end") == 0 ) skip --;
        if( skip < 0 ) return -1;
        k ++;
    }
    if( tokens[k] == NULL || skip != 0 ) return -1; // "then" isn't found
    *_then = k ++;

    while(tokens[k] != NULL) {
        if( ( strcmp(tokens[k], "else") == 0 || strcmp(tokens[k], "end") == 0 ) && skip == 0 ) break;
        if( strcmp(tokens[k], "if") == 0 ) skip ++;
        if( strcmp(tokens[k], "end") == 0 ) skip --;
        if( skip < 0 ) return -1;
        k ++;
    }

    if( tokens[k] == NULL || skip != 0 ) return -1; // "else" and "end" aren't found
    if( strcmp(tokens[k], "end") == 0 ) {
        *_end = k;
        return 1; 
    }
    *_else = k ++;

    while(tokens[k] != NULL) {
        if( strcmp(tokens[k], "end") == 0 && skip == 0 ) break;
        if( strcmp(tokens[k], "if") == 0 ) skip ++;
        if( strcmp(tokens[k], "end") == 0 ) skip --;
        if( skip < 0 ) return -1;
        k ++;
    }

    if( tokens[k] == NULL || skip != 0 ) return -1; // "end" isn't found
    *_end = k;

    return 1;
}

void signal_hander(int signum) {
    signal(SIGINT, SIG_DFL);
} 