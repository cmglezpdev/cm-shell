#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#include "execute.h"
#include "parser.h"
#include "builtin.h"

#define BOLD_CYAN "\033[1;36m"
#define WHITE "\033[0m"
#define BOLD_RED "\033[1;31m"


void head_shell(char *name) {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%scmsh@%s:%s%s$ %s", BOLD_RED, name, BOLD_CYAN, cwd, WHITE);
}


void cmsh_loop( void ) {
    char *line;
    int status;

    uid_t uid;
    struct passwd *pw;
    uid = getuid();
    pw = getpwuid(uid);

    CMSH_HOME = malloc(strlen(pw -> pw_dir) * sizeof(char));
    strcpy(CMSH_HOME, pw -> pw_dir);

    cmsh_init_vars();
    cmsh_create_history_file();

    do {
        head_shell(pw -> pw_name);
        line = cmsh_read_line();
        status = cmsh_pre_process(line);
    } while(1);   
}

int main(int ac, char **argv) {
    
    cmsh_loop();

    return EXIT_SUCCESS;
}