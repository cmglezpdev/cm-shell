
#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H
#endif


// List builtin commands, followed by their corresponding functions
extern char *builtin_str[3];

extern int (*builtin_func[3]) (char **);

// Function Declarations for builtin shell commands:
int cmsh_cd(char **args);

int cmsh_help(char **args);

int cmsh_exit(char **args);

int cmsh_num_builtins();
