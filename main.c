#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "parser.h"
#include "builtin.h"


#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"


void head_shell(char *name) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%cmsh@%s:%s%s$ %s", BOLD_RED, name, BOLD_CYAN, cwd, WHITE);
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