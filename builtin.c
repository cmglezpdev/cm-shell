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

char* CMSH_HOME;

// List builtin commands, followed by their corresponding functions
char *builtin_str[] = { 
    "cd", 
    "exit" 
};

char *builtin_str_out[] = { 
    "help",
    "history"
};

int (*builtin_func[]) (char **) = {
    &cmsh_cd,
    &cmsh_exit
};

int (*builtin_func_out[]) (char **) = {
    &cmsh_help,
    &cmsh_history
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
    exit(EXIT_SUCCESS);
}


int cmsh_history(char** args) {
    char** history = get_history();

    for(int i = 0; history[i] != NULL; i ++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
    free(history);
    return 1;
}


char** get_history() {
    char* hpath = get_history_file_path();
    char** commands = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char*));
    int fd = open(hpath, O_RDONLY | O_CREAT, 0600);
    char* doc = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char));
    read(fd, doc, CMSH_TOK_BUFF_SIZE);

    char* line = NULL;
    int s = 0, k = 0, n = strlen(doc);

    for(s = 0; s < n; s ++) {
        int e = s;
        while( e < n && doc[e] != '\n' ) e ++;
        line = sub_str(doc, s, e - 1);
        commands[k] = malloc((e - s) * sizeof(char));
        strcpy(commands[k ++], line);
        s = e;
    }

    commands[k] = NULL;
    if(line != NULL) free(line); 
    free(doc); free(hpath); close(fd);
    return commands;
}

char* get_again(int number) {
    if( number < 1 || number > 10 ) {
        perror("cmsh: The command number should be between [1..10]\n");
        exit(EXIT_FAILURE);
    }
    number --;

    char** commands = get_history();
    char* again = NULL;

    for(int i = 0; commands[i] != NULL; i ++ ) {
        if( number == i ) {
            again = malloc(strlen(commands[i]) * sizeof(char));
            strcpy(again, commands[i]);
            break;
        }
    }

    if( again == NULL ) {
        free(commands);
        perror("cmsh: The command doesn't exist in the history\n");
        exit(EXIT_FAILURE);
    }

    free(commands);
    return again;
}

void save_in_history(char *line) {
    char** commands = get_history();
    int t; // total commands
    for(t = 0; commands[t] != NULL; t ++);
    int start = (t < 10) ? 0 : 1;
    char* history = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char));

    for(int i = start; i < t; i ++) {
        strcat(history, commands[i]);
        strcat(history, "\n");
    }

    strcat(history, line);
    
    char* hpath = get_history_file_path();
    int fd = open(hpath, O_WRONLY | O_TRUNC);
    write(fd, history, strlen(history));

    free(history); free(commands); free(hpath);
    close(fd);
}


int cmsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int cmsh_num_builtins_out() {
    return sizeof(builtin_str_out) / sizeof(char *);
}

char* get_history_file_path() {
    char *hpath = malloc((strlen(CMSH_HOME) + strlen(CMSH_HISTORY_FILE) + 1) * sizeof(char));
    strcpy(hpath, CMSH_HOME);
    strcat(hpath, "/");
    strcat(hpath, CMSH_HISTORY_FILE);
    return hpath;
}
