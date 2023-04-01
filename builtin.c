#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "parser.h"
#include "builtin.h"


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


int cmsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0) {
        // Child process
        if( execvp(args[0], args) == -1 ) {
            perror("cmsh: command error\n");
        }
        exit (EXIT_FAILURE);
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


int cmsh_execute(char **args) {
    if( args[0] == NULL ) {
        return 1;
    }

    for(int i = 0; i < cmsh_num_builtins(); i ++) {
        if( strcmp(args[0], builtin_str[i]) == 0 )
            return (*builtin_func[i])(args);
    }

    return cmsh_launch(args);
}

