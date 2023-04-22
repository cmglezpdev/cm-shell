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
#include "execute.h"

char* CMSH_HOME;
char* vars[CMSH_SIZE_APHABET_VARIABLES];

// List builtin commands, followed by their corresponding functions
char *builtin_str[] = { 
    "cd", 
    "exit",
    "set",
    "unset",
    "true",
    "false"
};

char *builtin_str_out[] = { 
    "help",
    "history",
    "get"
};

int (*builtin_func[]) (char **) = {
    &cmsh_cd,
    &cmsh_exit,
    &cmsh_set,
    &cmsh_unset,
    &cmsh_true,
    &cmsh_false
};

int (*builtin_func_out[]) (char **) = {
    &cmsh_help,
    &cmsh_history,
    &cmsh_get
};


// Builtin functions implementation
int cmsh_cd(char **args) {
    if( args[1] == NULL ) {
        perror("cmsh: expected argument to \"cd\"\n");
        return EXIT_FAILURE;
    } else {
        if( chdir(args[1]) != 0 ) {
            perror("cmsh");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
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
    return EXIT_SUCCESS;
}

int cmsh_exit(char **args) {
    exit(EXIT_SUCCESS);
}


int cmsh_history(char** args) {
    char** history = get_history();

    for(int i = 0; history[i] != NULL; i ++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
    if(history) free(history);
    return EXIT_SUCCESS;
}


char** get_history() {
    char* hpath = get_history_file_path();
    char** commands = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char*));
    int fd = open(hpath, O_RDONLY | O_CREAT, 0600);
    char* doc = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char));
    size_t n = read(fd, doc, CMSH_TOK_BUFF_SIZE);

    char* line = NULL;
    int s = 0, k = 0;

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
        return NULL;
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


int cmsh_set(char** args) {
    if( args[1] == NULL ) {
        for(int i = 0; i < CMSH_SIZE_APHABET_VARIABLES; i ++) {
            if( vars[i] != NULL ) {
                printf("%c = %s\n", (char)(i + 'a'), vars[i]);
            }
        }
        return EXIT_SUCCESS;
    }

    char* var = args[1];
    char* value = args[2];
    char* buffer = NULL;

    if( strlen(var) > 1 ) {
        perror("cmsh: The variable must be a letter [a...z]\n");
        return EXIT_FAILURE;
    }

    int v = var[0] - 'a';
    if( v < 0 || v >= 26 ) {
        perror("cmsh: The varaible must be a lower case letter [a...z]\n");
        return EXIT_FAILURE;
    }

    if( value == NULL ) {
        perror("cmsh: The variable needs a value\n");
        return EXIT_FAILURE;
    }

    if( value[0] == '`' ) {
        value = sub_str(value, 1, strlen(value) - 2);

        int fd[2]; pipe(fd);
        pid_t pid = fork();
        int status = 0;

        if( pid == 0 ) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            close(fd[0]);
        
            cmsh_commands_process(value);
            exit(EXIT_FAILURE);
        } else if(pid > 0) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        buffer = malloc(CMSH_TOK_BUFF_SIZE * sizeof(char));

        close(fd[1]);
        read(fd[0], buffer, CMSH_TOK_BUFF_SIZE);
        close(fd[0]);

        value = buffer;
    }

    vars[v] = malloc(strlen(value) * sizeof(char));
    strcpy(vars[v], value);
    if( buffer != NULL ) free(buffer); 

    return EXIT_SUCCESS;
}

int cmsh_get(char** args) {

    char* var = args[1];

    if( var == NULL ) return EXIT_FAILURE;

    if( strlen(var) > 1 ) {
        perror("cmsh: The variable must be a letter [a...z]\n");
        return EXIT_FAILURE;
    }

    int v = var[0] - 'a';
    if( v < 0 || v >= 26 ) {
        perror("cmsh: The varaible must be a lower case letter [a...z]\n");
        return EXIT_FAILURE;
    }

    if( vars[v] == NULL ) {
        perror("cmsh: Variable no found\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", vars[v]);
    return EXIT_FAILURE;
}

int cmsh_unset(char** args) {
    char *var = args[1];
    if( var == NULL ) {
        perror("cmsh: command error: the variable is required\n");
        return EXIT_FAILURE;
    }

    if( strlen(var) > 1 ) {
        perror("cmsh: The variable must be a letter [a...z]\n");
        return EXIT_FAILURE;
    }

    int v = var[0] - 'a';
    if( v < 0 || v >= 26 ) {
        perror("cmsh: The varaible must be a lower case letter [a...z]\n");
        return EXIT_FAILURE;
    }

    if( vars[v] == NULL ) {
        perror("cmsh: Variable no found\n");
        return EXIT_FAILURE;
    }

    free(vars[v]);
    vars[v] = NULL;
    
    return EXIT_SUCCESS;
}

void cmsh_init_vars() {
    for(int v = 0; v < CMSH_SIZE_APHABET_VARIABLES; v ++)
        vars[v] = NULL;
}

void cmsh_create_history_file() {
    char* path = get_history_file_path();
    int fd = file_descriptor_out(path);
    close(fd);
    free(path);
}

int cmsh_true(char** args) {
    return EXIT_SUCCESS;
}

int cmsh_false(char** args) {
    return EXIT_FAILURE;
}
