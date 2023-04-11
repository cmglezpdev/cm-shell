//
// Created by cmglezpdev on 4/8/2023.
//

#ifndef PRACTIC_SHELL_EXECUTE_H
#define PRACTIC_SHELL_EXECUTE_H

#endif //PRACTIC_SHELL_EXECUTE_H


void extract_command(char** tokens, int start, char** command, int *size);

int cmsh_commands_process(char **tokens);

int cmsh_execute(char **args, char* input, char* output, int append);

int cmsh_launch(char **args, char* input, char* output, int append);
