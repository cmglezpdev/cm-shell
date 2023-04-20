
#ifndef SHELL_BUILTIN_H
#define SHELL_BUILTIN_H
#endif

// #define CMSH_HISTORY_FILE ".cmsh_history"
#define CMSH_HISTORY_FILE ".cmsh_history"
#define CMSH_SIZE_APHABET_VARIABLES 26

extern char* vars[CMSH_SIZE_APHABET_VARIABLES];
extern char* CMSH_HOME;

// List builtin commands, followed by their corresponding functions
extern char *builtin_str[3];

extern int (*builtin_func[3]) (char **);

extern char *builtin_str_out[3];

extern int (*builtin_func_out[3]) (char **);

char* get_history_file_path();

char** get_history();

char* get_again(int number);

void save_in_history(char *line);

void cmsh_init_vars();


// Function Declarations for builtin shell commands:
int cmsh_cd(char **args);

int cmsh_help(char **args);

int cmsh_exit(char **args);

int cmsh_history(char **args);

int cmsh_get(char **args);

int cmsh_set(char **args);


int cmsh_num_builtins();

int cmsh_num_builtins_out();
