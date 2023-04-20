
#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H
#endif

// #define CMSH_HISTORY_FILE ".cmsh_history"
#define CMSH_HISTORY_FILE ".cmsh_history"
extern char* CMSH_HOME;

// List builtin commands, followed by their corresponding functions
extern char *builtin_str[2];

extern int (*builtin_func[2]) (char **);

extern char *builtin_str_out[2];

extern int (*builtin_func_out[2]) (char **);

char* get_history_file_path();

char** get_history();

char* get_again(int number);

void save_in_history(char *line);

// Function Declarations for builtin shell commands:
int cmsh_cd(char **args);

int cmsh_help(char **args);

int cmsh_exit(char **args);

int cmsh_history(char **args);

int cmsh_num_builtins();

int cmsh_num_builtins_out();
