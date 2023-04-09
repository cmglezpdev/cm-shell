#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>
#include<fcntl.h>

#include "parser.h"
#include "builtin.h"

void print_command(char** args) {
    printf("COMMAND \n");
    for(int i = 0; args[i] != NULL; i ++)
        printf("%s ", args[i]);
    printf("\n");
}


// List builtin commands, followed by their corresponding functions
char *builtin_str[] = { 
    "cd", 
    "help",
    "exit" 
};

int (*builtin_func[]) (char **) = {
    &cmsh_cd,
    &cmsh_help, 
    &cmsh_exit
};


int redirect_out(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    }

    return fd;
}

int redirect_in(char *fileName) {
    char *end_ptr = 0;

    int fd = (int) strtol(fileName, &end_ptr, 10);

    if (*(end_ptr + 1) != '\0') {
        fd = open(fileName, O_RDONLY);
    }

    return fd;
}


// Builtin functions implementation
int cmsh_cd(char **args) {
    if( args[1] == NULL ) {
        fprintf(stderr, "cmsh: expected argument to \"cd\"\n");
    } else {
        if( chdir(args[1]) != 0 ) {
            perror("cmsh");
        }
    }
    return 1;
}

int cmsh_help(char **args)
{
    printf("Carlos Manuel Gonzalez's CMSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < cmsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int cmsh_exit(char **args) {
    return 0;
}


int cmsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


int cmsh_launch(char **args, char* input, char* output) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {

        int fd = -1;
        if( output != NULL ) {
            fd = redirect_out(output);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Child process
        if( execvp(args[0], args) == -1 ) {
            perror("cmsh: command error\n");
        }
        if( fd != -1 ) dup2(STDOUT_FILENO, fd);
        exit(EXIT_FAILURE);

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


int cmsh_execute(char **args, char* input, char* output) {
    if( args[0] == NULL ) {
        return 1;
    }

    for(int i = 0; i < cmsh_num_builtins(); i ++) {
        if( strcmp(args[0], builtin_str[i]) == 0 )
            return (*builtin_func[i])(args);
    }

    return cmsh_launch(args, input, output);
}

char* operators[] = {">", "<", ">>", "|"};
int is_operator(char* token) {
    for(int i = 0; i < 4; i ++) {
        if( strcmp(token, operators[i]) == 0 ) return 1;
    }
    return 0;
}

void extract_command(char** tokens, int start, char** command, int *size) {
    int i = 0, end = start;
    for(; tokens[end] != NULL && is_operator(tokens[end]) == 0; end ++, i ++) {
        command[i] = malloc(strlen(tokens[end]));
        strcpy(command[i], tokens[end]);
    }

    *size = end - start;
    command[*size] = NULL;
}

int cmsh_commands_process(char **tokens) {
    if( tokens[0] == NULL ) {
        return 1;
    }

    char** command;
    int t = 0;
    while( tokens[t] != NULL ) {
        int count_args = 0;
        command = malloc(100 * sizeof(char *));
        extract_command(tokens, t, command, &count_args);
        t += count_args;

        if( tokens[t] == NULL ) {
            return cmsh_execute(command, NULL, NULL);
            free(command);
            continue;
        }

        if( strcmp(tokens[t], ">") == 0 ) {
            if( tokens[t + 1] == NULL ) {
                return -1;
            }
            int status = cmsh_execute(command, NULL, tokens[t + 1]);
            free(command);
            if(status == -1) return -1;
        }
        t += 2;
    }

    return 1;
}
