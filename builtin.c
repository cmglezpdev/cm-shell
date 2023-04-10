#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>
#include<fcntl.h>

#include "utils.h"
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
