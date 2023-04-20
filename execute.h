//
// Created by cmglezpdev on 4/8/2023.
//

#ifndef PRACTIC_SHELL_EXECUTE_H
#define PRACTIC_SHELL_EXECUTE_H

int extract_command(char* line, char** tokens, int start, char** command);

int cmsh_commands_process(char *line);

int cmsh_instructions_process(char* line);

int cmsh_execute(char **args, int fd_in, int fd_out, int pipes[]);

int cmsh_launch(char **args, int fd_input, int fd_output, int pipes[]);

#endif //PRACTIC_SHELL_EXECUTE_H


