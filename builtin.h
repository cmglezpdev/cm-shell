
#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H
#endif


// List builtin commands, followed by their corresponding functions
extern char *builtin_str[3];

extern int (*builtin_func[3]) (char **);


// Mainly function to execute the commands
int cmsh_execute(char **args, char* input, char* output);

int cmsh_launch(char **args, char* input, char* output);


// Function Declarations for builtin shell commands:
int cmsh_cd(char **args);

int cmsh_help(char **args);

int cmsh_exit(char **args);

int cmsh_num_builtins();

void extract_command(char** tokens, int start, char** command, int *size);

int cmsh_commands_process(char **tokens);