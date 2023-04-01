#include "main.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"

void head_shell(char *name) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%cmsh@%s:%s%s$ %s", BOLD_RED, name, BOLD_CYAN, cwd, WHITE);
}

char *cmsh_read_line( void ) {
    char *line = NULL;
    size_t buffsize = 0;

    if( getline(&line, &buffsize, stdin) == -1 ) {
        if( feof(stdin) ) {
            exit(EXIT_SUCCESS);
        } else {
            perror("cmsh: getline\n");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

#define CMSH_TOK_BUFF_SIZE 1024
#define CMSH_TOK_DELIM " \t\r\n\a"

char **cmsh_split_line(char * line) {
    int buffsize = CMSH_TOK_BUFF_SIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char *));
    char *token;

    if( !tokens ) {
        fprintf(stderr, "cmsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CMSH_TOK_DELIM);
    while(token != NULL) {
        tokens[position ++] = token;
        if( position >= buffsize ) {
            buffsize += CMSH_TOK_BUFF_SIZE;
            tokens = realloc(tokens, buffsize * sizeof(char *));

            if( !tokens ) {
                fprintf(stderr, "cmsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CMSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
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




// Function Declarations for buildin shell commands:
int cmsh_cd(char **args);
int cmsh_help(char **args);
int cmsh_exit(char **args);

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

int cmsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
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


void cmsh_loop( void ) {
    char *line;
    char **args;
    int status;

    uid_t uid;
    struct passwd *pw;
    uid = getuid();
    pw = getpwuid(uid);

    do {
        head_shell(pw -> pw_name);
        line = cmsh_read_line();
        args = cmsh_split_line(line);
        status = cmsh_execute(args);
    } while(status);
    
    free(line);
    free(args);
}


int main(int ac, char **argv) {

    cmsh_loop();

    return EXIT_SUCCESS;
}